/*
 * This file is part of the Bus Pirate project
 * (http://code.google.com/p/the-bus-pirate/).
 *
 * Written and maintained by the Bus Pirate project.
 *
 * To the extent possible under law, the project has
 * waived all copyright and related or neighboring rights to Bus Pirate. This
 * work is published from United States.
 *
 * For details see: http://creativecommons.org/publicdomain/zero/1.0/.
 *
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 */

#ifndef BP_BASE_IO_H
#define BP_BASE_IO_H

#include <stddef.h>
#include <stdint.h>

// manage user terminal input
unsigned int bpUserNumberPrompt(unsigned int maxBytes, unsigned int maxValue,
                                unsigned int defValue);

// manage user terminal input
unsigned int bpGetUserInput(unsigned int *currentByte, unsigned int maxBytes,
                            unsigned char *terminalInput);

// reads forward to grab ASCII number string, returns byte value
unsigned int bpGetASCIInumber(unsigned int totalBytes,
                              unsigned int *currentByte,
                              unsigned char *commandArr);

// convert ASCII number string (HEX, BIN, DEC) to INT value
unsigned int ASCII2INT(unsigned char totalDigits, unsigned char *numberArr);

/**
 * Writes the given buffer to the serial port.
 *
 * @param[in] buffer the data to write.
 * @param[in] length how many bytes to write.
 */
void bp_write_buffer(const uint8_t *buffer, size_t length);

/**
 * Writes the given NULL-terminated string to the serial port.
 *
 * @param[in] string the string to write.
 */
void bp_write_string(const char *string);

/**
 * Writes the given NULL-terminated string to the serial port, followed by a
 * line break.
 *
 * @param[in] string the string to write.
 */
void bp_write_line(const char *string);

// output an 8bit/byte hex value to the user terminal
void bp_write_hex_byte(uint8_t value);

// output an 16bit/2byte hex value to the user terminal
void bp_write_hex_word(uint16_t value);

// output an 8bit/byte binary value to the user terminal
void bp_write_bin_byte(unsigned char c);

// output an 8bit/byte decimal value to the user terminal
void bp_write_dec_byte(unsigned char c);

// output an 16bit/integer decimal value to the user terminal
void bp_write_dec_word(unsigned int i);

// output an 32bit/long decimal value to the user terminal
void bp_write_dec_dword(unsigned long l);

// friendly version
void bp_write_dec_dword_friendly(unsigned long l);

// print an ADC measurement in decimal form
void bp_write_voltage(unsigned int adc);

// pseudofuncion break sequences
#define bpBR bp_write_line("")

#define bpSP UART1TX(' ') // macro for space

//
//
// Base user terminal UART functions
//
//
#define UART_NORMAL_SPEED 34
#define UART_FAST_SPEED 3

/* interrupt transfer related stuff */
extern uint8_t *UART1RXBuf;
extern unsigned int UART1RXToRecv;
extern unsigned int UART1RXRecvd;
extern uint8_t *UART1TXBuf;
extern unsigned int UART1TXSent;
extern unsigned int UART1TXAvailable;

// starts interrupt TX
void UART1TXInt(void);

// is byte available in RX buffer?
unsigned char UART1RXRdy(void);
unsigned char UART1TXEmpty(void);
// get a byte from UART
unsigned char UART1RX(void);
void WAITTXEmpty(void);

// add byte to buffer, pause if full
// uses PIC 4 byte UART FIFO buffer
void UART1TX(char c);

void ClearCommsError(void);
unsigned char CheckCommsError(void);
// sets the uart baudrate generator
void UART1Speed(unsigned char brg);
// Initialize the terminal UART for the speed currently set in
// bpConfig.termSpeed
void InitializeUART1(void);
//
//
// Ring buffer for UART
//
//
void UARTbufService(void);
void UARTbufFlush(void);
void UARTbufSetup(void);
void UARTbuf(char c);

void bp_write_hex_byte_to_ringbuffer(uint8_t value);

unsigned char USBUSARTIsTxTrfReady(void);
// void putUSBUSART(char *data, unsigned char length);

//
// Hardware functions and definitions
//
// Read the lower 16 bits from programming flash memory
unsigned int bpReadFlash(unsigned int page, unsigned int addr);

#endif /* !BP_BASE_IO_H */
