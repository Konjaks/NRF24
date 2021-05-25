
/*
 * PPR_NRF24.c
 *
 * Created: 16.03.2021 20:22:01
 *  Author: kenan
 */ 

#include "PPR_NRF24.h"
#include <avr/io.h>
#define F_CPU 16000000UL
#include <util/delay.h>
#include "../Uart.h"


uint8_t txDelay = 85;
uint8_t payload_size = 32;
uint8_t acknowledge_enabled = 0;
uint8_t dynamic_payload_size_enabled = 0;
uint8_t buffer_Writing_Pipe[5] = {0};

uint8_t NRF24_init()
{
	//init SPi
	init_spi();
		
	//CE (PL1) LOW
	NRF24_CE(0x00);
		
	//CSN (PB0) HIGH
	NRF24_CSN(0x01);
	
	//delay a bit 
	_delay_ms(107);
	
	write_register_NRF24(CONFIG, 0x0C);
	
	NRF24_setRetries(5, 15);
	
	NRF24_set_Data_Rate(NRF24_1MBPS);
	
	toggle_features();
	write_register_NRF24(FEATURE, 0);
	write_register_NRF24(DYNPD, 0);
	
	write_register_NRF24(STATUS, (RX_DR | TX_DS | MAX_RT));
	
	NRF24_set_Channel(76);
	
	flush_RX();
	flush_TX();
	
	NRF24_PowerUP();
	
	write_register_NRF24(CONFIG, read_register_NRF24(CONFIG) & ~PRIM_RX);
	
	return read_register_NRF24(RF_SETUP);
}

uint8_t write_register_NRF24(uint8_t regist, uint8_t value)
{
	uint8_t back = 0;
	//CSN pin LOW
	NRF24_CSN(0x00);
	
	back = spi_transfer_byte(W_REGISTER | (REGISTER_MASK & regist));
	spi_transfer_byte(value);
	//CSN pin HIGH
	NRF24_CSN(0x01);
	
	return back;
}

void write_registerss_NRF24(uint8_t regist, uint8_t *value, uint8_t length)
{
	//CSN pin LOW
	NRF24_CSN(0x00);
	
	spi_transfer_byte(W_REGISTER | (REGISTER_MASK & regist));
	spi_transfer(value, length);
	//CSN pin HIGH
	NRF24_CSN(0x01);
}

void NRF24_set_Channel(uint8_t channel)
{
	if (channel > 125)
	{
		write_register_NRF24(RF_CH, 125);
		return;
	}
	
	write_register_NRF24(RF_CH, channel);
}

uint8_t read_register_NRF24(uint8_t data)
{
	uint8_t buf = 0;
	
	//CSN pin LOW
	NRF24_CSN(0x00);
	
	spi_transfer_byte(R_REGISTER | (REGISTER_MASK & data));
	buf = spi_transfer_byte(0xFF);
	
	//CSN pin HIGH
	NRF24_CSN(0x01);
	
	return buf;
}

uint8_t NRF24_get_Channel()
{
	return read_register_NRF24(RF_CH);
}

uint8_t NRF24_set_Data_Rate(uint8_t Rate)
{
	uint8_t Data_Rate = 0;
	Data_Rate = read_register_NRF24(RF_SETUP);
	
	//default 1Mbps
	Data_Rate &= ~(RF_DR_LOW | RF_DR_HIGH);
	
	txDelay = 85;
	
	if (Rate == NRF24_250KBPS)
	{
		Data_Rate |= RF_DR_LOW;
		txDelay = 155;
	}
	else if (Rate == NRF24_2MBPS)
	{
		Data_Rate |= RF_DR_HIGH;
		txDelay = 65;
	}
	
	write_register_NRF24(RF_SETUP, Data_Rate);
	
	if (read_register_NRF24(RF_SETUP) == Data_Rate)
	{
		return 0x01;
	}
	
	return 0x00;
}


uint8_t NRF24_get_Data_Rate()
{
	uint8_t result = 0;
	
	result = read_register_NRF24(RF_SETUP);
	result &= (RF_DR_LOW | RF_DR_HIGH);
	
	if (result == RF_DR_LOW)
	{
		return NRF24_250KBPS;
	}
	else if (result == RF_DR_HIGH)
	{
		return NRF24_1MBPS;
	}
	else
	{
		return NRF24_2MBPS;
	}
}

