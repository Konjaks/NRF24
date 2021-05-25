
/*
 * Konjak_PPR_NRF24.h
 * 
 * Library for communication with a NRF24L01 Transreceiver 
 *
 * Author: kenan
 * Created: 16.03.2021 20:22:14 - 23.03 18:23, Vers 1.0
 */ 
#include <avr/io.h>
#include "nRF24L01.h"
#include "PPR_spi.h"

#ifndef PPR_NRF24_H
#define PPR_NRF24_H

//set the channel of the NRF24L01 module
uint8_t NRF24_init();

//0 - 125
void NRF24_set_Channel(uint8_t channel);

uint8_t NRF24_get_Channel();

//0 - > 250KBPS, 1 -> 1MBPS, 2 -> 2MBPS
uint8_t NRF24_set_Data_Rate(uint8_t Rate);

uint8_t NRF24_get_Data_Rate();

//level: 0 -> Min, 1 -> medium, 2 -> High, 3 -> MAX |  lna Enable or Disable
void NRF24_setPALevel(uint8_t level, uint8_t bool_lna_Enable);

uint8_t NRF24_getPALevel();

uint8_t NRF24_is_Connected();

void NRF24_setRetries(uint8_t delay, uint8_t count);

void NRF24_start_const_Carrier(uint8_t level, uint8_t channel);

void NRF24_stop_const_Carrier();

void NRF24_PowerDown();

void NRF24_PowerUP();

void NRF24_setAutoAck(uint8_t boool);

uint8_t NRF24_test_carrier();

void NRF24_openWritingPipe(const uint8_t *adress, uint8_t width);

uint8_t NRF24_get_PayloadSize();

//only Pipe 0
void NRF24_set_PayloadSize(uint8_t payload);

void NRF24_setAdressSize(uint8_t size);

//only pipe 1 - 5
void NRF24_openReceivingPipe(uint8_t pipe, uint8_t *adress, uint8_t width);

void NRF24_startListen();

void NRF24_stopListen();

uint8_t NRF24_Packet_available();

void NRF24_read(void *rec, uint8_t width);

uint8_t NRF24_send(const void *send, uint8_t width, uint8_t disable_ack);

void NRF24_Write_Fast_starting(const void* buffer, uint8_t width, uint8_t disable_ack, uint8_t send_with_CE);



//should be privated//
void write_registerss_NRF24(uint8_t regist, uint8_t *value, uint8_t length);

uint8_t write_register_NRF24(uint8_t regist, uint8_t value);

uint8_t read_register_NRF24(uint8_t data);

uint8_t NRF24_state();

void flush_RX();

void flush_TX();

void toggle_features();

void NRF24_delay_us(uint8_t delay);

uint8_t write_payload_NRF24(const void* buf, uint8_t width, const uint8_t type);

void NRF24_CSN(uint8_t high);

//#define NRF24_Config ((1<<MASK_RX_DR) | (1<<EN_CRC) | (0<<CRCO))
//#define TX_POWERUP write_register_NRF24(CONFIG, NRF24_Config | ((1<<PWR_UP) | (0<<PRIM_RX)))
//#define RX_POWERUP write_register_NRF24(CONFIG, NRF24_Config | ((1<<PWR_UP) | (1<<PRIM_RX)))


#define NRF24_250KBPS 0
#define NRF24_1MBPS 1
#define NRF24_2MBPS 2

#endif