
/*
 * spi.h
 *
 * Created: 16.03.2021 19:37:01
 *  Author: kenan
 */ 

#ifndef SPI_H
#define SPI_H

#include <avr/io.h>

void init_spi();

uint8_t spi_transfer_byte(uint8_t data);

void spi_transfer(uint8_t *data, uint8_t length);

void close_spi();

#endif