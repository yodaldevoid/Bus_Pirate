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

#ifndef BP_BINARY_IO_H
#define BP_BINARY_IO_H

#include <stdint.h>

/**
 * Result code indicating a successful binary I/O operation.
 */
#define BP_BINARY_IO_RESULT_SUCCESS 0x01

/**
 * Result code indicating a failed binary I/O operation.
 */
#define BP_BINARY_IO_RESULT_FAILURE 0x00

/**
 * @def REPORT_IO_SUCCESS()
 *
 * Outputs a success code to the binary I/O channel.
 */
#define REPORT_IO_SUCCESS()                                                    \
  do {                                                                         \
    user_serial_transmit_character(BP_BINARY_IO_RESULT_SUCCESS);               \
  } while (0)

/**
 * @def REPORT_IO_FAILURE()
 *
 * Outputs a failure code to the binary I/O channel.
 */
#define REPORT_IO_FAILURE()                                                    \
  do {                                                                         \
    user_serial_transmit_character(BP_BINARY_IO_RESULT_FAILURE);               \
  } while (0)

void binBB(void);
uint8_t binBBpindirectionset(uint8_t inByte);
uint8_t binBBpinset(uint8_t inByte);

void bp_binary_io_peripherals_set(uint8_t input_byte);

#ifdef BUSPIRATEV4
bool bp_binary_io_pullup_control(uint8_t control_byte);
#endif /* BUSPIRATEV4 */

void binwire(void);

#endif /* !BP_BINARY_IO_H */