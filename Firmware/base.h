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

#ifndef BP_BASE_H
#define BP_BASE_H

/**
 * MCU Operating Frequency, in Hz.
 */
#define FCY 16000000UL

#include <libpic30.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <xc.h>

#include "configuration.h"
#include "messages.h"

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
 * Value indicating a negative statement.
 */
#define NO 0

/**
 * Value indicating a positive statement.
 */
#define YES 1

/**
 * Returns the most significant 16 bits of the given 32-bits integer.
 *
 * @param[in] value a 32-bits integer whose MSW is required.
 */
#define HI16(value) (((uint32_t)(value) >> 16) & 0xFFFF)

/**
 * Returns the least significant 16 bits of the given 32-bits integer.
 *
 * @param[in] value a 32-bits integer whose LSW is required.
 */
#define LO16(value) ((uint32_t)(value)&0xFFFF)

/**
 * Returns the most significant 8 bits of the given 16-bits integer.
 *
 * @param[in] value a 16-bits integer whose MSB is required.
 */
#define HI8(value) (((uint16_t)(value) >> 8) & 0xFF)

/**
 * Returns the least significant 8 bits of the given 16-bits integer.
 *
 * @param[in] value a 16-bits integer whose LSB is required.
 */
#define LO8(value) ((uint16_t)(value)&0xFF)

/**
 * Returns the given value with only the requested bottom bits masked in.
 *
 * @param[in] value a 8-bits integer to mask.
 * @param[in] bits how many bits to leave, starting from LSB.
 */
#define MASKBOTTOM8(value, bits) (((uint8_t)value) & ((1 << (uint8_t)bits) - 1))

/**
 * Firmware version string, used at startup and for the 'i' command.
 */
#define BP_FIRMWARE_STRING "Community Firmware v7.1 - goo.gl/gCzQnW "

#ifdef BUSPIRATEV3
#include "hardwarev3.h"
#elif defined(BUSPIRATEV4)
#include "dp_usb/usb_stack_globals.h"
#include "hardwarev4a.h"
#include "onboard_eeprom.h"
#else
#error "No hardware defined in base.h."
#endif /* BUSPIRATEV3 || BUSPIRATEV4 */

#include "base_io.h"

/**
 * Current mode configuration settings structure.
 *
 * This is used to let protocol implementations to interact with the Bus Pirate
 * event loop.  Every time a mode is changed (a.k.a. a new protocol gets
 * activated) this structure is cleared.
 */
typedef struct {
  unsigned char speed;
  unsigned char numbits;
  unsigned char buf[16];
  uint8_t alternate_aux : 2;
  uint8_t periodicService : 1;
  uint8_t lsbEN : 1;
  uint8_t high_impedance : 1;

  /**
   * Values are 16-bits wide.
   */
  uint8_t int16 : 1;

  /**
   * Each I/O write from the protocol must be followed by a read operation.
   */
  uint8_t write_with_read : 1;

} mode_configuration_t;

typedef struct {
  unsigned char cmd;
  unsigned int num;
  unsigned int repeat;
} command_t;

/**
 * Reverses the bits in the given value and returns it.
 *
 * @param[in] value the value whose bits should be reversed.
 * @param[in] bits width of the value to reverse, in bits.
 *
 * @return the value with reversed bits.
 */
uint16_t bp_reverse_integer(const uint16_t value, const uint8_t bits);

/**
 * Reverses the bits in the given byte and returns it.
 *
 * @param[in] value the byte whose bits should be reversed.
 *
 * @return the value with reversed bits.
 */
inline uint8_t bp_reverse_byte(const uint8_t value);

/**
 * Reverses the bits in the given word and returns it.
 *
 * @param[in] value the word whose bits should be reversed.
 *
 * @return the value with reversed bits.
 */
inline uint16_t bp_reverse_word(const uint16_t value);

/**
 * Bring the board to a clean slate shortly before switching to a new
 * operational mode.
 */
void bp_reset_board_state(void);

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

/**
 * Takes one single ADC measurement and prints it to the serial port.
 */
void bp_adc_probe(void);

/**
 * Takes ADC measurements and prints them to the serial port until a byte is
 * sent to the serial port.
 */
void bp_adc_continuous_probe(void);

/**
 * Prints the given value to the user terminal according to the format settings
 * specified by bus_pirate_configuration_t.display_mode.
 *
 * @param[in] value the value to print to the serial port.
 */
void bp_write_formatted_integer(unsigned int value);

/**
 * Pauses execution for the given amount of milliseconds.
 *
 * @param[in] milliseconds the amount of milliseconds to wait.
 */
#define bp_delay_ms(milliseconds) __delay_ms(milliseconds)

/**
 * Pauses execution for the given amount of microseconds.
 *
 * @param[in] microseconds the amount of microseconds to wait.
 */
#define bp_delay_us(microseconds) __delay_us(microseconds)

#endif /* !BP_BASE_H */
