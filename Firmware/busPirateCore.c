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

//These functions abstract the various buses into a generic interface.
//add new modules here and in busPirateCore.h.
//be sure menu entries line up with the _bpConfig.busMode list in busPirateCore.h

#include "configuration.h"

#include "base.h"
#include "busPirateCore.h"

#ifdef BP_ENABLE_1WIRE_SUPPORT
#include "1wire.h"
#endif /* BP_ENABLE_1WIRE_SUPPORT */
#ifdef BP_ENABLE_UART_SUPPORT
#include "uart.h"
#endif
#ifdef BP_ENABLE_I2C_SUPPORT
#include "i2c.h"
#endif /* BP_ENABLE_I2C_SUPPORT */
#ifdef BP_ENABLE_SPI_SUPPORT
#include "spi.h"
#endif /* BP_ENABLE_SPI_SUPPORT */
#ifdef BP_ENABLE_JTAG_SUPPORT
#include "jtag.h"
#endif /* BP_ENABLE_JTAG_SUPPORT */
#ifdef BP_ENABLE_RAW_2WIRE_SUPPORT
#include "raw2wire.h"
#endif /* BP_ENABLE_RAW_2WIRE_SUPPORT */
#ifdef BP_ENABLE_RAW_3WIRE_SUPPORT
#include "raw3wire.h"
#endif /* BP_ENABLE_RAW_3WIRE_SUPPORT */
#ifdef BP_ENABLE_PC_AT_KEYBOARD_SUPPORT
#include "pc_at_keyboard.h"
#endif /* BP_ENABLE_PC_AT_KEYBOARD_SUPPORT */
#ifdef BP_ENABLE_HD44780_SUPPORT
#include "hd44780.h"
#endif /* BP_ENABLE_HD44780_SUPPORT */
#ifdef BP_ENABLE_PIC_SUPPORT
#include "pic.h"
#endif /* BP_ENABLE_PIC_SUPPORT */
#ifdef BP_ENABLE_DIO_SUPPORT
#include "dio.h"
#endif /* BP_ENABLE_DIO_SUPPORT */

extern bus_pirate_configuration_t bpConfig;
extern mode_configuration_t modeConfig;
extern int cmderror;	

void nullfunc1(void)
{	//bpWline("ERROR: command has no effect here");
	BPMSG1059;
	cmderror=1;
}

unsigned int nullfunc2(unsigned int c)
{	//bpWline("ERROR: command has no effect here");
	BPMSG1059;
	cmderror=1;
	return 0x100;
}

unsigned int nullfunc3(void)
{	//bpWline("ERROR: command has no effect here");
	BPMSG1059;
	cmderror=1;
	return 0;
}

void nullfunc4(unsigned int c)
{	//bpWline("ERROR: command has no effect here");
	BPMSG1059;
	cmderror=1;
}

void HiZsetup(void)
{	modeConfig.numbits=8;	// std 8 bits! :P (make this configurable?)
	modeConfig.int16=0;
}

void HiZcleanup(void)
{	modeConfig.numbits=8;	// std 8 bits! :P (make this configurable?)
	modeConfig.int16=0;
}

void HiZpins(void) {
#if defined(BUSPIRATEV4)
    //bpWline("CS\tMISO\tCLK\tMOSI");
    BPMSG1258;
#else
    //bpWline("CLK\tMOSI\tCS\tMISO");
    BPMSG1225;
#endif
}

void HiZsettings(void)
{	bpBR;
}

