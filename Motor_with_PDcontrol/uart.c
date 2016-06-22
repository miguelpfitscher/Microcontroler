#include "uart.h"


volatile uint8_t TX_BUFFER[MAX_BUFFER_SIZE];
volatile uint8_t RX_BUFFER[MAX_BUFFER_SIZE];

volatile uart_fifo txFIFO;
volatile uart_fifo rxFIFO;

volatile char param[5];

uint16_t convertEncoder_degree(uint16_t encod){
	uint16_t degree = roundf((float)360 * encod /(float)2248);
	return degree;
}

char checkChar(){
	char frontChar = '\0';

	if(rxFIFO.front != rxFIFO.back){
		frontChar = rxFIFO.buffer[rxFIFO.front];
	}
	return frontChar;
}


int parameterTreatment(){
	int n = 0;
	rxFIFO.front++; //first parameter 'P'
	while(n < 4){
		if((rxFIFO.buffer[rxFIFO.front] >= 48 && rxFIFO.buffer[rxFIFO.front] <= 57)){ //numeric parameter ####
			param[n] = rxFIFO.buffer[rxFIFO.front];
			n++;
			rxFIFO.front++;
		}
		else if(rxFIFO.buffer[rxFIFO.front] == '-' && n == 0){
			param[0] = '-';
			n++;
			rxFIFO.front++;
		}
		else{ //invalid parameter
			return 1;
		}
	}
	param[4] = '\0';
	return 0;
}

void UI(void){
	cli();
	//Check what is the last char on the buffer
	char frontChar = checkChar();

	//responsible for handling only char options (no param, e.g. p,z,g)
	if(rxFIFO.length == 1){
		switch(frontChar){
			case 'v':
			sendString("---------- Current Values -----------\r\n");
			_delay_ms(printDelay); //delay to print correctly
			char s[100];
			sprintf(s,"Kd: %d. Kp: %d. Vm: %d. Pr: %d. Pm: %d. T: %d.\r\n", (int) (Kd), (int) Kp, (int) Vm, (int) convertEncoder_degree(Pr), convertEncoder_degree(Pm), (int) T);
			sendString(s);
			_delay_ms(printDelay); //delay to print correctly
			clearRxBuffer();
			break;
			case 't':
			sendString("---------- Executing Trajectory -----------\r\n");
			_delay_ms(printDelay); //delay to print correctly
			startTraj = 1;
			clearRxBuffer();
			break;
			case 'h':
			sendString("---------- v to see the current values -----------\r\n");
			_delay_ms(printDelay); //delay to print correctly
			sendString("---------- t to execute trajectory -----------\r\n");
			_delay_ms(printDelay); //delay to print correctly
			sendString("---------- L to start printing the log -----------\r\n");
			_delay_ms(printDelay); //delay to print correctly
			sendString("---------- l to stop printing the log -----------\r\n");
			_delay_ms(printDelay); //delay to print correctly
			sendString("---------- P#### to increase Kp -----------\r\n");
			_delay_ms(printDelay); //delay to print correctly
			sendString("---------- p#### to decrease Kp -----------\r\n");
			_delay_ms(printDelay); //delay to print correctly
			sendString("---------- D#### to increase Kd -----------\r\n");
			_delay_ms(printDelay); //delay to print correctly
			sendString("---------- d#### to decrease Kd -----------\r\n");
			_delay_ms(printDelay); //delay to print correctly
			sendString("---------- r#### to set a new reference position -----------\r\n");
			_delay_ms(printDelay); //delay to print correctly
			clearRxBuffer();
			break;
			case 'L':
			sendString("---------- Starting printing log -----------\r\n");
			_delay_ms(printDelay); //delay to print correctly
			startLog = 1;
			clearRxBuffer();
			break;
			case 'l':
			sendString("---------- Stoping printing log -----------\r\n");
			_delay_ms(printDelay); //delay to print correctly
			startLog = 0;
			clearRxBuffer();
			break;
		}
	}
	//responsible for handling strings options
	else if(rxFIFO.length > 4){
		switch(frontChar){
			case 'P':
			if(parameterTreatment()){
				sendString("---------- Invalid Parameter----------\r\n"); //some invalid parameter was found

			}else{
				Kp = Kp + atoi(param);
				char s[200];
				sprintf(s,"---------- Increasing Kp to: %d ---------- \r\n", (int)Kp);
				sendString(s);
				_delay_ms(printDelay); //delay to print correctly
			}
			clearRxBuffer();
			break;
			case 'p':

			if(parameterTreatment()){
				sendString("---------- Invalid Parameter----------\r\n"); //some invalid parameter was found

			}else{
				if(Kp > atoi(param))
					Kp = Kp - atoi(param);
				else Kp = 0;
				char s[200];
				sprintf(s,"---------- Decreasing Kp to: %d ---------- \r\n", (int)Kp);
				sendString(s);

			}
			clearRxBuffer();
			break;
			case 'D':
			if(parameterTreatment()){
				sendString("---------- Invalid Parameter----------\r\n"); //some invalid parameter was found

			}else{
				Kd = Kd + (float)atoi(param)/1000;
				char s[200];
				sprintf(s,"---------- Increasing Kd to: %de-3 ---------- \r\n", (int)(Kd*1000));
				sendString(s);
				_delay_ms(printDelay); //delay to print correctly
			}
			clearRxBuffer();
			break;
			case 'd':

			if(parameterTreatment()){
				sendString("---------- Invalid Parameter----------\r\n"); //some invalid parameter was found

			}else{
				if(Kd > (float)atoi(param)/1000)
					Kd = Kd - (float)atoi(param)/1000;
				else Kd = 0;
				char s[200];
				sprintf(s,"---------- Decreasing Kd to: %de-3 ---------- \r\n", (int)(Kd*1000));
				sendString(s);
				_delay_ms(printDelay); //delay to print correctly
			}
			clearRxBuffer();
			break;
			case 'r':

			if(parameterTreatment()){
				sendString("---------- Invalid Parameter----------\r\n"); //some invalid parameter was found

			}else{
				char s[200];
				sprintf(s,"---------- Setting a new reference position to: %d ---------- \r\n", atoi(param));
				sendString(s);
				_delay_ms(printDelay); //delay to print correctly
				Pr = Pr + roundf((float)2248*(float)atoi(param)/(float)360);
			}
			clearRxBuffer();
			break;
			default:
			sendString("---------- Invalid Parameter----------\r\n");

			clearRxBuffer();
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

	UI();
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
	UCSR1B |= (1 << TXCIE1);    //enable tx interrupt
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
