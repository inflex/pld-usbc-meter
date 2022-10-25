# compass-switch
Compass switch, HMC5833L IC via I2C for Linux PC

Created as an automatic switching system for use with Openbroadcaster and a boom mounted microscope assembly.  Utilises the Linux uinput event system to simulate keypresses that OBS will detect to scene switch.

Designed with the ATTINY85 Module Development Board ( Digispark -   https://www.kickstarter.com/projects/digistump/digispark-the-tiny-arduino-enabled-usb-dev-board ), but no reason it cannot work with any other I2C interface device that is supported.

Source also contains two small/useful code files:
- i2c interface code ( i2c.[ch] )
- text keycode to actual integer keycode conversion routines (keycode-decode.[ch] )
- uinput routines for simulating your own kepresses 

Because this project is MIT licenced, feel free to take those bits of hard earned code and make your own lives easier.

YouTube - https://www.youtube.com/user/19PLD73