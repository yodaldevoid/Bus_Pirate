/*
 * This file is part of the Bus Pirate project
 * (http://code.google.com/p/the-bus-pirate/).
 *
 * Written and maintained by the Bus Pirate project.
 *
 * To the extent possible under law, the project has waived all copyright and
 * related or neighboring rights to Bus Pirate.  This work is published from
 * United States.
 *
 * For details see: http://creativecommons.org/publicdomain/zero/1.0/
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.
 */

#ifndef BP_AUX_PIN_H
#define BP_AUX_PIN_H

#include <stdbool.h>

void bpFreq(void);
unsigned long bpBinFreq(void);
void bpPWM(void);

void updatePWM(void);
void bp_aux_pin_set_high_impedance(void);
void bp_aux_pin_set_high(void);
void bp_aux_pin_set_low(void);
bool bp_aux_pin_read(void);

void bpServo(void);

extern int PWMfreq;
extern int PWMduty;

#endif /* !BP_AUX_PIN_H */