#ifndef UART_H_
#define UART_H_

#ifndef F_CPU
#define F_CPU 16000000ul
#endif

#include <avr/interrupt.h>
#include <avr/io.h>
#include <util/delay.h>
#include <string.h>

#define MAX_BUFFER_SIZE 512

volatile uint64_t bufferConflict;
volatile uint64_t missDeadline;
volatile uint64_t jobNumb;
volatile uint64_t greenToogles; //number of toogles of the green led
volatile uint8_t confMenu;
volatile uint16_t redPeriod;
volatile uint16_t yellowIOPeriod;
volatile uint8_t experiment;
volatile uint8_t go;
typedef struct {
	volatile uint8_t *buffer;
	uint16_t length;	// current number of bytes in buffer
	uint8_t front;
	uint8_t back;
} uart_fifo;

void clearRxBuffer(void);
void sendChar(uint8_t c);
void disableTxUARTinterrupt(void);
void setupUART(void);
void bufferInit(volatile uint8_t *buf, volatile uart_fifo *f);
inline void printExperimentData();
// return -1 if buffer is full
int8_t sendString(uint8_t *s);

#endif