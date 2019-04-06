# Bao Chicka Wow Wow blinking sign

![BCWW Sign at dusk](https://raw.github.com/jvyduna/BCWW-sign-2018/master/Physical%20sign%20design/BCWW%20signs%20sunset.jpg)

[Video of sign at Burning Man 2018 and fab process](https://vimeo.com/292647048)


## Files

The contents of this repo mirror the structure of the Arduino project directory. Dependencies are included in the libraries/ folder. For the time-of-day mode switching timers to work, you'll need to copy the contents of this directory into your Arduino build location for libraries. That's usually just ```{your Arduino homedir}/libraries```.

## Hardware

Arduino Pro or Pro Mini (config included)
ATMega328P (5V, 16 MHz)

## Pinout

Documented in the code

All 5 signs are connectted in parallel on a 10-conductor ribbon cable.
One line controls the servo that changes the fourth sign from  WOW to NOW

## Physical sign design files

Design (Illustrator)

Modeling / CAD (Autodesk Inventor)

CNC GCode: Vector / ShopBot
