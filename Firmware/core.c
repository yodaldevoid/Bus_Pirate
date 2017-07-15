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

#include "configuration.h"

#include "base.h"
#include "core.h"

#ifdef BP_ENABLE_1WIRE_SUPPORT
#include "1wire.h"
#endif /* BP_ENABLE_1WIRE_SUPPORT */

#ifdef BP_ENABLE_UART_SUPPORT
#include "uart.h"
#endif /* BP_ENABLE_UART_SUPPORT */

#ifdef BP_ENABLE_I2C_SUPPORT
#include "i2c.h"
#endif /* BP_ENABLE_I2C_SUPPORT */

#ifdef BP_ENABLE_SPI_SUPPORT
#include "spi.h"
#endif /* BP_ENABLE_SPI_SUPPORT */

#ifdef BP_ENABLE_JTAG_SUPPORT
#include "jtag.h"
#endif /* BP_ENABLE_JTAG_SUPPORT */

#ifdef BP_ENABLE_RAW_2WIRE_SUPPORT
#include "raw2wire.h"
#endif /* BP_ENABLE_RAW_2WIRE_SUPPORT */

#ifdef BP_ENABLE_RAW_3WIRE_SUPPORT
#include "raw3wire.h"
#endif /* BP_ENABLE_RAW_3WIRE_SUPPORT */

#ifdef BP_ENABLE_PC_AT_KEYBOARD_SUPPORT
#include "pc_at_keyboard.h"
#endif /* BP_ENABLE_PC_AT_KEYBOARD_SUPPORT */

#ifdef BP_ENABLE_HD44780_SUPPORT
#include "hd44780.h"
#endif /* BP_ENABLE_HD44780_SUPPORT */

#ifdef BP_ENABLE_PIC_SUPPORT
#include "pic.h"
#endif /* BP_ENABLE_PIC_SUPPORT */

#ifdef BP_ENABLE_DIO_SUPPORT
#include "dio.h"
#endif /* BP_ENABLE_DIO_SUPPORT */

#if defined(BP_ENABLE_RAW_2WIRE_SUPPORT) || defined(BP_ENABLE_RAW_3WIRE_SUPPORT)
#include "raw_common.h"
#endif /* BP_ENABLE_RAW_2WIRE_SUPPORT || BP_ENABLE_RAW_3WIRE_SUPPORT */

/**
 * Predefined "print settings" callback for protocols that do not have any
 * settings to begin with.
 *
 * Given the circumstances, all this function does is to print a CR+LF character
 * pair to the serial port.
 */
static void empty_print_settings_implementation(void);

/**
 * Prints the state of the pins for the high impedance mode.
 */
static void hiz_print_pins_state(void);

/**
 * Resets the current mode configuration to 8-bits data.
 */
static void reset_mode_to_8_bits(void);

/**
 * Predefined callback for running a protocol macro when there are none.
 *
 * All it does is print an appropriate error message to the serial port, and
 * flag an error condition.
 *
 * @param[in] value the macro to run, currently unused.
 */
static void null_macro_callback(uint16_t value);

/**
 * Predefined callback for running an operation without any expected I/O.
 *
 * All it does is print an appropriate error message to the serial port, and
 * flag an error condition.
 */
static void null_operation_callback(void);

/**
 * Predefined callback for sending data when no I/O is available.
 *
 * All it does is print an appropriate error message to the serial port, and
 * flag an error condition.
 *
 * @param[in] data the data to be sent, currently ignored.
 *
 * @return 0x0100, indicating an error condition?
 */
static uint16_t null_send_callback(uint16_t data);

/**
 * Predefined callback for reading data when no I/O is available.
 *
 * All it does is print an appropriate error message to the serial port, and
 * flag an error condition.
 *
 * @return dummy data, 0 in this case.
 */
static uint16_t null_data_read_callback(void);

/**
 * Predefined callback for reading a bit when no I/O is available.
 *
 * All it does is print an appropriate error message to the serial port, and
 * flag an error condition.
 *
 * @return dummy bit, OFF in this case.
 */
