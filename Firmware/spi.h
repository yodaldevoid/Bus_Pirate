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

#ifndef BP_SPI_H
#define BP_SPI_H

#include "configuration.h"

#ifdef BP_ENABLE_SPI_SUPPORT

#include <stdint.h>

void spiProcess(void);

/**
 * Start accepting binary I/O commands for SPI operations.
 */
void spi_enter_binary_io(void);

/**
 * Writes the given byte on the SPI bus.
 * 
 * @param[in] value the value to write.
 * 
 * @return the byte read from the bus after the data write.
 */
uint8_t spi_write_byte(const uint8_t value);

void SPIstart(void);
void SPIstartr(void);
void SPIstop(void);
unsigned int SPIread(void);
unsigned int SPIwrite(unsigned int c);
void SPIsetup(void);
void SPIsetup_exc(void);
void SPIcleanup(void);
void SPImacro(unsigned int macro);
void SPIpins(void);
void spi_setup(unsigned char spiSpeed);
void spiDisable(void);

void SPIsettings(void);

#endif /* BP_ENABLE_SPI_SUPPORT */

#endif /* !BP_SPI_H */