//============================================================================
// Name        : CC2500.h
// Author      : Pratheesh Prakash, pratheeshprakash@gmail.com
// Reference   : Yasir Ali, ali.yasir0@gmail.com
// Created	   : July 7th 2013
// Description : Library for interfacing CC2500
// Released into the public domain. 
//============================================================================

#include "Arduino.h"
#include "CC2500.h"

/*!
*  Register Values
*/
unsigned char val[48]=
	{
	0x06,0x01,0x06,0x0D,
	0xD3,0x91,0xFF,0x04,
	0x45, 
	0x00,					 	//address
	0x00,0x09,
	0x00,0x5D,0x93,0xB1,
	0x2D,0x3B,0x73,0x22,
	0xF8,0x00,0x07,0x30,
	0x18,0x1D,0x1C,0xC7,
	0x00,0xB2,0x87,0x6B,
	0xF8,0xB6,0x10,0xEA,
	0x0A,0x00,0x11,0x41,
	0x00,0x59,0x7F,0x3F,
	0x88,0x31,0x0B
	};		
					
CC2500::CC2500(void)
{
	pinMode(pinMOSI, OUTPUT);
	pinMode(pinMISO, INPUT);
	pinMode(pinSCK, OUTPUT);
	pinMode(pinCS, OUTPUT);
}

unsigned char CC2500::spiTransfer(unsigned char data)
{
   // start transmission
    SPDR = data;

    // wait for end of transmission
    while (!(SPSR & (1<<SPIF))) {
    };

    // return received byte
    return SPDR;
}

void CC2500::initialise(void)
{
	// disable device
	digitalWrite(pinCS, HIGH);
	// setup SPI control register: SPCR = 01010000
	// interrupt disabled, spi enabled, msb 1st, master, clk low when idle,
	// sample on rising edge of clk, system clock rate fastest
	SPCR = (1<<SPE) | (1<<MSTR);
	
	// clear data registers
	byte clr = SPSR;
	clr = SPDR;
	
	//reset
	reset();
	reset();
	// write registers till number 47
	for(int i=0;i<47;i++)
	{
		sendCommand(i, val[i]);
	}
	// write power setting to PATABLE memory using single access write. See table 31 on page 47
    // of datasheet
	sendCommand(CC2500_REG_PATABLE, 0xA9);

    // SIDLE: exit RX/TX
	sendStrobeCommand(CC2500_CMD_SIDLE);

    // SIDLE: exit RX/TX
    sendStrobeCommand(CC2500_CMD_SIDLE);

    // SPWD: enter power down mode when CSn goes high
	sendStrobeCommand(CC2500_CMD_SPWD);

    // SIDLE: exit RX/TX
    sendStrobeCommand(CC2500_CMD_SIDLE); 
}

void CC2500::reset(void)
{
	// enable device
	digitalWrite(pinCS, LOW);
	delayMicroseconds(1);
	
	// disable device and wait at least 40 microseconds
	digitalWrite(pinCS, HIGH);
	delayMicroseconds(41);
	
	// enable device
	digitalWrite(pinCS, LOW);
	
	// wait for device
	while (digitalRead(pinMISO) == HIGH) {
	};
	
	// send reset command (SRES)
	spiTransfer(0x30);
	
	// disable device
	digitalWrite(pinCS, HIGH);
}

void CC2500::disp(void)
{
	Serial.println("starting");
	for(int i=0;i<47;i++)
	{
		Serial.print(recCommand(i),HEX);Serial.print("  ");
		if(i%11==0 &&i !=0)
		Serial.println();
	}
	Serial.println();
}

unsigned char CC2500::sendByte(unsigned char data)
{
	// enable device
	digitalWrite(pinCS, LOW);
	
	// wait for device
	while (digitalRead(pinMISO) == HIGH) {
	};
	
	// send byte
	unsigned char result = spiTransfer(data);
	
	// disable device
	digitalWrite(pinCS, HIGH);
	
	// return result
	return result;
}

unsigned char CC2500::sendCommand(unsigned char command, unsigned char data)
{
	// enable device
    digitalWrite(pinCS, LOW);

    // wait for device
    while (digitalRead(pinMISO) == HIGH) {};

    // send command byte
    spiTransfer(command);

    // send data byte
    unsigned char result = spiTransfer(data);

    // disable device
    digitalWrite(pinCS, HIGH);

    // return result
    return result;
}

unsigned char CC2500::sendStrobeCommand(unsigned char command)
{
    // send command
    return sendByte(command);
}

unsigned char CC2500::sendBurstCommand(unsigned char command, unsigned char* data, unsigned char length)
{
    // enable device
    digitalWrite(pinCS, LOW);

    // wait for device
    while (digitalRead(pinMISO) == HIGH) {};

    // send command byte
    spiTransfer(command);

    unsigned char result = 0;

    // send data bytes
    for (int i=1; i<(length+1); ++i) 
	{
        result = spiTransfer(data[i]);
        data[i] = result;
        //if(i==(length+1))
        //data[i] = '\0';
    }

    // disable device
    digitalWrite(pinCS, HIGH);

    // return result
    return result;
}

unsigned char CC2500::recCommand(unsigned char command)
{
    command = command + 128;
    digitalWrite(pinCS, LOW);
    while (digitalRead(pinMISO) == HIGH) {};
    spiTransfer(command);
    unsigned char result=spiTransfer(0);
    digitalWrite(pinCS, HIGH);
    return result;
}

unsigned char CC2500::receiveData(unsigned char *data, unsigned char length)
{
	
	unsigned char result;
    unsigned char ret=0;
    
	// SIDLE: exit RX/TX
    sendStrobeCommand(CC2500_CMD_SIDLE);

	// SRX: enable RX
    sendStrobeCommand(CC2500_CMD_SRX);

	// wait a bit
    delay(10);

	// read RXBYTES register, result should be 0x11
    result = sendCommand(0xFB, 0x00);

	if (result >= 0x11) 
	{
		// single access RX FIFO to get number of bytes to read, should be 0x3C
		result = sendCommand(0xBF, 0x00);

		if (result == length) 
			{

            // read RX FIFO
            sendBurstCommand(0xFF, data, result);                	
            ret=1;// return 1 if data received
            // read 2 remaining bytes
            sendCommand(0xBF, 0x00);
            sendCommand(0xBF, 0x00);
			} 
		else 
			{
           	// something is not right, clear RX FIFO
           	result = sendCommand(0xFB, 0x00);
           	for (int i=0; i<result; ++i) 
				{
               	sendCommand(0xBF, 0x00);
                }
            }
	
	}
	// SIDLE: exit RX/TX
	sendStrobeCommand(CC2500_CMD_SIDLE);	
	return ret;
}

void CC2500::sendData(unsigned char *data, unsigned char length)
{
   
    // wait for previous packet to be completely sent
    while ((sendByte(0xF5) & 0x1F) > 1) {};

    // prepare burst Packet
    unsigned char packet[length];

    // First Byte = Length Of Packet = 60 = 0x3C
    packet[0] = length;

	for(int i = 1; i < length; i++)
	{	        	
        packet[i] = data[i];
	}


   	// SIDLE: exit RX/TX
    sendStrobeCommand(CC2500_CMD_SIDLE);

    // fill TX FIFO
    sendBurstCommand(0x7F, packet, length);

    // STX: enable TX
    sendStrobeCommand(CC2500_CMD_STX);
}