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
#define printDelay 10
//control variables
int32_t Vm, Pr, Pm, T;
float Kd, Kp;
uint8_t startLog, startTraj;

typedef struct {
	volatile uint8_t *buffer;
	uint16_t length;	// current number of bytes in buffer
	uint8_t front;
	uint8_t back;
} uart_fifo;

uint16_t convertEncoder_degree(uint16_t encod);
void clearRxBuffer(void);
void sendChar(uint8_t c);
void disableTxUARTinterrupt(void);
void setupUART(void);
void bufferInit(volatile uint8_t *buf, volatile uart_fifo *f);
// return -1 if buffer is full
int8_t sendString(uint8_t *s);

#endif