static bool null_bit_read_callback(void);

#ifdef BP_ENABLE_DIO_SUPPORT

/**
 * Predefined callback for running an operation without any expected I/O.
 */
static void silent_null_operation_callback(void);

#endif /* BP_ENABLE_DIO_SUPPORT */

extern bus_pirate_configuration_t bus_pirate_configuration;
extern mode_configuration_t mode_configuration;
extern bool command_error;

bus_pirate_protocol_t enabled_protocols[ENABLED_PROTOCOLS_COUNT] = {
    {.start = null_operation_callback,
     .start_with_read = null_operation_callback,
     .stop = null_operation_callback,
     .stop_from_read = null_operation_callback,
     .send = null_send_callback,
     .read = null_data_read_callback,
     .clock_high = null_operation_callback,
     .clock_low = null_operation_callback,
     .data_high = null_operation_callback,
     .data_low = null_operation_callback,
     .data_state = null_data_read_callback,
     .clock_pulse = null_operation_callback,
     .read_bit = null_bit_read_callback,
     .periodic_update = null_bit_read_callback,
     .run_macro = null_macro_callback,
     .setup = reset_mode_to_8_bits,
     .get_ready = reset_mode_to_8_bits,
     .cleanup = reset_mode_to_8_bits,
     .print_pins_state = hiz_print_pins_state,
     .print_settings = empty_print_settings_implementation,
     .name = "HiZ"}

#ifdef BP_ENABLE_1WIRE_SUPPORT
    ,
    {.start = onewire_reset,
     .start_with_read = onewire_reset,
     .stop = null_operation_callback,
     .stop_from_read = null_operation_callback,
     .send = onewire_write,
     .read = onewire_read,
     .clock_high = null_operation_callback,
     .clock_low = null_operation_callback,
     .data_high = onewire_data_high,
     .data_low = onewire_data_low,
     .data_state = onewire_data_state,
     .clock_pulse = onewire_clock_pulse,
     .read_bit = onewire_read_bit,
     .periodic_update = null_bit_read_callback,
     .run_macro = onewire_run_macro,
     .setup = onewire_setup,
     .get_ready = onewire_setup_exc,
     .cleanup = reset_mode_to_8_bits,
     .print_pins_state = onewire_pins_state,
     .print_settings = empty_print_settings_implementation,
     .name = "1-WIRE"}
#endif /* BP_ENABLE_1WIRE_SUPPORT */

#ifdef BP_ENABLE_UART_SUPPORT
    ,
    {.start = uart_start,
     .start_with_read = uart_start,
     .stop = uart_stop,
     .stop_from_read = uart_stop,
     .send = uart_write,
     .read = uart_read,
     .clock_high = null_operation_callback,
     .clock_low = null_operation_callback,
     .data_high = null_operation_callback,
     .data_low = null_operation_callback,
     .data_state = null_data_read_callback,
     .clock_pulse = null_operation_callback,
     .read_bit = null_bit_read_callback,
     .periodic_update = uart_periodic_callback,
     .run_macro = uart_run_macro,
     .setup = uart_setup,
     .get_ready = UARTsetup_exc,
     .cleanup = uart_cleanup,
     .print_pins_state = uart_pins_state,
     .print_settings = UARTsettings,
     .name = "UART"}
#endif /* BP_ENABLE_UART_SUPPORT */

#ifdef BP_ENABLE_I2C_SUPPORT
    ,
    {.start = i2c_start,
     .start_with_read = i2c_start,
     .stop = i2c_stop,
     .stop_from_read = i2c_stop,
     .send = i2c_write,
     .read = i2c_read,
     .clock_high = null_operation_callback,
     .clock_low = null_operation_callback,
     .data_high = null_operation_callback,
     .data_low = null_operation_callback,
     .data_state = null_data_read_callback,
     .clock_pulse = null_operation_callback,
     .read_bit = null_bit_read_callback,
     .periodic_update = null_bit_read_callback,
     .run_macro = i2c_macro,
     .setup = i2c_setup,
     .get_ready = i2c_setup_exc,
     .cleanup = i2c_cleanup,
     .print_pins_state = i2c_pins_state,
     .print_settings = i2c_print_settings,
     .name = "I2C"}
#endif /* BP_ENABLE_I2C_SUPPORT */

#ifdef BP_ENABLE_SPI_SUPPORT
    ,
    {.start = SPIstart,
     .start_with_read = SPIstartr,
     .stop = SPIstop,
     .stop_from_read = SPIstop,
     .send = SPIwrite,
     .read = SPIread,
     .clock_high = null_operation_callback,
     .clock_low = null_operation_callback,
     .data_high = null_operation_callback,
     .data_low = null_operation_callback,
     .data_state = null_data_read_callback,
     .clock_pulse = null_operation_callback,
     .read_bit = null_bit_read_callback,
     .periodic_update = null_bit_read_callback,
     .run_macro = SPImacro,
     .setup = SPIsetup,
     .get_ready = SPIsetup_exc,
     .cleanup = SPIcleanup,
     .print_pins_state = SPIpins,
     .print_settings = SPIsettings,
     .name = "SPI"}
#endif /* BP_ENABLE_SPI_SUPPORT */

#ifdef BP_ENABLE_RAW_2WIRE_SUPPORT
    ,
    {.start = R2Wstart,
     .start_with_read = R2Wstart,
     .stop = R2Wstop,
     .stop_from_read = R2Wstop,
     .send = R2Wwrite,
     .read = R2Wread,
     .clock_high = raw_set_clock_high,
     .clock_low = raw_set_clock_low,
     .data_high = raw_set_data_high,
     .data_low = raw_set_data_low,
     .data_state = raw_read_pin,
     .clock_pulse = raw_advance_clock,
     .read_bit = raw_read_bit,
     .periodic_update = null_bit_read_callback,
     .run_macro = R2Wmacro,
     .setup = R2Wsetup,
     .get_ready = R2Wsetup_exc,
     .cleanup = reset_mode_to_8_bits,
     .print_pins_state = R2Wpins,
     .print_settings = R2Wsettings,
     .name = "2WIRE"}
#endif /* BP_ENABLE_RAW_2WIRE_SUPPORT */

#ifdef BP_ENABLE_RAW_3WIRE_SUPPORT
    ,
    {.start = R3Wstart,
     .start_with_read = R3Wstartr,
     .stop = R3Wstop,
     .stop_from_read = R3Wstop,
     .send = R3Wwrite,
     .read = R3Wread,
     .clock_high = raw_set_clock_high,
     .clock_low = raw_set_clock_low,
     .data_high = raw_set_data_high,
     .data_low = raw_set_data_low,
     .data_state = raw_read_pin,
     .clock_pulse = raw_advance_clock,
     .read_bit = raw_read_bit,
     .periodic_update = null_bit_read_callback,
     .run_macro = null_macro_callback,
     .setup = R3Wsetup,
     .get_ready = R3Wsetup_exc,
     .cleanup = reset_mode_to_8_bits,
     .print_pins_state = R3Wpins,
     .print_settings = R3Wsettings,
     .name = "3WIRE"}
#endif /* BP_ENABLE_RAW_3WIRE_SUPPORT */

#ifdef BP_ENABLE_PC_AT_KEYBOARD_SUPPORT
    ,
    {.start = null_operation_callback,
     .start_with_read = null_operation_callback,
     .stop = null_operation_callback,
     .stop_from_read = null_operation_callback,
     .send = KEYBwrite,
     .read = KEYBread,
     .clock_high = null_operation_callback,
     .clock_low = null_operation_callback,
     .data_high = null_operation_callback,
     .data_low = null_operation_callback,
     .data_state = null_data_read_callback,
     .clock_pulse = null_operation_callback,
     .read_bit = null_bit_read_callback,
     .periodic_update = null_bit_read_callback,
     .run_macro = KEYBmacro,
     .setup = KEYBsetup,
     .get_ready = KEYBsetup_exc,
     .cleanup = reset_mode_to_8_bits,
     .print_pins_state = hiz_print_pins_state,
     .print_settings = empty_print_settings_implementation,
     .name = "KEYB"}
#endif /* BP_ENABLE_PC_AT_KEYBOARD_SUPPORT */

#ifdef BP_ENABLE_HD44780_SUPPORT
    ,
    {.start = LCDstart,
     .start_with_read = LCDstart,
     .stop = LCDstop,
     .stop_from_read = LCDstop,
     .send = LCDwrite,
     .read = null_data_read_callback,
     .clock_high = null_operation_callback,
     .clock_low = null_operation_callback,
     .data_high = null_operation_callback,
     .data_low = null_operation_callback,
     .data_state = null_data_read_callback,
     .clock_pulse = null_operation_callback,
     .read_bit = null_bit_read_callback,
     .periodic_update = null_bit_read_callback,
     .run_macro = LCDmacro,
     .setup = LCDsetup,
     .get_ready = LCDsetup_exc,
     .cleanup = spiDisable,
     .print_pins_state = LCDpins,
     .print_settings = empty_print_settings_implementation,
     .name = "LCD"}
#endif /* BP_ENABLE_HD44780_SUPPORT */

#ifdef BP_ENABLE_PIC_SUPPORT
    ,
    {.start = picstart,
     .start_with_read = picstart,
     .stop = picstop,
     .stop_from_read = picstop,
     .send = picwrite,
     .read = picread,
     .clock_high = null_operation_callback,
     .clock_low = null_operation_callback,
     .data_high = null_operation_callback,
     .data_low = null_operation_callback,
     .data_state = null_data_read_callback,
     .clock_pulse = null_operation_callback,
     .read_bit = null_bit_read_callback,
     .periodic_update = null_bit_read_callback,
     .run_macro = picmacro,
     .setup = picinit,
     .get_ready = picinit_exc,
     .cleanup = piccleanup,
     .print_pins_state = picpins,
     .print_settings = empty_print_settings_implementation,
     .name = "PIC"}
#endif /* BP_ENABLE_PIC_SUPPORT */

#ifdef BP_ENABLE_DIO_SUPPORT
    ,
    {.start = null_operation_callback,
     .start_with_read = null_operation_callback,
     .stop = null_operation_callback,
     .stop_from_read = null_operation_callback,
     .send = dio_write,
     .read = dio_read,
     .clock_high = null_operation_callback,
     .clock_low = null_operation_callback,
     .data_high = null_operation_callback,
     .data_low = null_operation_callback,
     .data_state = null_data_read_callback,
     .clock_pulse = null_operation_callback,
     .read_bit = null_bit_read_callback,
     .periodic_update = null_bit_read_callback,
     .run_macro = null_macro_callback,
     .setup = silent_null_operation_callback,
     .get_ready = silent_null_operation_callback,
     .cleanup = reset_mode_to_8_bits,
     .print_pins_state = hiz_print_pins_state,
     .print_settings = empty_print_settings_implementation,
     .name = "DIO"}
#endif /* BP_ENABLE_DIO_SUPPORT */
};

void null_operation_callback(void) {
  BPMSG1059;
  command_error = true;
}

uint16_t null_send_callback(uint16_t data) {
  BPMSG1059;
  command_error = true;
  return 0x100;
}

uint16_t null_data_read_callback(void) {
  BPMSG1059;
  command_error = true;
  return 0;
}

bool null_bit_read_callback(void) {
  BPMSG1059;
  command_error = true;
  return OFF;
}

void null_macro_callback(uint16_t value) {
  BPMSG1059;
  command_error = true;
}

void empty_print_settings_implementation(void) { bpBR; }

#ifdef BP_ENABLE_DIO_SUPPORT

void silent_null_operation_callback(void) { command_error = false; }

#endif /* BP_ENABLE_DIO_SUPPORT */

void hiz_print_pins_state(void) { MSG_SPI_PINS_STATE; }

void reset_mode_to_8_bits(void) {
  /* Sets the mode configuration to 8 bits. */

  mode_configuration.numbits = 8;
  mode_configuration.int16 = 0;
}