void NRF24_setPALevel(uint8_t level, uint8_t bool_lna_Enable)
{
	uint8_t data = 0;
	data = read_register_NRF24(RF_SETUP) & 0xF8;
	
	if (level > 3)
	{
		level = 3;
	}
	
	data |= ((level<<1) | (bool_lna_Enable & 0x01));
	
	write_register_NRF24(RF_SETUP, data);
}

uint8_t NRF24_getPALevel()
{
	return read_register_NRF24(RF_SETUP) & (RF_PWR_LOW | RF_PWR_HIGH) >> 1;
}

uint8_t NRF24_is_Connected()
{
	uint8_t data = 0;
	data = read_register_NRF24(SETUP_AW);
	
	if ((data >= 1) && (data <= 3))
	{
		return 0x01;
	}
	return 0x00;
}

void NRF24_setRetries(uint8_t delay, uint8_t count)
{
	write_register_NRF24(SETUP_RETR, (delay & 0x0F) << ARD | (count & 0x0F) << ARC);
}

void NRF24_start_const_Carrier(uint8_t level, uint8_t channel)
{
	write_register_NRF24(RF_SETUP, (read_register_NRF24(RF_SETUP) | 0x80));
	write_register_NRF24(RF_SETUP, (read_register_NRF24(RF_SETUP) | PLL_LOCK));
	
	NRF24_setPALevel(level, 0);
	NRF24_set_Channel(channel);
	
	//CE HIGH
	NRF24_CE(0x01);
}

void NRF24_stop_const_Carrier()
{
	write_register_NRF24(RF_SETUP, (read_register_NRF24(RF_SETUP) & ~(0x80)));
	write_register_NRF24(RF_SETUP, (read_register_NRF24(RF_SETUP) & ~(PLL_LOCK)));
		
	//CE LOW
	NRF24_CE(0x00);
}

void toggle_features()
{
	//CSN LOW
	NRF24_CSN(0x00);
	spi_transfer_byte(ACTIVATE);
	spi_transfer_byte(ACTIVATE_2);
	//CSN HIGH
	NRF24_CSN(0x01);
}

void flush_RX()
{
	//CSN LOW
	NRF24_CSN(0x00);
	spi_transfer_byte(FLUSH_RX);
	//CSN HIGH
	NRF24_CSN(0x01);
}

void flush_TX()
{
	//CSN LOW
	NRF24_CSN(0x00);
	spi_transfer_byte(FLUSH_TX);
	//CSN HIGH
	NRF24_CSN(0x01);
}

void NRF24_PowerUP()
{
	if (!(read_register_NRF24(CONFIG) & PWR_UP))
	{
		write_register_NRF24(CONFIG, read_register_NRF24(CONFIG) | PWR_UP);
		_delay_ms(6);		
	}
}

void NRF24_PowerDown()
{
	//CE Low
	NRF24_CE(0x00);
	write_register_NRF24(CONFIG, read_register_NRF24(CONFIG) & ~PWR_UP);
}

void NRF24_setAutoAck(uint8_t boool)
{
	if (boool >= 0x01)
	{
		write_register_NRF24(EN_AA, 0x3F);
		acknowledge_enabled = 1;
	}
	else
	{
		write_register_NRF24(EN_AA, 0x00);
		acknowledge_enabled = 0;
	}
}

uint8_t NRF24_test_carrier()
{
	return (read_register_NRF24(CD) & 0x01);
}

void NRF24_setAdressSize(uint8_t size)
{
	if (size -= 2)
	{
		write_register_NRF24(SETUP_AW, size % 4);
	}
	else
	{
		write_register_NRF24(SETUP_AW, 0);
	}
}

void NRF24_openWritingPipe(const uint8_t *adress, uint8_t width)
{	
	write_registerss_NRF24(RX_ADDR_P0, adress, width);
	write_registerss_NRF24(TX_ADDR, adress, width);
	
	write_register_NRF24(RX_PW_P0, payload_size);
}

void NRF24_set_PayloadSize(uint8_t payload)
{
	if (payload <= 32)
	{
		payload_size = payload;
	}
	else
	{
		payload_size = 32;
	}
}

