
#include <stdio.h>
#include "pico.h"
#include "pico/stdlib.h"
#include "hardware/pio.h"
#include "touch.pio.h"

#define LED_1 18
#define LED_2 19
#define LED_3 20
#define LED_4 21
#define LED_PIN PICO_DEFAULT_LED_PIN
#define TOUCH_PIO pio0
#define TOUCH_PIN 2 //GPIO number for the first touch button
#define TOUCH_NUMBER 12 //number of sequential touch buttons

volatile uint touch_state = 0;
volatile uint touch_state_last =0;
volatile bool touch_change_flg = 0;

void touch_isr_handeler(void){ //This assumes that the state machines are 0-2 in order and that all three are used.
    if(!pio_sm_is_rx_fifo_empty(TOUCH_PIO, 0)){
        touch_state = (touch_state & 0xffffffe0)|(pio_sm_get(TOUCH_PIO,0));
    }
    if(!pio_sm_is_rx_fifo_empty(TOUCH_PIO, 1)){
        touch_state = (touch_state & 0xfffffc1f)|(pio_sm_get(TOUCH_PIO,1)<<5);
    }
    if(!pio_sm_is_rx_fifo_empty(TOUCH_PIO, 2)){
        touch_state = (touch_state & 0xffff83ff)|(pio_sm_get(TOUCH_PIO,2)<<10);
    }
    if(touch_state!=touch_state_last){
        touch_change_flg = 1;
    }
    touch_state_last = touch_state;
    //printf("touch_state: %20b isr\n", touch_state);
}

int touch_setup(PIO pio_touch,int num_buttons, int start_pin, const float clk_div){
    if(num_buttons>20){
        return 1;
    }
    int sm;
    uint offset_touch = pio_add_program(TOUCH_PIO, &touch_program);
    if(num_buttons>0){
        sm = pio_claim_unused_sm(pio_touch,true);//Panic if unavailible
        pio_set_irq0_source_enabled(pio_touch,sm,true);//state machine number happens to be equal to rx fifo not empty bit for that state machine
        touch_init(pio_touch, sm, offset_touch, start_pin, (num_buttons>5 ? 5: num_buttons), clk_div);
        pio_sm_set_enabled(pio_touch, sm, true);
    }
    if(num_buttons>5){
        sm = pio_claim_unused_sm(pio_touch,true);//Panic if unavailible
        pio_set_irq0_source_enabled(pio_touch,sm,true);//state machine number happens to be equal to rx fifo not empty bit for that state machine
        touch_init(pio_touch, sm, offset_touch, start_pin + 5, (num_buttons>10 ? 5: (num_buttons - 5)), clk_div);
        pio_sm_set_enabled(pio_touch, sm, true);
    }
    if(num_buttons>10){
        sm = pio_claim_unused_sm(pio_touch,true);//Panic if unavailible
        pio_set_irq0_source_enabled(pio_touch,sm,true);//state machine number happens to be equal to rx fifo not empty bit for that state machine
        touch_init(pio_touch, sm, offset_touch, start_pin + 10, (num_buttons>15 ? 5: (num_buttons - 10)), clk_div);
        pio_sm_set_enabled(pio_touch, sm, true);
    }
    if(num_buttons>15){
        sm = pio_claim_unused_sm(pio_touch,true);//Panic if unavailible
        pio_set_irq0_source_enabled(pio_touch,sm,true);//state machine number happens to be equal to rx fifo not empty bit for that state machine
        touch_init(pio_touch, sm, offset_touch, start_pin + 15, (num_buttons-15), clk_div); //get the numbers right
        pio_sm_set_enabled(pio_touch, sm, true);
    }
    irq_set_exclusive_handler(PIO0_IRQ_0,touch_isr_handeler);
    //irq_add_shared_handler(PIO0_IRQ_0, touch_isr_handeler,PICO_SHARED_IRQ_HANDLER_DEFAULT_ORDER_PRIORITY);
    irq_set_enabled(PIO0_IRQ_0, true);   
}

int main(){
    stdio_init_all();
    static const float pio_clk_div = 40; //This should be tuned for the size of the buttons
    gpio_init(LED_1);
    gpio_set_dir(LED_1, GPIO_OUT);
    gpio_init(LED_2);
    gpio_set_dir(LED_2, GPIO_OUT);
    gpio_init(LED_3);
    gpio_set_dir(LED_3, GPIO_OUT);
    gpio_init(LED_4);
    gpio_set_dir(LED_4, GPIO_OUT);
    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);

    touch_setup(TOUCH_PIO, TOUCH_NUMBER, TOUCH_PIN, pio_clk_div);

    while (true){
        if(touch_change_flg){
            touch_change_flg = 0;
            printf("touch_state: %20b \n", touch_state);
            }
        gpio_put (LED_1, !pio_sm_is_rx_fifo_empty(TOUCH_PIO, 0));
        gpio_put (LED_2, !pio_sm_is_rx_fifo_empty(TOUCH_PIO, 1));
        gpio_put (LED_3, !pio_sm_is_rx_fifo_empty(TOUCH_PIO, 2));
        gpio_put (LED_4, (touch_state>>3)&1);
        gpio_put (LED_PIN, touch_state ? 1 : 0);
    }
    return 0;
}