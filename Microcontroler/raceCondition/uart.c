#include "uart.h"


volatile uint8_t TX_BUFFER[MAX_BUFFER_SIZE];
volatile uint8_t RX_BUFFER[MAX_BUFFER_SIZE];

volatile uart_fifo txFIFO;
volatile uart_fifo rxFIFO;

char checkChar(){
	char frontChar = '\0';
	
	if(rxFIFO.front != rxFIFO.back){
		frontChar = rxFIFO.buffer[rxFIFO.front];
	}
	return frontChar;
}

inline void resetVariables(){
	bufferConflict = 0;
	missDeadline = 0;
	jobNumb = 0;
	greenToogles = 0; //number of toogles of the green led
}

//set pwm, prescale = 1024 //
void setPwm(uint32_t freq){
	 uint32_t t = ((float)16000000 - (1024 * freq))/(float)(1024 * freq);
	 OCR1B = roundf(t);
}

inline void printExperimentData(uint64_t time_ms){
	char s[100];
	sprintf(s,"Time: %d ms. Conflicts: %d. Missed deadlines: %d. Job releases: %d. Green led Toogles: %d\r\n", (int) time_ms,(int) bufferConflict, 																			                (int) missDeadline, (int) jobNumb, (int) greenToogles);
	sendString(s);
	_delay_ms(400); //delay to print correctly
}

inline void startExperiment(){
	switch(experiment){
		case 1: //Setting everything to 2hz
		redPeriod = 500; // 2hz
		yellowIOPeriod = 20;  //2hz
	    setPwm(2); //2hz pwm
	    break;
		case 4: //experiment 4 || 5 
		case 5:
		redPeriod = 100; // 10hz
		yellowIOPeriod = 4;  //10hz
	    setPwm(10); //10hz pwm
	    break;
		case 6: //experiment 6 || 7 || 8
		case 7:
		case 8:
		redPeriod = 100; // 10hz
		yellowIOPeriod = 4;  //10hz
	    setPwm(10); //10hz pwm
	    break;
		default: //experiment 2 || 3 and when there is no experiment setted. everything to 10hz
		redPeriod = 100; // 10hz
		yellowIOPeriod = 4;  //10hz
	    setPwm(10); //10hz pwm
		break;  
	}
	
	confMenu = 0; //flag to quit menu
}

void confgMenu(void){
	cli();
	//Check what is the last char on the buffer
	char frontChar = checkChar();
	
	//Parameters from the user
	char param[4];
	
	//Green LED new freq
	int newFreq = 0;
	
	//responsible for handling only char options (no param, e.g. p,z,g)
	if(rxFIFO.length == 1 && go != 0){
		switch(frontChar){
			case 'p':
			sendString("---------- Data Collected -----------\r\n");
			_delay_ms(10); //delay to print correctly
			char s[100];
			sprintf(s,"Conflicts: %d. Missed deadlines: %d. Job releases: %d. Green led Toogles: %d\r\n", (int) bufferConflict, (int) missDeadline, (int) jobNumb, (int) greenToogles);
			sendString(s);
			_delay_ms(10); //delay to print correctly
			clearRxBuffer();
			break;
			case 'z': 
			sendString("---------- Reseting Variables -----------\r\n");
			_delay_ms(10); //delay to print correctly
			resetVariables();
			clearRxBuffer(); 
			break;
			case 'g': 
			sendString("---------- Starting Experiment -----------\r\n");
			_delay_ms(10); //delay to print correctly
			clearRxBuffer();
			startExperiment(); //start experiment and quit
			break;
			case 'q': 
			sendString("------- Quiting Configuration Menu -------\r\n"); //quit case.
			_delay_ms(10); //delay to print correctly
			clearRxBuffer();
			experiment = 0;
			confMenu = 0; //flag to quit menu
			break;
		}
	}
	//responsible for handling strings options
	else if(rxFIFO.length > 1 && go != 0){
		switch(frontChar){
			case 'e':
			if(rxFIFO.length == 2){ //just one experiment param
				int n;
				for(n = 0; n < 2; n++){
					param[n] = rxFIFO.buffer[rxFIFO.front];
					rxFIFO.front++;
				}
				if(param [1] >= 48 && param [1] <= 57){ //numeric
					char s[200];
					sprintf(s,"---------- Setting Experiment #:%c ---------- \r\n", param[1]);
					sendString(s);
					_delay_ms(10); //delay to print correctly
					experiment = param[1]; //setting experiment number
					clearRxBuffer();
				}else{
					char s[200];
					sprintf(s,"---------- Invalid Parameter---------- \r\n");
					sendString(s);
					_delay_ms(10); //delay to print correctly
					clearRxBuffer();
				}
			}
			
			break;
			case 'r':
			if(rxFIFO.length > 4){
				char s[200];
				int n = 0;
				rxFIFO.front++; //first parameter 'r'
				while(n < 5){
					if(rxFIFO.buffer[rxFIFO.front] >= 48 && rxFIFO.buffer[rxFIFO.front] <= 57 ){ //numeric parameter ####
						param[n] = rxFIFO.buffer[rxFIFO.front];						
					}else{ //invalid parameter
						break;
					}
					n++;
					rxFIFO.front++;
				}
				
				if(n < 4){
					sendString("---------- Invalid Parameter----------\r\n"); //some invalid parameter was found
					_delay_ms(10); //delay to print correctly
					
				}else{
					newFreq = atoi(param);
					sprintf(s,"---------- SETTING GREEN LED PERIOD TO: %d ms ---------- \r\n", (int)newFreq);
					sendString(s);
					_delay_ms(10); //delay to print correctly
					setPwm((float)1/newFreq);
				}
				clearRxBuffer();
			}
			break;
			default: 
			sendString("---------- Invalid Parameter----------\r\n");
			_delay_ms(10); //delay to print correctly
			clearRxBuffer();
			break;
		}
	}
	else if(rxFIFO.length > 1 && go == 0){
		int n;
		switch(frontChar){
			case 'g':
				
				for(n = 0; n < 2; n++){
					param[n] = rxFIFO.buffer[rxFIFO.front];
					rxFIFO.front++;
				}
				if(param[1] == 'o'){
					clearRxBuffer();
					sendString("Starting\r\n");
					_delay_ms(5);
					go = 1;
				}else{
					clearRxBuffer();
					sendString("Type go to start\r\n");
					_delay_ms(10);
				}
			break;
			default: 
			clearRxBuffer();
			sendString("Type go to start\r\n");
			_delay_ms(10);
			break;
		}	
	}
	sei();
}

