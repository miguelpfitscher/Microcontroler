
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <stdio.h>
#include <time.h>

#include "hough.h"
#include "uart.h"

volatile uint64_t time_ms; // global time tracker
volatile uint64_t time_40hz; 


volatile uint8_t fInRead = 0;

volatile uint8_t randNum = 0;
volatile uint8_t releaseJitter = 0;
volatile uint8_t releaseHough = 0;
uint64_t toggleTime;
uint64_t yellowtoggleTime;

#define LOOP_COUNT_1MS 160
			//test = time_ms;
			//test = time_ms - test;
			//char s[200];
			//sprintf(s,"%dms\r\n", (int)test);
			//sendString(s);
			//_delay_ms(4000); //delay to print correctly

inline void setupLeds(){
	DDRC |= (1 << DDC7);	// yellow led
	DDRB |= (1 << DDB0);	// red led
	DDRD |= (1 << DDD5);	// green led
}

inline void setupButtonA(){
	DDRB &= ~(1 << DDB3);
	PORTB |= (1 << PORTB3);
	// set up for internal interrupt on INT0 = PD0
	// interrupt will activate when pin is low
	// This setup allows for a software interrupt
	EICRA &= ~(1 << ISC00);
	EICRA &= ~(1 << ISC01);
	DDRD |= (1 << PORTD0);
	PORTD |= (1 << PORTD0);	
	// enable pin change interrupt for button A
	PCICR |= (1 << PCIE0);
	PCMSK0 |= (1 << PCINT3);
}

inline void setupTimer0(){//1ms - 1000hz
	time_ms=0;
	TCCR0A |= (1 << WGM01);    // Enables CTC mode for timer0
	TCCR0B |= (1 << CS01);     // For setting the timer offset value to 64
	TCCR0B |= (1 << CS00);     // For setting the timer offset value to 64
	OCR0A = 249;               // Sets the counter to 249
	TIMSK0 |= (1 << OCIE0A); // Enable time keeper (timer0)
}

inline void setupTimer3(){//40 hz 	 
	TCCR3B |= (1 << WGM32); //ctc enable
	TCCR3B |= (1 << CS32) | (1 << CS10); //1024 prescale
	OCR3A = 390;
	TIMSK3 = (1 << OCIE3A); //Enable 40Hz timer 3
	time_40hz=0;
}

inline void setupTimer1(){ //10 hz pwm
  greenToogles = 0;
  TCCR1A |= (1<<WGM10)|(1<<WGM11)|(1<<COM1B1);
  TCCR1B |= (1 << CS32) | (1 << CS30); //1024 prescale
  OCR1B = 1561;
  TIMSK1 = (1 << OCIE1B); //enable timer 1 interrupt
}

inline void setupIO(){
	DDRD |= (1 << DDD6); // yellow led pin o/i
	DDRB |= (1 << DDB6); // green led pin o/i
	DDRB |= (1 << DDB4); // red led pin o/i
	redPeriod = 100; //red toogle default
	yellowIOPeriod = 4; //yellow toogle default
}

inline void toggleIO_redled(){
	PORTB ^= (1 << PORTB4); //toggle red led i/o
	
}

inline void toggleIO_yellowled(){
	PORTD ^= (1 << PORTD6); //toggle yellow led i/o
	jobNumb++;
}


inline void forDelay(uint32_t y){
	volatile uint64_t i;
	for(i = 0; i < LOOP_COUNT_1MS * y ; i++){  
		
	}		
	
}

inline void pauseSystem(){
	cli();
	TCCR1B &= ~((1 << CS32) | (1 << CS30)| (1 << CS31)); //disable pwm
	TIMSK3 &= ~(1 << OCIE3A); //disable interrupt timer 3
	TIMSK1 &= ~(1 << OCIE1B); //disable timer 1 interrupt
	TIMSK0 &= ~(1 << OCIE0A); //disable timer 0 interrupt
	PORTB &= ~(1 << PORTB4); //turn off red led i/o
	PORTD &= ~(1 << PORTD6);  //turn off yellow led i/o
	
	PORTC &= ~(1 << PORTC7); //turn off yellow led
	PORTB &= (1 << PORTB0); //turn off red led
	
}

inline void restartSystem(){
	TCCR1B |= ((1 << CS32) | (1 << CS30)); //enable pwm
	TIMSK1 |= (1 << OCIE1B); //enable timer 1 interrupt
	TIMSK0 |= (1 << OCIE0A); //disable timer 0 interrupt
	TIMSK3 |= (1 << OCIE3A); //enable interrupt timer 3
	time_40hz = 0; //reseting time
	time_ms = 0;
	bufferConflict = 0;
	missDeadline = 0;
	jobNumb = 0;
	greenToogles = 0;
	toggleTime = 0;
	yellowtoggleTime = 0;
	disableTxUARTinterrupt();	
}

