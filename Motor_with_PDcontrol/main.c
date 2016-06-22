
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "uart.h"

uint32_t time_ms;
volatile uint16_t traj;
uint16_t printLog;
uint16_t startControler;
#define MaxTorque 4496 //

#define MinDcycle 0.04
#define MaxDcycle 0.50
//pwm and encoder variables
volatile uint8_t direction = 0; // for calibration

static char global_last_m1a_val;
static char global_last_m1b_val;
volatile int64_t global_counts_m1;
volatile int64_t last_global_counts_m1;
static char global_error_m1;

inline void setupTimer0(){//5000hz
	time_ms = 0;
	TCCR0A |= (1 << WGM01);    // Enables CTC mode for timer0
	TCCR0B |= (1 << CS01);     // For setting the timer offset value to 64
	TCCR0B |= (1 << CS00);     // For setting the timer offset value to 64
	OCR0A = 49;               // Sets the counter to 249

}

volatile int16_t LastPm;
//encoder and pwm logic

inline void setDcyclePwm(float duCycle){
	 uint32_t t = (float)255*(float)duCycle;
	 OCR1B = roundf(t);
}

inline void goForward(){
	if(direction){
		PORTE |= (1 << PORTE2); //setting 1
	}else{
		PORTE &= ~(1 << PORTE2); //setting 0
	}
	float relativeTorque = (float)T/(float)MaxTorque;
	if(relativeTorque < MinDcycle && relativeTorque > 0.01){
		setDcyclePwm(MinDcycle);
	}else if(relativeTorque >= MaxDcycle){
		setDcyclePwm(MaxDcycle);
	}else{
		setDcyclePwm(relativeTorque);
	}
}

inline void goReverse(){
	if(direction){
		PORTE &= ~(1 << PORTE2); //setting 0
	}else{
		PORTE |= (1 << PORTE2); //setting 1
	}
	float relativeTorque = (float)abs(T)/(float)MaxTorque;
	if(relativeTorque < MinDcycle && relativeTorque > 0.01){
		setDcyclePwm(MinDcycle);
	}else if(relativeTorque >= MaxDcycle){
		setDcyclePwm(MaxDcycle);
	}else{
		setDcyclePwm(relativeTorque);
	}
}

inline void setupTimer1(){
  DDRB |= (1 << DDB6); // o/i
  TCCR1A |= (1<<WGM10)|(1<<COM1B1); // pwm 8 bits
  TCCR1B |=  (1 << CS10); //1 prescale
}

inline void calibrationRoutine(){
	setDcyclePwm(0.7);
	PORTE &= ~(1 << PORTE2); //setting 0
	_delay_ms(4);
	cli();
	if(global_counts_m1 > 0){
		direction = 0;
		setDcyclePwm(0);
		global_counts_m1 = 0;
	}else{
		direction = 1;
		setDcyclePwm(0);
		global_counts_m1 = 0;
	}
	sei();
}

uint16_t convertDegree_encoder(uint16_t degree){ 		//2248 = 360
	uint16_t t = roundf((float)2248*(float)degree/(float)360);
	return t;
}

inline void encoderInit(){
	global_last_m1a_val = (PINB & (1 << PINB5)) >> PINB5;
	global_last_m1b_val = (PINB & (1 << PINB4)) >> PINB4;
	global_counts_m1 = 0;
	global_error_m1 = 0;
	PCICR |= (1 << PCIE0);
	PCMSK0 |= (1 << PCINT4); //enable encoder interruption
	PCMSK0 |= (1 << PCINT5);
}

inline void setupLeds(){
	DDRC |= (1 << DDC7);	// yellow led
	DDRD |= (1 << DDD5);	// green led
}

//PD controler
inline void PD_controler(){
		if(T >= 0){
			goForward();
		}
		else{
			goReverse();
		}
}

int main(void){

	USBCON = 0;	// dont have to reset chip after flashing
	setupLeds();
	setupTimer1();
	setupTimer0();
	encoderInit();
	setupUART();
	sei();
	calibrationRoutine();
	Kp = 30;
	Kd = 300;
	char s[100];
	sendString("Enter h to see all the options\r\n");
	_delay_ms(printDelay); //delay to print correctly
	TIMSK0 |= (1 << OCIE0A); // Enable time keeper (timer0)
	for(;;) {
			if(startControler){
				Vm = abs(global_counts_m1 - last_global_counts_m1);
				T = Kp * (Pr - global_counts_m1) - Kd * Vm;
				PD_controler();
				last_global_counts_m1 = global_counts_m1;
				startControler = 0;
			}
			else if(printLog){
				sprintf(s,"%d, %d, %d\r\n", (int) convertEncoder_degree(abs(Pr)), convertEncoder_degree(abs(global_counts_m1)), (int) T);
				sendString(s);
				_delay_ms(5);
				printLog = 0;
			}
			if(startTraj){
				switch (traj) {
					case 0:
						startLog = 1;
						Pr = Pr + convertDegree_encoder(90); //rotate 90
						traj++;
					break;
					case 2:
						Pr = Pr - convertDegree_encoder(360); //rotate reverse 360
						traj++;
					break;
					case 4:
						Pr = Pr + convertDegree_encoder(5); //rotate forward 5
						traj++;
					break;
					case 5:
						if(Pr == Pm){
							traj = 0;
							startTraj = 0;
							startLog = 0;
						}
					default:
						if(Pr == Pm){
							_delay_ms(500); //hold 0.5
							traj++;
						}
					break;
				}

			}

	}
}

ISR(TIMER0_COMPA_vect){ //5000 Hz
	time_ms++;
	startControler = 1;
	if(Vm != 0 && startLog == 1 && time_ms % 50 == 0){
		printLog = 1;
	}
}

// When the user releases the button, output race condition data
ISR(PCINT0_vect) {

		unsigned char m1a_val = (PINB & (1 << PINB5)) >> PINB5;
		unsigned char m1b_val = (PINB & (1 << PINB4)) >> PINB4;
		char plus_m1 = m1a_val ^ global_last_m1b_val;
		char minus_m1 = m1b_val ^ global_last_m1a_val;

		if(plus_m1){
			global_counts_m1 += 1;
		}
		if(minus_m1){
			global_counts_m1 -= 1;
		}
		Pm = global_counts_m1; //measured value
		if(m1a_val != global_last_m1a_val && m1b_val != global_last_m1b_val)
			global_error_m1 = 1;

		global_last_m1a_val = m1a_val;
		global_last_m1b_val = m1b_val;

}

ISR(__vector_default){}
