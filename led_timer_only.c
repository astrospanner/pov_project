/*

 led_timer_only.c by Will Grainger
 Jan 2014

 Part of the Spoke Images project

 Matching blog post: http://wp.me/p4dUG7-G


The MIT License (MIT)

Copyright (c) 2014 Will Grainger

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.

*/#include <xc.h>

// defined in MPLAB IDE v8.30 to use 12f675
// Hardware config:
//  pin 5 (GP2) ->   pin 11 on 74.595 (SH_CP)   (data clock)
//  pin 6 (GP1) ->   pin 14 on 74.595 (DATA)    (data)
//  pin 7 (GP0) ->   pin 12 on 74.595 (ST_CP)   (parallel load)
//  pin 2 (GP5) -> LED

// hall effect switch -> pin 3 (GP3) 
// no adc, no comparator
// interrupt on change for GPIO pins...

// ST_CP low, SH_CP goes high, DATA stored into shift register
// ST_CP goes high, shift register outputted 

// Config bits set in IDE
// Oscillator            internal RC No clock 
// watchdog timer        Off
// power up timer        Off
// master clear enable   Internal 
// brown out detect      OFF
// code protect          OFF
// data ee read protect  OFF

//  #include <12F675.h>

#define NOOFLINES 64

void init() {
	// set internal oscillator freqeunecy
	OSCCAL = 0x80; // set internal oscillator to mid frequency

	// GP0,1,2 as outputs (=0) rest as inputs
	TRISIO = 0b00001000;
	
	// disable comparators

	// disable ADC 
	ANSEL = 0b00000000;  // set ANS[0:3] bits to zero


	GPIO = 0; // clear GPIO

	TMR0 = 0;
	// interrupts
	IOC = 8; // interrupt on change for GP3
	INTCON = 0;
	OPTION_REG = 0b0001000; 
	T0IF = 0;
	T0IE = 1;
	GPIF = 1;
	GIE = 1;
}

unsigned int volatile magcounter; // specify as volatile as only set in ISR...

unsigned int volatile displaytimer;
unsigned int volatile displaytimerload;
unsigned int  volatile linetodisplay;
char volatile linetodisplay_changed;

void interrupt general_ISR(void) {
	// interrupt service routine. 
	// two interrupts to check; GPIO and timer0 overflow...

	if (T0IE && T0IF) { //are TMR0 interupts enable and is TMR0 interrupt set?
		T0IF = 0; // clear in software
		
		GPIO = GPIO ^ 0b00100000;


	}
}

void send_byte_to_595(char a) {
// Sends the byte a, LSB first, to the 74'595 shift register
	int i;	
	char b;

	b = a;
	for (i=0; i<8; i++) {
		if ((b & 0x01) == 0x01) {
			GPIO = GPIO | 0b00000010; // data is high...
			_delay(2);
		} else {
			GPIO = GPIO & 0b11111000; // data is low
			_delay(2);
		}
		GPIO = GPIO | 0b00000100; // clock  high
		_delay(2);
		GPIO = GPIO & 0b11111000;  // clear the bits
		b = b >>  1; // shift it right
	}
}

void display_595() {
// toggle the parallel load bit of the output to display the byte(s) that have been sent.
	_delay(2);
	GPIO = GPIO | 1;
	_delay(2);
	GPIO = GPIO & 0b11111110;
}
		

main() {
	long i;
	char j;


	// set some defaults...
	linetodisplay = 0;
	linetodisplay_changed = 0;
	displaytimerload = 10;
	displaytimer = 10;


	init();

	magcounter = 0;


	while ( 1==1 ) {
		if (linetodisplay_changed == 1) {
			linetodisplay_changed = 0;
			send_byte_to_595(0xd2);
			display_595();
		}
	}
}