//initialize the system
inline void systemSetup(){ 
	
	bufferConflict = 0;
	missDeadline = 0;
	jobNumb=0;
	toggleTime = 0;
	yellowtoggleTime = 0;
	
	setupLeds();
	setupButtonA();
	setupTimer0();
	setupTimer1();
	setupTimer3();	
	
	srand(time(NULL)); //for random generation	
}

inline void printOptions(){
	sendString("Type 'go' to start\r\n");
	forDelay(5);
	sendString("Press Button A to Configuration Mode\r\n");
	forDelay(5);
	sendString("Type 'p' to see the results\r\n");
	forDelay(5);
	sendString("Type 'z' to reset variables\r\n");
	forDelay(5);
	sendString("Type 'g' to start experiment\r\n");
	forDelay(5);
	sendString("Type 'q' to quit the configuration mode\r\n");
	forDelay(5);
	sendString("Type 'e'+'#' to set an experiment\r\n");
	forDelay(5);
	sendString("Type 'r'+'####' to set the green led period\r\n");
}

int main(void){
	
	//houghTransform variables
	volatile uint16_t row, col;
    volatile uint16_t nextByte;
	
	USBCON = 0;	// dont have to reset chip after flashing
	go = 0;	
	setupIO();
	setupUART();
	
	//to see if io are functional
	toggleIO_redled(); 
	toggleIO_yellowled();
	forDelay(150);
	toggleIO_redled();
	toggleIO_yellowled();
	
	clearRxBuffer();
	UCSR1B |= (1 << TXCIE1); //enable tx interrupt
	printOptions();
	while(!go);
	systemSetup(); 
	 
	sei();
	for(;;) {
		// set global flag indicating in the process of reading time_ms
		cli();
		fInRead = 1;
		sei();
		
		if (toggleTime <= time_ms) { //RED LED TASK			
			if(toggleTime < time_ms)
				missDeadline++;
			cli();
			fInRead = 0;
			sei();
			toggleIO_redled(); //toogle 10HZ				
			toggleTime = time_ms + redPeriod;
			jobNumb++;
			
		}else if(releaseJitter){
			cli();
			fInRead = 0;
			sei();
			PORTC ^= (1 << PORTC7); //turn on yellow led	
			forDelay(5);// 5 ms delay busy wait
			PORTC ^= (1 << PORTC7); //turn off yellow led			
			releaseJitter = 0;
			jobNumb++;
			
		}else if (releaseHough){ //close to 50 ms
			cli();
			fInRead = 0;
			sei();			
			houghTransform( (uint16_t) &red, (uint16_t) &green, (uint16_t) &blue );			
			releaseHough = 0;
			jobNumb++;			
		}
		if(experiment != 0 && time_ms > 30000){//15 seconds storing experiment data.
			pauseSystem();
			clearRxBuffer();
			setupUART();
			UCSR1B |= (1 << TXCIE1); //enable tx interrupt
			printExperimentData(time_ms);
			confMenu = 1; //system is in configuration mode.
			while(confMenu); //system stay here until user press quit in the terminal			
			restartSystem();				
		}
		
	}
}

//1000 hz timer
ISR(TIMER0_COMPA_vect){ 
	if ( fInRead ) bufferConflict++; // Time Tracker that checks for race condition
  time_ms++;
  if(time_ms % 100 == 0){ //10hz
  	  if(releaseHough == 1){ //if it is still inside hough
  		missDeadline++;
  	  }
	  releaseHough = 1; //releaseHough
  }
      
}

//40 hz timer
ISR(TIMER3_COMPA_vect){ 		
	
	if(yellowtoggleTime <= time_40hz){	
		if(experiment == '8'){		
			sei();
			forDelay(105);
		}else if (experiment == '3'){		
			forDelay(20);
		}else if (experiment == '5'){
			forDelay(30);
		}else if (experiment == '7'){
			forDelay(105);
		}
		yellowtoggleTime = time_40hz + yellowIOPeriod;	
		toggleIO_yellowled(); //toggle yellow led 10hz
									
	}
	
	randNum = rand() % 5; //random number generation 0-4
	if(randNum == 4){
		if(releaseJitter == 1) //if it is still inside the jitter code
			missDeadline++;
		releaseJitter = 1;	
	}		
	time_40hz++;
}

ISR(TIMER1_COMPB_vect){
	greenToogles++;
	
	if(experiment == '2'){
		forDelay(20);
	}else if (experiment == '4'){
		forDelay(30);
	}else if (experiment == '6'){
		forDelay(105);
	}
}

// When the user releases the button, output race condition data
ISR(PCINT0_vect) {
	
	// if buttonA released, and the system isn't in configuration mode.
	if (PINB & (1 << PINB3) && confMenu == 0){
		pauseSystem();
		clearRxBuffer();
		setupUART();
		UCSR1B |= (1 << TXCIE1); //enable tx interrupt
		_delay_ms(2); //to print correctly
		sendString("\r\n-----------Configuration Mode-----------\r\n");
		_delay_ms(10); //delay to print correctly
		confMenu = 1; //system is in configuration mode.
		while(confMenu); //system stay here until user press quit in the terminal			
		restartSystem();
	}
}

ISR(__vector_default){}