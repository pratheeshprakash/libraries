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
    /*!
    *  Constructor
    */
	CC2500(void);
	
	/*!
    *  Transfer one byte of data through SPI, returns Status register of CC2500
    */
	unsigned char spiTransfer(unsigned char data);
	
	/*!
    *  Initialise CC2500
    */
	void initialise(void);
	
	/*!
    *  Reset CC2500
    */
	void reset(void);
	
	/*!
    *  Display all registers from 0 - 47 in table format
    */
	void disp(void);
	
	/*!
    *  Send one byte of data to CC2500
    */
	unsigned char sendByte(unsigned char data);
	
	/*!
    *  Send Address and Data to CC2500
    */
	unsigned char sendCommand(unsigned char command, unsigned char data);
	
	/*!
    *  Send only address for Strobe commands
	*  Strobe commands: are preprogrammed commands stored at particular 
	*  register addresses
    */
	unsigned char sendStrobeCommand(unsigned char command);
	
	/*!
    *  Send burst command
	*  Burst commands are used to reduce data size when reading 
	*  consecutive registeres, so address for first one is only transmitted
    */
	unsigned char sendBurstCommand(unsigned char command, unsigned char* data,unsigned char length);
	
	/*!
    *  Read regiter at particular address
    */
	unsigned char recCommand(unsigned char command);
	
	/*!
    *  Receive data on Radio Freq.
    */
	unsigned char receiveData(unsigned char *data, unsigned char length);
	
	/*!
    *  Send data on Radio Freq.
    */
	void sendData(unsigned char *data, unsigned char length);

private:

					

};
		

#endif