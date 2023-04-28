# ADS7924
Linux interrupt driven kernel driver for ADS7924 2.2 V, 12-Bit, 4-Channel, MicroPOWER Analog-to-Digital Converter With I2C Interface from Texas Instruments

[Datasheet](https://www.ti.com/lit/ds/symlink/ads7924.pdf?ts=1677340148175)

To optain a detiled description about this module you need [Doxygen](https://www.doxygen.nl).
1) Change in directory ```./doc```
2) Call ```doxygen```
3) Open the file ```./doc/html/index.html``` with your preferred web browser.

Apperring the drivers device-files in the directory ```/dev```:
```
adc<0-n><A-B>[0-3]
 |   |    |    |
 |   |    |    +- Channel number
 |   |    +------ I2C-address (chip-number) A = 0x48; B = 0x49
 |   +----------- Number of I2C-bus
 +--------------- Prefix
```
Where ```adc<0-n><A-B>``` is the interface for registers which concerns the whole chip,

and ```adc<0-n><A-B>[0-3]``` one of the four channels.

In the directory ```./src/test``` you can find the source code for a demo- and test- application to compile it you need additently the command line-parser which you can [obtain here](https://github.com/UlrichBecker/command_line_option_parser).

[Another helpful tool for developing Linux drivers can be found here.](https://github.com/UlrichBecker/ioctl4bash)

This code is a few years old since I last updated it. You probably need to customize it a bit to the latest Linux kernel source.

**A few words about my coding style.**

Yes, I know my code is not [Linux-style](https://www.kernel.org/doc/html/latest/process/coding-style.html).
For example, I open curly braces in a new line, and don't use tabs except in makefiles.
So the code can never officially become part of the Linux kernel, which I never planned to do.
However, since I am the only author of the code so far and have maintained it alone so far, I keep the code in a form that is most pleasing to my eyes.
I ask you to respect that as well, I respecting the coding style of others as well.

There is a saying in Germany that means: "Einem geschenkten Gaul schaut man nicht ins Maul." ("You don't look a gift horse in the mouth.")

;-)
