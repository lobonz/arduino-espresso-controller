# arduino-espresso-controller
An Arduino Mega Based control system for an Espresso Machine

## Background
I got my hands on a Nuova Simonelli Premier Maxi 2 Group Espresso machine for free, the main control board had blown.
The relay board was in tact so I just used 10amp relays to control the higher amp relays which were still controllable via simple switches so the machine could still operate.
Drawbacks were that filling the tank, operating the group valves, tank valve and pump were all manual so to draw a shot it invovled flicking two switches and manually timing it to the correct amount.

## What this code does:
- Monitor the fill level, engage the pump if too low and stops the pump when full.
- Allows one button to begin drawing a shot and one button to stop.
- Has two programmable buttons which can be programmed to whatever volume you like monitoring the flow sensors, e.g. Single Shot and Double Shot.

## Programming the buttons
To program the two buttons you hold the stop button down for the group head you want to program for 5 seconds, then the two programmable buttons begin flashing to indicate the choices, select the button to program, then the go button will flash, when ready (with scales) press the go button, the stop button will begin flashing the the draw will begin, when you have reached the required volume press the stop button this will save the number of pulses the flow sensor sends to the eeprom to remember it through restarts.




