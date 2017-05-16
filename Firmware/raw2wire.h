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

#ifndef BP_RAW2WIRE_H
#define BP_RAW2WIRE_H

#include "configuration.h"

#ifdef BP_ENABLE_RAW_2WIRE_SUPPORT

#include <stdbool.h>

void R2Wstart(void);
void R2Wstop(void);
unsigned int R2Wwrite(unsigned int c);
unsigned int R2Wread(void);
void R2Wmacro(unsigned int c);
void R2Wsetup(void);
void R2Wsetup_exc(void);
void R2Wpins(void);
void R2Wsettings(void);

#endif /* BP_ENABLE_RAW_2WIRE_SUPPORT */

#endif /* !BP_RAW2WIRE_H */
