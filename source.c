
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

uint touch_state = 0;
volatile bool touch_change_flg = 0;

void touch_isr_handeler(void){
    while (!pio_sm_is_rx_fifo_empty(pio0, 0))
    {
        touch_state = (touch_state & 0xffffffe0)|(pio_sm_get(pio0,0));
        touch_change_flg = 1;
    }
        while (!pio_sm_is_rx_fifo_empty(pio0, 1))
    {
        touch_state = (touch_state & 0xfffffc1f)|(pio_sm_get(pio0,1)<<5);
        touch_change_flg = 1;
    }
        while (!pio_sm_is_rx_fifo_empty(pio0, 2))
    {
        touch_state = (touch_state & 0xffff83ff)|(pio_sm_get(pio0,2)<<10);
        touch_change_flg = 1;
    }
        while (!pio_sm_is_rx_fifo_empty(pio0, 3))
    {
        touch_state = (touch_state & 0xfff07fff)|(pio_sm_get(pio0,3)<<15);
        touch_change_flg = 1;
    }
}

int main(){
    stdio_init_all();
    static const float pio_clk_div = 40;
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

//Set up pio interrupts
    pio_set_irq0_source_mask_enabled(pio0, pis_sm0_rx_fifo_not_empty|pis_sm1_rx_fifo_not_empty|pis_sm2_rx_fifo_not_empty|pis_sm3_rx_fifo_not_empty,true);
    //pio_set_irq0_source_mask_enabled(pio0,pis_sm2_rx_fifo_not_empty,true);
    irq_set_exclusive_handler(PIO0_IRQ_0,touch_isr_handeler);
    irq_set_enabled(PIO0_IRQ_0, true);

//Set up capasitive touch pio
    uint offset_touch = pio_add_program(pio0, &touch_program);
    touch_init(pio0, 0, offset_touch, 2, 5, pio_clk_div);
    touch_init(pio0, 1, offset_touch, 7, 5, pio_clk_div);
    touch_init(pio0, 2, offset_touch, 12, 2, pio_clk_div);
    pio_sm_set_enabled(pio0, 0, true); //Enable first state machine of pio0
    pio_sm_set_enabled(pio0, 1, true);
    pio_sm_set_enabled(pio0, 2, true);

    while (true){
        // if(touch_change_flg){
        //     touch_change_flg = 0;
        //     //printf("touch_state: %32b \n", touch_state);
        //     };
        printf("touch_state: %32b \n", touch_state);
        busy_wait_ms(500);
        gpio_put (LED_1, touch_state&1);
        gpio_put (LED_2, (touch_state>>1)&1);
        gpio_put (LED_3, (touch_state>>2)&1);
        gpio_put (LED_4, (touch_state>>3)&1);
        gpio_put (LED_PIN, touch_state ? 1 : 0);
    }
    return 0;
}