/*
 * This file is part of the Bus Pirate project (http://code.google.com/p/the-bus-pirate/).
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

#include "configuration.h"

#define TERMINAL_BUFFER_SIZE 4096

typedef enum {
	BP_HIZ = 0,
#ifdef BP_ENABLE_1WIRE_SUPPORT
	BP_1WIRE,
#endif /* BP_ENABLE_1WIRE_SUPPORT */
#ifdef BP_ENABLE_UART_SUPPORT
	BP_UART,
#endif /* BP_ENABLE_UART_SUPPORT */
#ifdef BP_ENABLE_I2C_SUPPORT
	BP_I2C,
#endif /* BP_ENABLE_I2C_SUPPORT */
#ifdef BP_ENABLE_SPI_SUPPORT
	BP_SPI,
#endif /* BP_ENABLE_SPI_SUPPORT */
#ifdef BP_ENABLE_RAW_2WIRE_SUPPORT
	BP_RAW2WIRE,
#endif /* BP_ENABLE_RAW_2WIRE_SUPPORT */
#ifdef BP_ENABLE_RAW_3WIRE_SUPPORT
	BP_RAW3WIRE,
#endif /* BP_ENABLE_RAW_3WIRE_SUPPORT */
#ifdef BP_ENABLE_PC_AT_KEYBOARD_SUPPORT
	BP_PCATKBD,
#endif /* BP_ENABLE_PC_AT_KEYBOARD_SUPPORT */
#ifdef BP_ENABLE_HD44780_SUPPORT
	BP_HD44780,
#endif /* BP_ENABLE_HD44780_SUPPORT */
#ifdef BP_ENABLE_PIC_SUPPORT
	BP_PICPROG,
#endif /* BP_ENABLE_PIC_SUPPORT */
#ifdef BP_ENABLE_DIO_SUPPORT
	BP_DIO,
#endif /* BP_ENABLE_DIO_SUPPORT */
	MAXPROTO
} protocol_t;

//these settings persist between modes
// such as terminal side baud rate, display mode, bus mode
typedef struct {
	unsigned char *terminalInput; //hold user terminal input
	unsigned char termSpeed;
	enum {
	 	HEX=0,
		DEC,
		BIN,
		RAW,
	}displayMode;
	protocol_t busMode;
	// Device IDs from the chip
	unsigned int dev_type;
	unsigned int dev_rev;
	unsigned char HWversion;				//holds hardware revision for v3a/v3b
	unsigned char quiet:1;					// no output 
#ifdef BP_ENABLE_BASIC_SUPPORT
	unsigned char basic:1;					// basic commandline
#endif /* BP_ENABLE_BASIC_SUPPORT */
	unsigned char overflow:1; 			//overflow error flag
} bus_pirate_configuration_t;

typedef struct {
	void (*protocol_start)(void);
	void (*protocol_startR)(void);
	void (*protocol_stop)(void);
	void (*protocol_stopR)(void);
	unsigned int (*protocol_send)(unsigned int);
	unsigned int (*protocol_read)(void);
	void (*protocol_clkh)(void);
	void (*protocol_clkl)(void);
	void (*protocol_dath)(void);
	void (*protocol_datl)(void);
	unsigned int (*protocol_dats)(void);
	void (*protocol_clk)(void);
	unsigned int (*protocol_bitr)(void);
	unsigned int (*protocol_periodic)(void);
	void (*protocol_macro)(unsigned int);
	void (*protocol_setup)(void);
	void (*protocol_setup_exc)(void);
	void (*protocol_cleanup)(void);
	void (*protocol_pins)(void);
	void (*protocol_settings)(void);
	char protocol_name[8];
} proto;

//bridge UART input in UART mode
void busPirateAsyncUARTService(void);