proto protos[MAXPROTO] = {
{	nullfunc1,				// start
	nullfunc1,				// startR
	nullfunc1,				// stop
	nullfunc1,				// stopR
	nullfunc2,				// send
	nullfunc3,				// read
	nullfunc1,				// clkh
	nullfunc1,				// clkl
	nullfunc1,				// dath
	nullfunc1,				// datl
	nullfunc3,				// dats
	nullfunc1,				// clk
	nullfunc3,				// bitr
	nullfunc3,				// periodic
	nullfunc4,				// macro
	HiZsetup,				// setup
	HiZsetup,               // setup_exc
	HiZcleanup,				// cleanup
	HiZpins,				// pins
	HiZsettings,			// HiZ doesn't have settings
	"HiZ" 					// name
}
#ifdef BP_ENABLE_1WIRE_SUPPORT
,
{	DS1wireReset,			// start
	DS1wireReset,			// startR
	nullfunc1,				// stop
	nullfunc1,				// stopR
	OWwrite,				// send
	OWread,					// read
	nullfunc1,				// clkh
	nullfunc1,				// clkl
	OWdath,					// dath
	OWdatl,					// datl
	OWstate,				// dats
	OWbitclk,				// clk
	OWbitr, 				// bitr
	nullfunc3,				// periodic
	OWmacro,				// macro
	OWsetup,				// setup
	OWsetup,           // setup_exc
	HiZcleanup,				// cleanup
	OWpins,					// pins
	HiZsettings,			// settings
	"1-WIRE" 				// name
}
#endif /* BP_ENABLE_1WIRE_SUPPORT */
#ifdef BP_ENABLE_UART_SUPPORT
,
{	UARTstart,				// start
	UARTstart,				// startR
	UARTstop,				// stop
	UARTstop,				// stopR
	UARTwrite,				// send
	UARTread,				// read
	nullfunc1,				// clkh
	nullfunc1,				// clkl
	nullfunc1,				// dath
	nullfunc1,				// datl
	nullfunc3,				// dats
	nullfunc1,				// clk
	nullfunc3,				// bitr
	UARTperiodic,			// periodic
	UARTmacro,				// macro
	UARTsetup,				// setup
	UARTsetup_exc,          // setup_exc
	UARTcleanup,			// cleanup
	UARTpins,
	UARTsettings,
	"UART" 					// name
}
#endif /* BP_ENABLE_UART_SUPPORT */
#ifdef BP_ENABLE_I2C_SUPPORT
,
{	I2Cstart,				// start
	I2Cstart,				// startR
	I2Cstop,				// stop
	I2Cstop,				// stopR
	I2Cwrite,				// send
	I2Cread,				// read
	nullfunc1,				// clkh
	nullfunc1,				// clkl
	nullfunc1,				// dath
	nullfunc1,				// datl
	nullfunc3,				// dats
	nullfunc1,				// clk
	nullfunc3,				// bitr
	nullfunc3,				// periodic
	I2Cmacro,				// macro
	I2Csetup,				// setup
	I2Csetup_exc,        // setup_exc
	I2Ccleanup,				// cleanup
	I2Cpins,
	I2Csettings,
	"I2C" 					// name
}
#endif /* BP_ENABLE_I2C_SUPPORT */
#ifdef BP_ENABLE_SPI_SUPPORT
,
{	SPIstart,				// start
	SPIstartr,				// startR
	SPIstop,				// stop
	SPIstop,				// stopR
	SPIwrite,				// send
	SPIread,				// read
	nullfunc1,				// clkh
	nullfunc1,				// clkl
	nullfunc1,				// dath
	nullfunc1,				// datl
	nullfunc3,				// dats
	nullfunc1,				// clk
	nullfunc3,				// bitr
	nullfunc3,				// periodic
	SPImacro,				// macro
	SPIsetup,				// setup
	SPIsetup_exc,        // setup_exc
	SPIcleanup,				// cleanup
	SPIpins,
	SPIsettings,
	"SPI" 					// name
}
#endif /* BP_ENABLE_SPI_SUPPORT */
#ifdef BP_ENABLE_RAW_2WIRE_SUPPORT
,
{	R2Wstart,				// start
	R2Wstart,				// startR
	R2Wstop,				// stop
	R2Wstop,				// stopR
	R2Wwrite,				// send
	R2Wread,				// read
	R2Wclkh,				// clkh
	R2Wclkl,				// clkl
	R2Wdath,				// dath
	R2Wdatl,				// datl
	R2Wbitp,				// dats (=bitpeek)
	R2Wclk,					// clk
	R2Wbitr,				// bitr
	nullfunc3,				// periodic
	R2Wmacro,				// macro
	R2Wsetup,				// setup
	R2Wsetup_exc,        // setup_exc
	HiZcleanup,				// cleanup
	R2Wpins,
	R2Wsettings,
	"2WIRE"					// name
}
#endif /* BP_ENABLE_RAW_2WIRE_SUPPORT */
#ifdef BP_ENABLE_RAW_3WIRE_SUPPORT
,
{	R3Wstart,				// start
	R3Wstartr,				// startR
	R3Wstop,				// stop
	R3Wstop,				// stopR
	R3Wwrite,				// send
	R3Wread,				// read
	R3Wclkh,				// clkh
	R3Wclkl,				// clkl
	R3Wdath,				// dath
	R3Wdatl,				// datl
	R3Wbitp,				// dats
	R3Wclk,					// clk
 	R3Wbitr,				// bitr
	nullfunc3,				// periodic
	nullfunc4,				// macro
	R3Wsetup,				// setup
	R3Wsetup_exc,        // setup_exc
	HiZcleanup,				// cleanup
	R3Wpins,
	R3Wsettings,
	"3WIRE" 					// name
}
#endif /* BP_ENABLE_RAW_3WIRE_SUPPORT */
#ifdef BP_ENABLE_PC_AT_KEYBOARD_SUPPORT
,
{	nullfunc1,				// start
	nullfunc1,				// startR
	nullfunc1,				// stop
	nullfunc1,				// stopR
	KEYBwrite,				// send
	KEYBread,				// read
	nullfunc1,				// clkh
	nullfunc1,				// clkl
	nullfunc1,				// dath
	nullfunc1,				// datl
	nullfunc3,				// dats
	nullfunc1,				// clk
	nullfunc3,				// bitr
	nullfunc3,				// periodic
	KEYBmacro,				// macro
	KEYBsetup,				// setup
	KEYBsetup_exc,       // setup_exc
	HiZcleanup,				// cleanup
	HiZpins,
	HiZsettings,
	"KEYB" 					// name
}
#endif /* BP_ENABLE_PC_AT_KEYBOARD_SUPPORT */
#ifdef BP_ENABLE_HD44780_SUPPORT
,
{	LCDstart,				// start
	LCDstart,				// startR
	LCDstop,				// stop
	LCDstop,				// stopR
	LCDwrite,				// send
	nullfunc3,				// read
	nullfunc1,				// clkh
	nullfunc1,				// clkl
	nullfunc1,				// dath
	nullfunc1,				// datl
	nullfunc3,				// dats
	nullfunc1,				// clk
	nullfunc3,				// bitr
	nullfunc3, 				// periodic
	LCDmacro,				// macro
	LCDsetup,				// setup
	LCDsetup_exc,        // setup_exc
	spiDisable,				// cleanup
	LCDpins,
	HiZsettings,
	"LCD" 					// name
}
#endif /* BP_ENABLE_HD44780_SUPPORT */
#ifdef BP_ENABLE_PIC_SUPPORT
,
{	picstart,				// start
	picstart,				// startR
	picstop,				// stop
	picstop,				// stopR
	picwrite,				// send
	picread,				// read
	nullfunc1,				// clkh
	nullfunc1,				// clkl
	nullfunc1,				// dath
	nullfunc1,				// datl
	nullfunc3,				// dats
	nullfunc1,				// clk
	nullfunc3,				// bitr
	nullfunc3, 				// periodic
	picmacro,				// macro
	picinit,				   // setup
	picinit_exc,         // setup_exc
	piccleanup,				// cleanup
	picpins,
	HiZsettings,
	"PIC" 					// name
}
#endif /* BP_ENABLE_PIC_SUPPORT */
#ifdef BP_ENABLE_DIO_SUPPORT
,
{	nullfunc1,				// start
	nullfunc1,				// startR
	nullfunc1,				// stop
	nullfunc1,				// stopR
	dio_write,				// send
	dio_read,				// read
	nullfunc1,				// clkh
	nullfunc1,				// clkl
	nullfunc1,				// dath
	nullfunc1,				// datl
	nullfunc3,				// dats
	nullfunc1,				// clk
	nullfunc3,				// bitr
	nullfunc3,				// periodic
	nullfunc4,				// macro
	dio_setup,				// setup
	dio_setup_execute,        // setup_exc
	HiZcleanup,				// cleanup
	HiZpins,
	HiZsettings,
	"DIO" 					// name
}
#endif /* BP_ENABLE_DIO_SUPPORT */
};