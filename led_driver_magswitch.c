#include <xc.h>

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

void init() {
	// set internal oscillator freqeunecy
	OSCCAL = 0x80; // set internal oscillator to mid frequency

	// GP0,1,2 as outputs (=0) rest as inputs
	TRISIO = 0b00001000;
	
	// disable comparators

	// disable ADC 
	ANSEL = 0b00000000;  // set ANS[0:3] bits to zero

	GPIO = 0; // clear GPIO

	// interrupts
	IOC = 8; // interrupt on change for GP3
	INTCON = 0;
	INTCON = INTCON | 0b10001000; // enable global interrupts, allow interrupt on change, disable all others
}

int volatile counter; // specify as volatile as only set in ISR...
int volatile counterchanged; 

void interrupt general_ISR(void) {
	// interrupt service routine. 
	// only care about GPIF at the moment...
	char b;

	if (INTCON & 0x01 == 0x01) {
		b = GPIO; // reads GPIO, allows interrupt to be cleared
		INTCON = INTCON & 0b11111110; // Clear GPIF
		if ((b & 0b00001000) == 0b00001000) {
			counter++;
			counterchanged = 1;
		}


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

	init();

	while ( 1==1 ) {
		for (i=0; i<10000; i++) {

			if (counterchanged ==1) {
				counterchanged = 0;
			
				send_byte_to_595(counter); 
				display_595();
			}
		}
     	GPIO = GPIO ^ 0b00100000;  
	}
}