uint8_t NRF24_get_PayloadSize()
{
	return payload_size;
}

void NRF24_openReceivingPipe(uint8_t pipe, uint8_t *adress, uint8_t width)
{
	if (pipe <= 5 && pipe != 0)
	{
		switch (pipe)
		{
			case 1:
			write_registerss_NRF24(RX_ADDR_P1, adress, width);
			write_register_NRF24(RX_PW_P1, payload_size);
			write_register_NRF24(EN_RXADDR, read_register_NRF24(EN_RXADDR) | ERX_P1);
			break;
			
			case 2:
			write_registerss_NRF24(RX_ADDR_P2, adress, 1);
			write_register_NRF24(RX_PW_P2, payload_size);
			write_register_NRF24(EN_RXADDR, read_register_NRF24(EN_RXADDR) | ERX_P2);
			break;
			
			case 3:
			write_registerss_NRF24(RX_ADDR_P3, adress, 1);
			write_register_NRF24(RX_PW_P3, payload_size);
			write_register_NRF24(EN_RXADDR, read_register_NRF24(EN_RXADDR) | ERX_P3);
			break;
			
			case 4:
			write_registerss_NRF24(RX_ADDR_P4, adress, 1);
			write_register_NRF24(RX_PW_P4, payload_size);
			write_register_NRF24(EN_RXADDR, read_register_NRF24(EN_RXADDR) | ERX_P4);
			break;
			
			case 5:
			write_registerss_NRF24(RX_ADDR_P5, adress, 1);
			write_register_NRF24(RX_PW_P5, payload_size);
			write_register_NRF24(EN_RXADDR, read_register_NRF24(EN_RXADDR) | ERX_P5);
			break;
			
			default:
				return;
			break;
		}
	}
}

void NRF24_closeReceivingPipe(uint8_t pipe)
{
	switch (pipe)
	{
		case 0:
		write_register_NRF24(EN_RXADDR, read_register_NRF24(EN_RXADDR) & (~ERX_P0));
		break;
		
		case 1:
		write_register_NRF24(EN_RXADDR, read_register_NRF24(EN_RXADDR) & (~ERX_P1));
		break;
		
		case 2:
		write_register_NRF24(EN_RXADDR, read_register_NRF24(EN_RXADDR) & (~ERX_P2));
		break;
		
		case 3:
		write_register_NRF24(EN_RXADDR, read_register_NRF24(EN_RXADDR) & (~ERX_P3));
		break;
		
		case 4:
		write_register_NRF24(EN_RXADDR, read_register_NRF24(EN_RXADDR) & (~ERX_P4));
		break;
		
		case 5:
		write_register_NRF24(EN_RXADDR, read_register_NRF24(EN_RXADDR) & (~ERX_P5));
		break;
		
		default:
		return;
		break;
	}
}

void NRF24_startListen()
{
	//NRF24_PowerUP();
	
	write_register_NRF24(CONFIG, read_register_NRF24(CONFIG) | PRIM_RX);
	write_register_NRF24(STATUS, RX_DR | TX_DS | MAX_RT);
	
	//CE HIGH
	NRF24_CE(0x01);
	
	NRF24_closeReceivingPipe(0);
	
	if (read_register_NRF24(FEATURE) & EN_ACK_PAY)
	{
		flush_TX();
	}
}

void NRF24_stopListen()
{
	//CE LOW
	NRF24_CE(0x00);
	
	NRF24_delay_us(txDelay);
	
	if (read_register_NRF24(FEATURE) & EN_ACK_PAY)
	{
		NRF24_delay_us(txDelay);
		flush_TX();
	}
	
	write_register_NRF24(CONFIG, (read_register_NRF24(CONFIG) & ~(PRIM_RX)));
	
	write_register_NRF24(EN_RXADDR, read_register_NRF24(EN_RXADDR) | ERX_P0); 	
}

uint8_t NRF24_Packet_available()
{
	if (!(read_register_NRF24(FIFO_STATUS) & RX_EMPTY)) 
	{
		/*uint8_t rec = 0;
		//CSN LOW
		PORTL &= ~(1<<PL1);
		rec = spi_transfer_byte(0xFF);
		//CSN HIGH
		PORTL |= (1<<PL1);
		*/
		return 1;
	}
	else
	{
		return 0;	
	}
}


