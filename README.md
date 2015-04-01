#what-tinys.it
###What Tinys.it? A 3D printed LED watch with Atmel Atmega328P microcontroller programmed with Arduino

####Story
I'll wanted to build a LED watch with Arduino but, searching on the web, i could not find what i was looking for. So I've decided to build my own.
I wanted to use Attiny85 microcontroller from my Tinys board [Tinys](http://www.tinys.it/) but it has only few I/O and can't be used to drive many LEDs. So i decided to use an Atmega328P (the same micro of Arduino UNO board) in TQFP package with 32 pin.
For perfect timing i've used a 32.768 KHz Crystal connected to Xtal pins of Atmega328p running in asynchronous mode. In this way i can use internal 8Mhz oscillator as main clock of the microcontroller and the overflow of timer2 (used for the external crystal) as interrupt for increase the seconds.
Why 32.768Khz Crystal? Because using prescaler to 128 we have 256 Hz, so 1 overflow/sec. It's the most used crystal for watches for this reason.
