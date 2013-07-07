//============================================================================
// Name        : CC2500.h
// Author      : Pratheesh Prakash, pratheeshprakash@gmail.com
// Reference   : Yasir Ali, ali.yasir0@gmail.com
// Created	   : July 7th 2013
// Description : Library for interfacing CC2500
// Released into the public domain. 
//============================================================================

#ifndef CC2500_h
#define CC2500_h

#include "Arduino.h"

#define pinCS  10
#define pinSCK 13
#define pinMOSI 11
#define pinMISO 12
// command strobe registers,
#define CC2500_CMD_SRES         0x30    // Reset chip
#define CC2500_CMD_SFSTXON      0x31    // Enable and calibrate frequency synthesizer
#define CC2500_CMD_SXOFF        0x32    // Turn off crystal oscillator
#define CC2500_CMD_SCAL         0x33    // Calibrate frequency synthesizer and turn it off
#define CC2500_CMD_SRX          0x34    // Enable RX. Perform calibration if enabled
#define CC2500_CMD_STX          0x35    // Enable TX. If in RX state, only enable TX if CCA passes
#define CC2500_CMD_SIDLE        0x36    // Exit RX / TX, turn off frequency synthesizer
#define CC2500_CMD_SRSVD        0x37    // Reserved
#define CC2500_CMD_SWOR         0x38    // Start automatic RX polling sequence (Wake-on-Radio)
#define CC2500_CMD_SPWD         0x39    // Enter power down mode when CSn goes high
#define CC2500_CMD_SFRX         0x3A    // Flush the RX FIFO buffer
#define CC2500_CMD_SFTX         0x3B    // Flush the TX FIFO buffer
#define CC2500_CMD_SWORRST      0x3C    // Reset real time clock
#define CC2500_CMD_SNOP         0x3D    // No operation, returns status byte
// burst write registers
#define CC2500_REG_PATABLE      0x3E    // PA control settings table
#define CC2500_REG_TXFIFO       0x3F    // Transmit FIFO; Single access is +0x00, burst is +0x40
#define CC2500_REG_RXFIFO       0x3F    // Receive FIFO; Single access is +0x80, burst is +0xC0

class CC2500
{
public:
	CC2500();
	unsigned char spiTransfer(unsigned char data)
	void initialise();
	void reset();
	void disp();
	unsigned char sendByte(unsigned char data);
	unsigned char sendCommand(unsigned char command, unsigned char data);
	unsigned char sendStrobeCommand(unsigned char command);
	unsigned char sendBurstCommand(unsigned char command, unsigned char* data,unsigned char length);
	unsigned char recCommand(unsigned char command);
	unsigned char receiveData(unsigned char *data, unsigned char length);
	void sendData(unsigned char *data, unsigned char length);

private:
	unsigned char val[]{
0x06,0x01,0x06,0x0D,
0xD3,0x91,0xFF,0x04,
0x45, 
0x00,						//address
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
};							// register values

};


#endif