void clearRxBuffer(void){
	int n;
	for(n=0; n<(int)rxFIFO.length; n++){
		rxFIFO.buffer[n] = '\0';
	}
	rxFIFO.back = 0;
	rxFIFO.front = 0;
	rxFIFO.length = 0;
}

ISR(USART1_RX_vect) {	
	rxFIFO.buffer[rxFIFO.back] = UDR1;
	rxFIFO.back++;
	if(rxFIFO.back >= MAX_BUFFER_SIZE)
	rxFIFO.back = 0;
	rxFIFO.length++;	
	
	confgMenu();	
}

ISR(USART1_TX_vect) {
	
	if(txFIFO.length == 0) {	// buffer is empty so return
		return;
	}
	if(UCSR1A & (1<<UDRE1)) {	// ensure data register is empty
		sendChar(txFIFO.buffer[txFIFO.front++]);
		txFIFO.length--;
		txFIFO.front &= (MAX_BUFFER_SIZE - 1); // wrap around back to 0
	}

}

void bufferInit(volatile uint8_t *buf, volatile uart_fifo *f) {
	f->buffer = buf;
	f->front = 0;
	f->back = 0;
	f->length = 0;
}

/*
Setup UART1 for 57.6k Baud Rate
using transmit and receive interrupts
*/
void setupUART(void) {
	bufferInit(TX_BUFFER, &txFIFO);
	bufferInit(RX_BUFFER, &rxFIFO);
	
	//UBRR = fosc / (16 * baud) - 1

	UBRR1 = ((F_CPU/(16*57600)) - 1);
	UCSR1C |= (1 << UCSZ11) | (1 << UCSZ10);		// 8 bit char size
	UCSR1B |= (1 << RXCIE1);	// enable receive interrupt
	UCSR1B |= (1 << RXEN1);		// enable receive
	UCSR1B |= (1 << TXEN1);		// enable transmit
}

void disableTxUARTinterrupt(void) {
	
	UCSR1B &= ~(1 << TXCIE1);	// disable transmit interrupt
}

// return -1 if buffer is full
int8_t sendString(uint8_t *s) {
	cli();
	if((txFIFO.length + sizeof(s)) > MAX_BUFFER_SIZE) { return -1; }	// not enough room in buffer

	while(*s != 0x00) {	// put string to send in buffer
		txFIFO.buffer[txFIFO.back++] = *s++;
		txFIFO.length++;
		txFIFO.back &= (MAX_BUFFER_SIZE - 1); // wrap around back to 0
	}

	if(UCSR1A & (1<<UDRE1)) {	// if data register is empty
		sendChar(txFIFO.buffer[txFIFO.front++]);
		txFIFO.length--;
		txFIFO.front &= (MAX_BUFFER_SIZE - 1); // wrap around back to 0
	}
	sei();
	return 0;
}

void sendChar(uint8_t c) {
	UDR1 = c;
}