uint8_t buffer_Rec[33] = {0};
void NRF24_read(void *rec, uint8_t width)
{
	uint8_t blank_read = 0;
	uint8_t packet_size = 0;
	uint8_t *local = (uint8_t*)rec;
	
	if (width > MAX_PAYLOAD_LEN)
	{
		width = MAX_PAYLOAD_LEN;
	}
	
	if (dynamic_payload_size_enabled == 0)
	{
		if (width > payload_size)
		{
			width = payload_size;
		}
		
		blank_read = payload_size - width;
	}
	
	packet_size = width + blank_read + 1;
	
	NRF24_CSN(0x00);

	buffer_Rec[0] = spi_transfer_byte(R_RX_PAYLOAD);
	uint8_t x = 0;
	
	
	while (width--)
	{
		*local++ = spi_transfer_byte(0xFF);
	}
	
	while (blank_read--)
	spi_transfer_byte(0xFF);
	
	NRF24_CSN(0x01);
	
	write_register_NRF24(STATUS, RX_DR | MAX_RT | TX_DS);
}

void NRF24_delay_us(uint8_t delay)
{
	 while(delay > 0) 
	 {
		 _delay_us(1);
		 delay--;
	 }
}

uint8_t NRF24_state()
{
	uint8_t state = 0;
	NRF24_CSN(0x00);
	state = spi_transfer_byte(0xFF);
	NRF24_CSN(0x01);
	return state;
}


uint8_t NRF24_send(const void *send, uint8_t width, const uint8_t disable_ack)
{
	NRF24_Write_Fast_starting(send, width, disable_ack, 1);
	
	uint32_t timeout = 0;
	
	while (!(NRF24_state() & (TX_DS | MAX_RT)))
	{
		if (timeout >= 4500)
		{
			return 0;
		}
		
		timeout++;
	}
	
	//CE LOW
	NRF24_CE(0x00);
	
	uint8_t state = 0;
	state = write_register_NRF24(STATUS, RX_DR | TX_DS | MAX_RT);
	
	if (state & MAX_RT)
	{
		flush_TX();
		return 0;
	}
	
	return 1;
}

void NRF24_Write_Fast_starting(const void* buffer, uint8_t width, const uint8_t disable_ack, const uint8_t send_with_CE)
{
	if (disable_ack == 0x01)
	{
		write_payload_NRF24(buffer, width, W_TX_PAYLOAD_NOACK);
	}
	else
	{
		write_payload_NRF24(buffer, width, W_TX_PAYLOAD);
	}
	
	if (send_with_CE == 0x01)
	{
		NRF24_CE(0x01);
	}
}

uint8_t write_payload_NRF24(const void* buf, uint8_t width, const uint8_t type)
{
	uint8_t blank_read = 0;
	uint8_t packet_size = 0;
	
	uint8_t *local = (const uint8_t*)buf;
	
	
	if (width > MAX_PAYLOAD_LEN)
	{
		width = MAX_PAYLOAD_LEN;
	}
	
	if (dynamic_payload_size_enabled == 0)
	{
		if (width > payload_size)
		{
			width = payload_size;
		}
		
		blank_read = payload_size - width;
	}
	
	NRF24_CSN(0x00);

	buffer_Rec[0] = spi_transfer_byte(type);
	
	while (width--)
	spi_transfer_byte(*local++);
	
		
	while (blank_read--)
	spi_transfer_byte(0x00);
	
	NRF24_CSN(0x01);
	
	return buffer_Rec[0];
}

void NRF24_CSN(uint8_t high)
{
	if (high >= 0x01)
	{
		//CSN HIGH
		PORTB |= (1<<PB0);
		_delay_us(6);
	}
	else
	{
		//CSN LOW
		PORTB &= ~(1<<PB0);
		_delay_us(5);
	}
}

void NRF24_CE(uint8_t high)
{
	if (high >= 0x01)
	{
		//CE HIGH
		PORTL |= (1<<PL1);
	}
	else
	{
		//CE LOW
		PORTL &= ~(1<<PL1);
	}
}