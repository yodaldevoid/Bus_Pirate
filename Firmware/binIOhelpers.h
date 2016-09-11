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

#ifndef BP_BINIOHELPERS_H
#define BP_BINIOHELPERS_H

#include <stdbool.h>
#include <stdint.h>

#include "configuration.h"

void bp_binary_io_peripherals_set(uint8_t input_byte);

#ifdef BUSPIRATEV4
bool bp_binary_io_pullup_control(uint8_t control_byte);
#endif /* BUSPIRATEV4 */

#endif /* !BP_BINIOHELPERS_H */
