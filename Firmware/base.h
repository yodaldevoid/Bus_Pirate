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
#ifndef BP_BASE_H
#define BP_BASE_H

#include <p24Fxxxx.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define FALSE   0
#define TRUE    (!FALSE)

/**
 * Value indicating a bit to be OFF.
 */
#define OFF 0

/**
 * Value indicating a bit to be ON.
 */
#define ON 1

/**
 * Value indicating a pin direction to be OUTPUT.
 */
#define OUTPUT 0

/**
 * Value indicating a pin direction to be INPUT.
 */
#define INPUT 1

/**
 * Value indicating a pin state to be LOW.
 */
#define LOW 0

/**
 * Value indicating a pin state to be HIGH.
 */
#define HIGH 1

/**
 * Returns the most significant 16 bits of the given 32-bits integer.
 * 
 * @param[in] value a 32-bits integer whose MSW is required.
 */
#define HI16(value) (((uint32_t) (value) >> 16) & 0xFFFF)

/**
 * Returns the least significant 16 bits of the given 32-bits integer.
 * 
 * @param[in] value a 32-bits integer whose LSW is required.
 */
#define LO16(value) ((uint32_t) (value) & 0xFFFF)

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////// [ BUS PIRATE SETTINGS ] ////////////////////////////////////////////////////
/////////// NOTE:
///////////     Configuration and Settings for building the Bus Pirate firmware. For more information and support
///////////  On building your own firmware. Please visit the forums on DangerousPrototypes.com. Enjoy the BP :)
///////////
///////////                                                 Bus Pirate - Brought to you by DangerousPrototypes.com

///////////////////////////////////////
// HARDWARE VERSION
#if defined(__PIC24FJ256GB106__)        //v4 chip
#define BUSPIRATEV4
#elif defined(__PIC24FJ64GA002__)       //v3/v2go
#define BUSPIRATEV3
#endif

////////////////////////////////////////
// FIRMWARE VERSION STRING
// Build Firmware Version String
// used in 'i' and startup
#define BP_FIRMWARE_STRING "Community Firmware v7.0 - goo.gl/gCzQnW "

#define LANGUAGE_EN_US

////////////////////////////////////////
// MODE SELECTION
// Default Mode Selection
// BP_MAIN is the default mode setting; varys by hardware version
// BP_ADDONS is for special builds with special modes
// BP_CUSTOM is for selecting your own modes below (find: BP_CUSTOM) and uncomment wanted modes
#define BP_MAIN
//#define BP_ADDONS
//#define BP_CUSTOM

////////////////////////////////////////
// OTHER CONFIGS
// BP_ENABLE_HISTORY = Enable the 'h' command for whatever reason it was disabled this enables it. can be disabled again.
// These settings should be left within the BUSPIRATEV4 define due to lack of space on v3. Unless you disable a mode or two
// for a custom v3 build, only include the following options on a v4 build.
#ifdef BUSPIRATEV4
// Enables the 'h' command to show history
#define BP_ENABLE_HISTORY
#endif /* BUSPIRATEV4 */

///////////////////////
///////////////////////////// [ END OF CONFIGURATION ]//////////////////
////////////////////////////////////////////////////////////
/////////////////////////////////////

#ifdef BUSPIRATEV3
#include "hardwarev3.h"
#elif defined(BUSPIRATEV4)
#include "hardwarev4a.h"
#include "dp_usb/usb_stack_globals.h"
#include "onboard_eeprom.h"
#else
#error "No hardware defined in base.h."
#endif /* BUSPIRATEV3 || BUSPIRATEV4 */

#include "baseIO.h"
#include "baseUI.h"

//Debugging mode for BPv4, comment out for normal compiling
//Adds alternative communicaton to UART1 over AUX1 and AUX2 instead of USB.
//#define BPV4_DEBUG  

#if defined(BP_MAIN)
#define BP_USE_1WIRE
#define BP_USE_HWUART //hardware uart (now also MIDI)
#define BP_USE_I2C
#define BP_USE_HWSPI //hardware spi

#ifdef BUSPIRATEV4
#define BP_USE_BASIC
#define BP_USE_I2C_HW
//#define BP_USE_BASICI2C  // use an i2ceeprom for storing
#define BP_USE_RAW2WIRE
#define BP_USE_RAW3WIRE
#define BP_USE_DIO //binary mode
#define BP_USE_PCATKB
#define BP_USE_PIC
#define BP_USE_JTAG
#define BP_USE_LCD // include HD44780 LCD library       
#define BP_USE_SUMP
#endif /* BUSPIRATEV4 */
        
