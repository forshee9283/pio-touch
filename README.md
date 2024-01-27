# pio-touch
Example code in pio assembly and C for a capacitive touch method that requires only copper pads. I've attempted to optimize the code to use as few hardware components as possible (just copper pads) and for flexibility. The speed at which the state machines run will need to be tuned to the size of the copper pads and the desired sensitivity. Fine tuning of some of the delays in the pio code may be necessary.

Known issues:
The pio code will re trigger with the same values occasionally this is filtered in C with 'touch_change_flg' but will create additional interrupts.

If using more than one state machine (i.e. more than five buttons) the state machines will be out of sync. The signal from the other state machines can change the reading. This is worse when one hand is used across multiple state machines.

Simplified Theory of Operation:
The pio blocks will set a pin high, wait a very short amount of time, and then read that same pin. Because of the internal pull down the voltage will discharge based on how much capacitance is on the pin. Adding a finger on a copper pad adds enough capacitance to hold the voltage high enough to read high assuming the system is tuned properly for the size of the pad.

Button Signal Without a Finger Applied
![image](https://github.com/forshee9283/pio-touch/assets/16594111/57ce9b21-93eb-4570-94d0-a2586dd0c150)

Button Signal With a Finger Applied
![image](https://github.com/forshee9283/pio-touch/assets/16594111/06c05c33-3b1f-476f-aed6-ddb9cdf87f0b)
