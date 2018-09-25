
/*
 * This file is part of the Bus Pirate project
 * (http://code.google.com/p/the-bus-pirate/).
 *
 * Initial written by Chris van Dongen, 2010.
 *
 * To the extent possible under law, the project has
 * waived all copyright and related or neighboring rights to Bus Pirate.
 *
 * For details see: http://creativecommons.org/publicdomain/zero/1.0/.
 *
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 */

#ifndef BP_PIC_H
#define BP_PIC_H

#include "configuration.h"

#ifdef BP_ENABLE_PIC_SUPPORT

#define PICMODE6 6
#define PICMODE4 4

#define PICSPEED 100 // +- 5KHz

#define PICCMD 1
#define PICDATA 0
#define PICMODEMSK 0xFE
#define PICCMDMSK 0x01

void pic_setup_prepare(void);
void pics_setup_execute(void);
void pic_start(void);
void pic_stop(void);
unsigned int pic_read(void);
unsigned int pic_write(unsigned int c);
void pic_cleanup(void);
void binpic(void);
void pic_run_macro(unsigned int macro);
void pic_print_pins_state(void);

#endif /* BP_ENABLE_PIC_SUPPORT */

#endif /* !BP_PIC_H */