#elif defined(BP_ADDONS)
// most used protos
#define BP_USE_LCD // include HD44780 LCD library       
#define BP_USE_RAW2WIRE
#define BP_USE_RAW3WIRE
#define BP_USE_PCATKB
#define BP_USE_LCD // include HD44780 LCD library
#define BP_USE_PIC
#define BP_USE_DIO //binary mode
#define BP_USE_SUMP

#elif defined(BP_CUSTOM)

// most used protos
//#define BP_USE_1WIRE
//#define BP_USE_HWUART //hardware uart (now also MIDI)
//#define BP_USE_I2C
//#define BP_USE_I2C_HW
//#define BP_USE_HWSPI //hardware spi
//#define BP_USE_RAW2WIRE
//#define BP_USE_RAW3WIRE
//#define BP_USE_PCATKB
//#define BP_USE_LCD // include HD44780 LCD library
//#define BP_USE_PIC
//#define BP_USE_DIO //binary mode

#else
#error "No Bus Pirate configuration defined."
#endif

// only 1 should be uncommented
//#define BASICTEST
//#define BASICTEST_I2C
//#define BASICTEST_UART
//#define BASICTEST_R3W
//#define BASICTEST_PIC10                       // program blink a led
//#define BASICTEST_PIC10_2                     // read whole pic

//sets the address in the bootloader to jump to on the bootloader command
//must be defined in asm
asm (".equ BLJUMPADDRESS, 0xABF8");

//
//
//
// END CONFIG AREA
//
//
//

// If BUSPIRATEv4 then use the en_US.h otherwise
// use the v3 one. The version3 one does not include
// the modes that the v3 cannot fit. This way we dont
// waste space on strings we wont see.
//
// Also note; the BPV4 project file uses the en_US.s file
// the buspurate v3 uses the other one...
#ifdef BUSPIRATEV4
#ifdef LANGUAGE_EN_US
#include "translations/BPv4_en_US.h"
#else
#error "No language defined in base.h."
#endif /* LANGUAGE_EN_US */
#else
#ifdef LANGUAGE_EN_US
#include "translations/BPv3_en_US.h"
#else
#error "No language defined in base.h."
#endif /* LANGUAGE_EN_US */
#endif /* BUSPIRATEV4 */

//these settings are destroyed between modes.
//this is used to pass stuff from protocol libraries to BP
//for example, allowpullup determines if the P menu is available
//TO DO: add global scratch buffer to setting array for use in 
//	1-Wire enumeration, JTAG input buffer, etc...

typedef struct {
    unsigned char speed;
    unsigned char numbits;
    unsigned char buf[16];
    unsigned char altAUX : 2; // there are 4 AUX on BUSPIRATEV4
    unsigned char periodicService : 1;
    unsigned char lsbEN : 1;
    unsigned char HiZ : 1;
    unsigned char int16 : 1; // 16 bits output?
    unsigned char wwr : 1; // write with read
} mode_configuration_t;

typedef struct {
    unsigned char cmd;
    unsigned int num;
    unsigned int repeat;
} command_t;

unsigned int bpRevByte(unsigned int c);

//reset all peripherals before configuring a new bus protocol
void bpInit(void);

/**
 * Reads a value from the ADC on the given channel.
 * 
 * @warning this function assumes the ADC is already enabled, and will not turn
 *          it on or off.
 * 
 * @param[in] channel the channel to read data from.
 * @return the value read from the channel.
 */
unsigned int bp_read_adc(unsigned int channel);

//takes a measurement from the ADC probe and prints the result to the user terminal
void bpADCprobe(void);
void bpADCCprobe(void);

//print byte c to the user terminal in the format 
//  specified by the bpConfig.displayMode setting
void bpWbyte(unsigned int c);

/**
 * Pauses execution for the given amount of milliseconds.
 *
 * @param[in] milliseconds the amount of milliseconds to wait.
 */
void bp_delay_ms(unsigned int milliseconds);

/**
 * Pauses execution for the given amount of microseconds.
 *
 * @param[in] microseconds the amount of microseconds to wait.
 */
void bp_delay_us(unsigned int microseconds);

#endif /* BP_BASE_H */
