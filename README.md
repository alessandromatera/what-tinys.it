#"What Tinys.it?"
###A 3D printed LED watch with Atmega328P microcontroller programmed with Arduino

####The Story
I'll wanted to build a LED watch with Arduino but, searching on the web, i could not find what i was looking for. So I've decided to build my own.
I wanted to use Attiny85 microcontroller from my Tinys board [Tinys](http://www.tinys.it/) but it has only few I/O and can't be used to drive many LEDs. So i decided to use an Atmega328P (the same micro of Arduino UNO board) in TQFP package with 32 pin.
For perfect timing i've used a 32.768 KHz Crystal connected to Xtal pins of Atmega328p running in asynchronous mode. In this way i can use internal 8Mhz oscillator as main clock of the microcontroller and the overflow of timer2 (used for the external crystal) as interrupt for increase the seconds.
Why 32.768Khz Crystal? Because using prescaler to 128 we have 256 Hz, so 1 overflow/sec. It's the most used crystal for watches for this reason.
What about the name? Well, my web site is [Tinys.it](http://www.tinys.it/) :D

####The Project
I'll wanted to build a watch with 60 LEDs but to keep the clock size small, i've decide to use only 32 LED (1 LED/100 seconds). But even if the microcontroller has 32 pin, they aren't all I/O: only 25 pins can be used for the 32 LEDs and the button. To drive a larger number of LEDs with few pins i've used the [Charlieplexing Matrix](http://en.wikipedia.org/wiki/Charlieplexing), in this way i can drive 6 LEDs with only 3 Pins. Becouse i can't use two different color of LEDs, to distinguish between hours and minutes, the minutes LED is blinking. 
To reduce battery drain the watch is always in sleep. To show-up the time is needed to press the button.
As the clock hands, the hours LED isn't stay in the same position: is moving a bit, consequently to minutes.
The watch is powered by a 3 Volt CR2032 battery. With this battery the LEDs resistors can be shorted.

####The Firmware
After setup the timer2 overflow interrupt and the button change-pin interrupt, the microcontroller goes to sleep. Every time the timer2 goes overflow (1 overflow/seconds) the microcontroller wakes up and increase the seconds. 
When the button is pressed for a small period of time, the button's interrupt occur and the time is show up. Instead, if the button is pressed for two seconds, we entering in programming mode, where we can set the time: after all LEDs are turned on, we can release the button and the current hours LED is turned on. Pressing one time the button hours change to the next. To record the current hours you need to press the button for about 2 seconds. After the hours LED is switched off, release the button and the minutes LED is turned on. Like the hours LED push the button to choose the right minutes and then long press to record the minutes. After the minutes LED is turned off, all LEDs is turned on. The corrected time is show up.

####Upload the sketch
To Upload the firmware i use avr in Mac terminal and an Arduino UNO as ISP programmer. 
To do that follow these steps:

1. Connect the Arduino UNO board to PC or Mac.
2. Select the `ArduinoISP` sketch from `File -> Examples`
3. Upload the sketch to the Arduino UNO board
4. Open the watch sketch
5. Compile the sketch using Arduino IDE
6. Connect the SPI watch pins to Arduino UNO pins lin this way:
	* Watch PIN `Reset - JP6` to Arduino PIN `10`
	* Watch PIN `11 - JP3` to Arduino PIN `11`
	* Watch PIN `12 - JP4` to Arduino PIN `12`
	* Watch PIN `13 - JP5` to Arduino PIN `13`
	* Watch PIN `V+ - JP1` to Arduino PIN `+3.3V`
	* Watch PIN `GND - JP2` to Arduino PIN `GND`
7. On Mac terminal (i think is similar for windows user) write this
	```
	avrdude -v -v -v -v -cstk500v1 -pm328p  -P/dev/tty.usbmodemfa131 -b19200 -D -e -Ulock:w:0x3F:m -Uefuse:w:0x07:m -Uhfuse:w:0xDE:m -Ulfuse:w:0xE2:m -Uflash:w:/path/of/your/hex/file.hex:i
	```
	
	where:
	 - `tty.usbmodemfa131` is the serial port of Arduino board
	 - `/path/of/your/hex/file.hex` is the path of .hex file that can be taken from the Arduino IDE enabling `verbose output` from `Preferences -> Show Verbose output during: -> compilation`
	
All this stuff because i wanted to program the board using SPI not using Arduino bootloader. I Think that the watch can be programmed like an Arduino UNO using the `breadboard 8Mhz` bootloader and soldering two wires to pin `0` and `1` of the watch, but i've not tried it.
I've have used fuses for disabling `BOD` so you need to modify the bootloader of Arduino to do this. (Maybe you can do this in sketch?).

####The Case
The 3D printed case was realized by my brother Andrea Matera.