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

#ifndef BP_1WIRE_H
#define BP_1WIRE_H

#include "configuration.h"

#ifdef BP_ENABLE_1WIRE_SUPPORT

#include <stdbool.h>
#include <stdint.h>

/**
 * Enters binary I/O mode for sending 1-Wire commands directly.
 */
void binary_io_enter_1wire_mode(void);

/**
 * Reads some data from the bus.
 *
 * @return the data being read from the bus.
 */
uint16_t onewire_read(void);

/**
 * Sends the given data to the bus.
 *
 * @param[in] data the data to send to the bus.
 *
 * @return the data being read from the bus if any has to be read, or
 *         0 otherwise.
 */
uint16_t onewire_write(uint16_t data);

/**
 * Reads one bit from the data bus.
 *
 * @return the bit read from the bus.
 *
 * @see bus_pirate_protocol_t.protocol_read_bit
 */
bool onewire_read_bit(void);

/**
 * Pulses the clock line, if one is present.
 *
 * @see bus_pirate_protocol_t.protocol_clock_pulse
 */
void onewire_clock_pulse(void);

/**
 * Pulls the bus data line LOW.
 *
 * @see bus_pirate_protocol_t.protocol_data_low
 */
void onewire_data_low(void);

/**
 * Pulls the bus data line HIGH.
 *
 * @see bus_pirate_protocol_t.protocol_data_high
 */
void onewire_data_high(void);

/**
 * Prepares the protocol for operating on the bus.
 *
 * @see bus_pirate_protocol_t.protocol_setup
 * @see bus_pirate_protocol_t.protocol_get_ready
 */
void onewire_setup(void);

/**
 * Runs the macro identified by the given marker.
 *
 * @param[in] macro the macro identifier.
 *
 * @see bus_pirate_protocol_t.protocol_run_macro
 */
void onewire_run_macro(uint16_t macro);

/**
 * Prints the current state of hardware pins when in the current protocol mode.
 *
 * @see bus_pirate_protocol_t.protocol_print_pins_state
 */
void onewire_pins_state(void);

/**
 * Reads the state of the data line, if one is present.
 *
 * @return the state on the data line.
 *
 * @see bus_pirate_protocol_t.protocol_data_state
 */
uint16_t onewire_data_state(void);

/**
 * Resets the bus.
 */
void onewire_reset(void);

#endif /* BP_ENABLE_1WIRE_SUPPORT */

#endif /* !BP_1WIRE_H */