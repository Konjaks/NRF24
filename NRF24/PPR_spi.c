
/*
 * spi.c
 *
 * Created: 16.03.2021 19:35:30
 *  Author: kenan
 */ 
#define F_CPU 16000000UL
#include "PPR_spi.h"
#include <avr/io.h>
#include <util/delay.h>


void init_spi()
{
	//Init SPI, MSBFirst, Mode0, CLKDiv4
	SPCR = (0<<SPIE) | (1<<SPE) | (0<<DORD) | (1<<MSTR) | (0<<CPOL) | (0<<CPHA) | (0<<SPR1) | (0<<SPR0);
	
	//enable double speed SPI
	SPSR |= (1<<SPI2X);
	
	//CSN Pin (PB0), SCK (PB1), MOSI (PB2) as Output
	DDRB |= (1<<DDB0) | (1<<DDB1) | (1<<DDB2);
	
	//CSN HIGH
	PORTB |= (1<<PB0);
	
	//MISO (PB3) as Input
	DDRB &= ~(1<<DDB3);
	
	//CE (PL1) as Output
	DDRL |= (1<<DDL1);
	
	//CE (PL1) LOW
	PORTL &= ~(1<<PL1);
	
	//Output or Pullup OFF
	PORTB &= ~((1<<PB1) | (1<<PB2) | (1<<PB3));
}

uint8_t spi_transfer_byte(uint8_t data)
{
	uint16_t timeout = 0;
	SPDR = data;
	while((!(SPSR & (1<<SPIF)) & (timeout <= 65000))) {timeout++;};
	return SPDR;
}

void spi_transfer(uint8_t *data, uint8_t length)
{
	for (uint8_t i = 0; i < length; i++)
	{
		uint16_t timeout = 0;
		SPDR = data[i];
		while((!(SPSR & (1<<SPIF)) & (timeout <= 65000))) {timeout++;};
	}
}


void close_spi()
{
	//Close SPI
	SPCR = 0;
	
	//CE as output (PL1), Low
	DDRL |= (1<<DDL1);
	PORTL &= ~(1<<PL1);
	
	//MISO (PB3), !SS/CSN Pin (PB0), SCK (PB1), MOSI (PB2) as Input, High impedance
	DDRB &= ~((1<<DDB3) | (1<<DDB0) | (1<<DDB1) | (1<<DDB2));
	PORTB &= ~((1<<PB3) | (1<<PB0) | (1<<PB1) | (1<<PB2));
}