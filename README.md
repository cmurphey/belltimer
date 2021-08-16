# belltimer
Arduino Bell Ring Timer for a church bell tower using a single relay, lcd shield, and real-time-clock module.

The project provides the following:
* A real-time-clock to manage the time
* A LCD/Button shield to display the time
* A single relay to turn on and off the bell ring mechanism
* Software to manage:
  * Setting the time using the LCD shield buttons
  * Ringing the bell at certain hours
  * Ringing the bell at church time

## Setting the time with the LCD buttons
By default the time will be displayed.

To enter time setting mode, push the select button to reach the desired setting;
1. Hour
2. Minute
3. Year
4. Month
5. Day
6. Day of the week
7. Store the time

Press up or down to change the value.

## Manually ring the bell
Press the right button.

## Required libraries

* The DS3231 RTC module uses the NorthernWidget library -  https://github.com/NorthernWidget/DS3231
* The LCD module uses the standard lcd module - https://www.arduino.cc/reference/en/libraries/liquidcrystal/
* The RTC module communicates via I2C using the Wire library - https://www.arduino.cc/en/reference/wire 
