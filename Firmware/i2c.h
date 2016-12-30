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

#ifndef BP_I2C_H
#define BP_I2C_H

#include "configuration.h"

#ifdef BP_ENABLE_I2C_SUPPORT

/**
 * Start I2C operations.
 */
void i2c_start(void);

void i2cProcess(void);
void binI2C(void);

unsigned int I2Cread(void);
unsigned int I2Cwrite(unsigned int c);

void I2Cstop(void);
void I2Csetup(void);
void I2Csetup_exc(void);
void I2Ccleanup(void);
void I2Cmacro(unsigned int c);
void I2Cpins(void);
void I2Csettings(void);

#endif /* BP_ENABLE_I2C_SUPPORT */

#endif /* !BP_I2C_H */