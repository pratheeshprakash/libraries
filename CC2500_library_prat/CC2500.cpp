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
	0x06,	//IOCFG2 
	0x01,	//IOCFG1 
	0x06,	//IOCFG0 
	0x0D,	//FIFOTHR
	0xD3,	//SYNC1  
	0x91,	//SYNC0  
	0xFF,	//PKTLEN 
	0x04,	//PKTCTRL
	0x45,	//PKTCTRL 
	0x00,	//ADDR   					 	
	0x00,	//CHANNR 
	0x09,	//FSCTRL1
	0x00,	//FSCTRL0
	0x5D,	//FREQ2  
	0x93,	//FREQ1  
	0xB1,	//FREQ0  
	0x2D,	//MDMCFG4
	0x3B,	//MDMCFG3
	0x73,	//MDMCFG2
	0x22,	//MDMCFG1
	0xF8,	//MDMCFG0
	0x00,	//DEVIATN
	0x07,	//MCSM2  
	0x30,	//MCSM1  
	0x18,	//MCSM0  
	0x1D,	//FOCCFG 
	0x1C,	//BSCFG  
	0xC7,	//AGCCTRL
	0x00,	//AGCCTRL
	0xB2,	//AGCCTRL
	0x87,	//WOREVT1
	0x6B,	//WOREVT0
	0xF8,	//WORCTRL
	0xB6,	//FREND1 
	0x10,	//FREND0 
	0xEA,	//FSCAL3 
	0x0A,	//FSCAL2 
	0x00,	//FSCAL1 
	0x11,	//FSCAL0 
	0x41,	//RCCTRL1
	0x00,	//RCCTRL0
	0x59,	//FSTEST 
	0x7F,	//PTEST  
	0x3F,	//AGCTEST
	0x88,	//TEST2  
	0x31,	//TEST1  
	0x0B 	//TEST0  
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
    for (int i=0; i<=(length+1); i++) 
	{
		result = spiTransfer(data[i]);
        if(i<length)data[i] = result;  //write only till 1 less than length
    }

    // disable device
    digitalWrite(pinCS, HIGH);

    // return result
    return result;
}

unsigned char CC2500::recCommand(unsigned char command) //command for getting register values
{
    command = command + 0x80;
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
    while (sendByte(0xF5) & 0x1F > 1) { };

	// prepare burst Packet
	unsigned char dp[length+1];
	// First Byte = Length Of Packet = 60 = 0x3C
	dp[0] = length;

	// SIDLE: exit RX/TX
    sendStrobeCommand(CC2500_CMD_SIDLE);
    
    // fill TX FIFO
    for(int i=1;i<=length;i++)
    {
      dp[i]=data[i-1];
	  
    }
	
	//send data in burst
	sendBurstCommand(0x7F, dp, length);
    
    // STX: enable TX
    sendStrobeCommand(CC2500_CMD_STX);
}