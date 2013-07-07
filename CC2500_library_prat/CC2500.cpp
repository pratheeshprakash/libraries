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

CC2500::CC2500()
{
	pinMode(pinMOSI, OUTPUT);
	pinMode(pinMISO, INPUT);
	pinMode(pinSCK, OUTPUT);
	pinMode(pinCS, OUTPUT);
}

void CC2500::initialise()
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
}

void CC2500::reset()
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
    while (digitalRead(pinMISO) == HIGH) {
    };

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

unsigned char sendBurstCommand(unsigned char command, unsigned char* data,
    unsigned char length)
{
    // enable device
    digitalWrite(pinCS, LOW);

    // wait for device
    while (digitalRead(pinMISO) == HIGH) {
    };

    // send command byte
    spiTransfer(command);

    unsigned char result = 0;

    // send data bytes
    for (int i=1; i<(length+1); i++) {
        result = spiTransfer(0x3D);//send no operation to get the return value
        data[i] = result;
        if(i==(length+1))
        data[i] = '\0';
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
 while (digitalRead(pinMISO) == HIGH) {
    };
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