/*
 * example.c
 *
 * Created: 05.04.2021 17:16:21
 * Author : Kenan Ergin
 */ 


//Definition of Clock Speed
#define F_CPU 16000000UL

//including of the libraries
#include <util/delay.h>
#include "NRF24/PPR_NRF24.h"


//definition of sending/receiving addresses
//								Master		C1
const uint8_t adresses[2][6] = { "sende", "KBPPR" };

//definition of start channel
uint8_t used_channel = 120;

//Data struct of Controller 1
typedef struct Send
{
	uint8_t identify;
	uint8_t X;
	uint8_t Y;
	uint8_t Batt;
	uint8_t VCC;
	uint8_t digital;
};

//Data struct from Master
typedef struct Receive
{
	uint8_t channel;
	uint8_t listen;				// adress of controller
	uint8_t masteer;
	uint8_t data;				//  Bit7  Bit6  Bit5  Bit4  Bit3  Bit2  Bit1  Bit0
};								//  nix   nix   nix   nix   nix   nix   nix   Send y/n

struct Send Controller;
struct Receive master;


int main(void)
{
	//(Sending Packet)
	//set values in the structs
	Controller.identify = 93;
	Controller.X = 0;
	Controller.Y = 0;
	Controller.Batt = 0;
	Controller.VCC = 0;
	Controller.digital = 0;

	// Init NRF24L01
	NRF24_init();

	//set Datarate
	NRF24_set_Data_Rate(NRF24_2MBPS);

	//set Restries every 1 * (time constant[mili sek]); 15 times
	NRF24_setRetries(1, 15);

	//PA level from 1 - 3, without PLNA Gain - 0 or with PLNA Gain 1
	NRF24_setPALevel(2, 0);

	//set size of packet
	NRF24_set_PayloadSize(sizeof(Controller));

	//set RF Channel
	NRF24_set_Channel(used_channel);

	//Disable autoacknowledgment
	NRF24_setAutoAck(0x00);

	//Set the adress size for writing pipe
	NRF24_setAdressSize(5);

	//set writing pipe
	NRF24_openWritingPipe(adresses[1], 5);

	//set receiving pipe
	NRF24_openReceivingPipe(1, adresses[0], 5);
	NRF24_startListen();

	while (1)
	{
		//check if we receive something
		if (NRF24_Packet_available())
		{
			//store the received information
			NRF24_read(&master, sizeof(master));

			//check the received information
			if ((master.masteer == 102) && (master.listen == Controller.identify))
			{
				//reading the x axis
				Read_x_axis();

				//reading the y axis
				Read_y_axis();

				//measure sometimes Vcc
				Measure_loop_divided();

				//stop Listen, so that we can write to the channel
				NRF24_stopListen();
				//Send Packet without acknowledgment
				NRF24_send(&Controller, sizeof(Controller), 0);
				//Listen again to channel
				NRF24_startListen();
			}

		}
		_delay_ms(2);
	}
}
