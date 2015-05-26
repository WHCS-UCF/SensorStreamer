/*
 * SensorStreamer.cpp
 *
 * Created: 5/26/2015 2:05:02 PM
 *  Author: Jimmy
 */ 


/*
	The goal of this program is to continuously read the temperature from a tmp36 temperature module.
	The program assumes that Vout of the sensor is connected to ADC pin 3 of the ATMEGA88A.
	The ADC initialize function assumes that you are supplying AREF with VCC.
	This program relies upon the MEGA88A_UART_LIBRARY for displaying the temperature| https://github.com/WHCS-UCF/MEGA88A_UART_LIBRARY
*/

#include <avr/io.h>
#include <stdlib.h>
#include <string.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include "MEGA88A_UART_LIBRARY.h"

volatile bool timeToPerformARead = true;

//Initializes an LED attached to Port B pin 0.
void LED_Init() {
	DDRB |= (1<<DDB0);
	PORTB|= (1<<DDB0);
}

void LED_blink() {
	PORTB ^= (1<<DDB0);
	_delay_ms(1000);
	PORTB ^= (1<<DDB0);	
}

void startup_indication_routine() {
	int i = 0;
	for(i=0;i<3;i++){
		LED_blink();
		_delay_ms(1000);
	}
}

void adc_init() {
	// AREF = Vcc
	ADMUX = (0<<REFS0);
	
	// ADC Enable and prescaler of 128
	// 1000000/128
	ADCSRA = (1<<ADEN)|(1<<ADPS2)|(1<<ADPS1)|(1<<ADPS0);
}

uint16_t adc_read(uint8_t ch) {
	// select the corresponding channel 0~7
	// ANDing with ’7? will always keep the value
	// of ‘ch’ between 0 and 7
	ch &= 0b00000111;  // AND operation with 7
	ADMUX = (ADMUX & 0xF8)|ch; // clears the bottom 3 bits before ORing
	
	// start single convertion
	// write ’1? to ADSC
	ADCSRA |= (1<<ADSC);
	
	// wait for conversion to complete
	// ADSC becomes ’0? again
	// till then, run loop continuously
	while(ADCSRA & (1<<ADSC));
	
	return (ADC);
}

void timer_1_init() {
	TCCR1B |= ( 1 << CS11 ); //Enable timer 1 (16 bit timer) with prescale value of 8
	TIMSK1 |= ( 1 << TOIE1 ); //Enable overflow interrupt for Timer 1.
}

ISR(TIMER1_OVF_vect) {
	timeToPerformARead = true;
}

int main(void) {
	int temperature;
	char temperatureString[16];
	
	adc_init();
	initUart();
	LED_Init();
	timer_1_init();
	sei(); //enable interrupts
	
	startup_indication_routine();
	
    while(1)
    {
		if(timeToPerformARead) {
			LED_blink();
			temperature = adc_read(3);
			strcpy(temperatureString, itoa(temperature, temperatureString, 10));
		
			USART_sendString(temperatureString);
			USART_sendString("\n\r");
			
			timeToPerformARead = false;
		}		
    }
}