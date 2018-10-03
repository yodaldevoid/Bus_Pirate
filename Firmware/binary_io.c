/*
 * This file is part of the Bus Pirate project
 * (http://code.google.com/p/the-bus-pirate/).
 *
 * Written and maintained by the Bus Pirate project.
 *
 * To the extent possible under law, the project has waived all copyright and
 * related or neighboring rights to Bus Pirate. This work is published from
 * United States.
 *
 * For details see: http://creativecommons.org/publicdomain/zero/1.0/
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.
 */

#include <stdbool.h>

/* Binary access modes for Bus Pirate scripting */

#include "configuration.h"

#include "base.h"
#include "bitbang.h"
#include "selftest.h"

#ifdef BP_ENABLE_SPI_SUPPORT
#include "spi.h"
#endif /* BP_ENABLE_SPI_SUPPORT */
#ifdef BP_ENABLE_I2C_SUPPORT
#include "i2c.h"
#endif /* BP_ENABLE_I2C_SUPPORT */
#ifdef BP_ENABLE_UART_SUPPORT
#include "uart.h"
#endif /* BP_ENABLE_UART_SUPPORT */
#ifdef BP_ENABLE_1WIRE_SUPPORT
#include "1wire.h"
#endif /* BP_ENABLE_1WIRE_SUPPORT */
#ifdef BP_ENABLE_PIC_SUPPORT
#include "pic.h"
#endif /* BP_ENABLE_PIC_SUPPORT */
#ifdef BP_ENABLE_JTAG_SUPPORT
#include "jtag.h"
#ifdef BP_JTAG_OPENOCD_SUPPORT
#include "openocd.h"
#endif /* BP_JTAG_OPENOCD_SUPPORT */
#endif /* BP_ENABLE_JTAG_SUPPORT */
#ifdef BP_ENABLE_SMPS_SUPPORT
#include "smps.h"
#endif /* BP_ENABLE_SMPS_SUPPORT */

#include "aux_pin.h"
#include "binary_io.h"
#include "core.h"

extern mode_configuration_t mode_configuration;
extern bus_pirate_configuration_t bus_pirate_configuration;

/**
 * Sends the Binary I/O mode identifier over to the serial channel.
 *
 * Unlike other instances of mode identifier reporting, this is called several
 * times in the same function and therefore it makes sense to encapsulate it in
 * a single function to save on code size.
 */
static void send_binary_io_mode_identifier(void);

typedef enum {
  IO_COMMAND_GROUP_GENERIC = 0,
  IO_COMMAND_GROUP_BULK_TRANSFER,
  IO_COMMAND_GROUP_BULK_CLOCK_TICKS,
  IO_COMMAND_GROUP_BULK_BITS,
  IO_COMMAND_GROUP_CONFIGURE_PERIPHERALS,
  IO_COMMAND_GROUP_BULK_READ,
  IO_COMMAND_GROUP_SET_SPEED,
  IO_COMMAND_GROUP_CONFIGURATION = 8,
  IO_COMMAND_GROUP_PIC = 10,
  IO_COMMAND_GROUP_SMPS = 15
} io_command_group;

typedef enum {
  IO_COMMAND_EXIT = 0,
  IO_COMMAND_REPORT_ID_STRING,
  IO_COMMAND_SEND_I2C_START_BIT,
  IO_COMMAND_SEND_I2C_STOP_BIT,
  IO_COMMAND_CS_LOW,
  IO_COMMAND_CS_HIGH,
  IO_COMMAND_BITBANG_READ_BYTE,
  IO_COMMAND_BITBANG_READ_BIT,
  IO_COMMAND_PEEK_INPUT_BIT,
  IO_COMMAND_CLOCK_TICK,
  IO_COMMAND_CLOCK_LOW,
  IO_COMMAND_CLOCK_HIGH,
  IO_COMMAND_DATA_LOW,
  IO_COMMAND_DATA_HIGH
} binary_io_command;

static inline bool handle_binary_io_command(const binary_io_command command,
                                            const size_t wires);
static inline void handle_bulk_transfer(const uint8_t command,
                                        const size_t wires);
static inline void handle_bulk_clock_ticks(const uint8_t command);
static inline void handle_bulk_bits(const uint8_t command);

enum {
  PICUNK = 0,
  PIC416,
  PIC424,
  PIC614,
};

static void PIC24NOP(void);
static void PIC614Write(unsigned char cmd, unsigned char datl,
                        unsigned char dath);
static void PIC614Read(unsigned char c);
static void PIC416Read(unsigned char c);
static void PIC416Write(unsigned char cmd, unsigned char datl,
                        unsigned char dath);
static void PIC424Write_internal(unsigned long cmd, unsigned char pn);
static void PIC424Write(unsigned char *cmd, unsigned char pn);
static void PIC424Read(void);

#define R3WMOSI_TRIS BP_MOSI_DIR
#define R3WCLK_TRIS BP_CLK_DIR
#define R3WMISO_TRIS BP_MISO_DIR
#define R3WCS_TRIS BP_CS_DIR

#define R3WMOSI BP_MOSI
#define R3WCLK BP_CLK
#define R3WMISO BP_MISO
#define R3WCS BP_CS

static void binSelfTest(bool jumper_test);
static void binReset(void);

/*
Bitbang is like a player piano or bitmap. The 1 and 0 represent the pins.
So for the four Bus Pirate pins we use the the bits as follows:
COMMAND|POWER|PULLUP|AUX|CS|MISO|CLK|MOSI.

The Bus pirate also responds to each write with a read byte showing the current
state of the pins.

The bits control the state of each of those pins when COMMAND=1.
When COMMAND=0 then up to 127 command codes can be entered on the lower bits.
0x00 resets the Bus Pirate to bitbang mode.

Data:
1xxxxxxx //COMMAND|POWER|PULLUP|AUX|MOSI|CLK|MISO|CS

Commands:
00000000 //Reset to raw BB mode, get raw BB version string
00000001 //enter rawSPI mode
00000010 //enter raw I2C mode
00000011 //enter raw UART mode
00000100 // enter raw 1-wire
00000101 //enter raw wire mode
00000110 // enter openOCD
00000111 // pic programming mode
00001111 //reset, return to user terminal
00010000 //short self test
00010001 //full self test with jumpers
00010010 // setup PWM
00010011 // clear PWM
00010100 // ADC measurement

// Added JM  Only with BP4
00010101 // ADC ....
00010110 // ADC Stop
00011000 // XSVF Player
// End added JM
//
010xxxxx //set input(1)/output(0) pin state (returns pin read)
 */

void send_binary_io_mode_identifier(void) { MSG_BBIO_MODE_IDENTIFIER; }

void binBB(void) {
  static unsigned char inByte;
  unsigned int i;

  bp_enable_mode_led();
  binReset();
  send_binary_io_mode_identifier();

  while (1) {

    inByte = user_serial_read_byte();

    if ((inByte & 0b10000000) == 0) { // if command bit cleared, process command
      if (inByte == 0) {              // reset, send BB version
        send_binary_io_mode_identifier();
      } else if (inByte == 1) { // goto SPI mode
        binReset();
#ifdef BP_ENABLE_SPI_SUPPORT
        spi_enter_binary_io(); // go into rawSPI loop
#endif                         /* BP_ENABLE_SPI_SUPPORT */
        binReset();
        send_binary_io_mode_identifier();
      } else if (inByte == 2) { // goto I2C mode
        binReset();
#ifdef BP_ENABLE_I2C_SUPPORT
        binary_io_enter_i2c_mode();
#endif /* BP_ENABLE_I2C_SUPPORT */
        binReset();
        send_binary_io_mode_identifier();
      } else if (inByte == 3) { // goto UART mode
        binReset();
#ifdef BP_ENABLE_UART_SUPPORT
        binUART();
#endif
        binReset();
        send_binary_io_mode_identifier();
      } else if (inByte == 4) { // goto 1WIRE mode
        binReset();
#ifdef BP_ENABLE_1WIRE_SUPPORT
        binary_io_enter_1wire_mode();
#endif /* BP_ENABLE_1WIRE_SUPPORT */
        binReset();
        send_binary_io_mode_identifier();
      } else if (inByte == 5) { // goto RAW WIRE mode
        binReset();
        binwire();
        binReset();
        send_binary_io_mode_identifier();
      } else if (inByte == 6) { // goto OpenOCD mode
        binReset();
#ifdef BP_JTAG_OPENOCD_SUPPORT
        binOpenOCD();
#endif /* BP_JTAG_OPENOCD_SUPPORT */
        binReset();
        send_binary_io_mode_identifier();
      } else if (inByte == 7) { // goto pic mode
        binReset();
#ifdef BP_ENABLE_PIC_SUPPORT
        binary_io_enter_pic_mode();
#endif /* BP_ENABLE_PIC_SUPPORT */
        binReset();
        send_binary_io_mode_identifier();
      } else if (inByte == 0b1111) { // return to terminal
        user_serial_transmit_character(1);
        BP_LEDMODE = 0;                       // light MODE LED
        user_serial_wait_transmission_done(); // wait untill TX finishes
#ifndef BUSPIRATEV4
        __asm volatile("RESET");
#endif
#ifdef BUSPIRATEV4 // cannot use ASM reset on BPv4
        binReset();
        return;
#endif
        // self test is only for v2go and v3
      } else if (inByte == 0b10000) { // short self test
        binSelfTest(0);
      } else if (inByte == 0b10001) { // full self test with jumpers
        binSelfTest(1);
      } else if (inByte == 0b10010) { // setup PWM

        // cleanup timers from FREQ measure
        T2CON = 0; // 16 bit mode
        T4CON = 0;
        OC5CON = 0; // clear PWM settings

        BP_AUX_RPOUT = OC5_IO; // setup pin

        // get one byte
        i = user_serial_read_byte();
        if (i & 0b10)
          T2CONbits.TCKPS1 = 1; // set prescalers
        if (i & 0b1)
          T2CONbits.TCKPS0 = 1;

        // get two bytes
        i = (user_serial_read_byte() << 8);
        i |= user_serial_read_byte();
        OC5R = i; // Write duty cycle to both registers
        OC5RS = i;
        OC5CON = 0x6; // PWM mode on OC, Fault pin disabled

        // get two bytes
        i = (user_serial_read_byte() << 8);
        i |= user_serial_read_byte();
        PR2 = i; // write period

        T2CONbits.TON = 1; // Start Timer2
        user_serial_transmit_character(1);
      } else if (inByte == 0b10011) { // clear PWM
        T2CON = 0;                    // stop Timer2
        OC5CON = 0;
        BP_AUX_RPOUT = 0; // remove output from AUX pin
        user_serial_transmit_character(1);
        // ADC only for v1, v2, v3
      } else if (inByte == 0b10100) {  // ADC reading (x/1024)*6.6volts
        AD1CON1bits.ADON = 1;          // turn ADC ON
        i = bp_read_adc(BP_ADC_PROBE); // take measurement
        AD1CON1bits.ADON = 0;          // turn ADC OFF
        user_serial_transmit_character((i >> 8)); // send upper 8 bits
        user_serial_transmit_character(i);        // send lower 8 bits
      } else if (inByte == 0b10101) { // ADC reading (x/1024)*6.6volts
        AD1CON1bits.ADON = 1;         // turn ADC ON
        while (1) {
          i = bp_read_adc(BP_ADC_PROBE); // take measurement
          user_serial_wait_transmission_done();
          user_serial_transmit_character((i >> 8)); // send upper 8 bits
          // while(UART1TXRdy==0);
          user_serial_transmit_character(i); // send lower 8 bits

          if (user_serial_ready_to_read() == 1) { // any key pressed, exit
            i = user_serial_read_byte();          // /* JTR usb port; */;
            break;
          }
        }
        AD1CON1bits.ADON = 0;         // turn ADC OFF
      } else if (inByte == 0b10110) { // binary frequency count access
        unsigned long l;
        l = bp_measure_frequency();
        user_serial_transmit_character((l >> (8 * 3)));
        user_serial_transmit_character((l >> (8 * 2)));
        user_serial_transmit_character((l >> (8 * 1)));
        user_serial_transmit_character((l));
//--- Added JM
#ifdef BUSPIRATEV4
      } else if (inByte == 0b11000) { // XSVF Player to program CPLD
        bp_enable_voltage_regulator();
        MSG_XSV1_MODE_IDENTIFIER;
        jtag();
#endif
        //--- End added JM
      } else if ((inByte >> 5) & 0b010) { // set pin direction, return read
        user_serial_transmit_character(bitbang_pin_direction_set(inByte));
      } else { // unknown command, error
        user_serial_transmit_character(0);
      }

    } else { // data for pins
      user_serial_transmit_character(bitbang_pin_state_set(inByte));
    } // if
  }   // while
} // function

void binReset(void) {
  bp_disable_3v3_pullup();
  bitbang_pin_direction_set(0xff); // pins to input on start
  bitbang_pin_state_set(0);        // startup everything off, pins at ground
}

uint8_t bitbang_pin_direction_set(const uint8_t direction_mask) {

  /* Set directions. */

  BP_AUX0_DIR = (direction_mask & 0b00010000) ? INPUT : OUTPUT;
  BP_MOSI_DIR = (direction_mask & 0b00001000) ? INPUT : OUTPUT;
  BP_CLK_DIR = (direction_mask & 0b00000100) ? INPUT : OUTPUT;
  BP_MISO_DIR = (direction_mask & 0b00000010) ? INPUT : OUTPUT;
  BP_CS_DIR = (direction_mask & 0b00000001) ? INPUT : OUTPUT;

  /* Wait a bit. */

  bp_delay_us(5);

  /* Return current state. */

  return ((BP_AUX0 == HIGH) ? 0b00010000 : 0b00000000) |
         ((BP_MOSI == HIGH) ? 0b00001000 : 0b00000000) |
         ((BP_CLK == HIGH) ? 0b00000100 : 0b00000000) |
         ((BP_MISO == HIGH) ? 0b00000010 : 0b00000000) |
         ((BP_CS == HIGH) ? 0b00000001 : 0b00000000);
}

uint8_t bitbang_pin_state_set(const uint8_t state_mask) {

  /* Set state. */

  bp_set_voltage_regulator_state((state_mask & 0b01000000) ? ON : OFF);
  bp_set_pullup_state((state_mask & 0b00100000) ? ON : OFF);

  BP_AUX0 = (state_mask & 0b00010000) ? HIGH : LOW;
  BP_MOSI = (state_mask & 0b00001000) ? HIGH : LOW;
  BP_CLK = (state_mask & 0b00000100) ? HIGH : LOW;
  BP_MISO = (state_mask & 0b00000010) ? HIGH : LOW;
  BP_CS = (state_mask & 0b00000001) ? HIGH : LOW;

  /* Wait a bit. */

  bp_delay_us(5);

  /* Return current state. */

  return ((BP_AUX0 == HIGH) ? 0b00010000 : 0b00000000) |
         ((BP_MOSI == HIGH) ? 0b00001000 : 0b00000000) |
         ((BP_CLK == HIGH) ? 0b00000100 : 0b00000000) |
         ((BP_MISO == HIGH) ? 0b00000010 : 0b00000000) |
         ((BP_CS == HIGH) ? 0b00000001 : 0b00000000);
}

void binSelfTest(bool jumper_test) {
  static volatile unsigned int tick = 0;
  unsigned char errors, inByte;

  errors = perform_selftest(false, jumper_test); // silent self-test
  if (errors)
    BP_LEDMODE = 1;                       // light MODE LED if errors
  user_serial_transmit_character(errors); // reply with number of errors

  while (1) {
    // echo incoming bytes + errors
    // tests FTDI chip, UART, retrieves results of test
    if (user_serial_ready_to_read()) {
      inByte = user_serial_read_byte(); // check input
      if (inByte != 0xff) {
        user_serial_transmit_character(inByte + errors);
      } else {
        user_serial_transmit_character(0x01);
        return; // exit if we get oxff, else send back byte+errors
      }
    }

    if (!errors) {
      if (tick == 0) {
        tick = 0xFFFF;
        BP_LEDMODE ^= 1; // toggle LED
      }
      tick--;
    }
  }
}

void bp_binary_io_peripherals_set(unsigned char inByte) {
  bp_set_voltage_regulator_state((inByte & 0b00001000) == 0b00001000);
  bp_set_pullup_state((inByte & 0b00000100) == 0b00000100);

  BP_AUX0_DIR = OUTPUT;
  BP_AUX0 = (inByte & 0b00000010) ? ON : OFF;

  // CS pin, follows HiZ setting
  if (inByte & 0b1) {
    if (mode_configuration.high_impedance == 1) {
      IODIR |= CS; // CS iput in open drain mode
    } else {
      IOLAT |= CS;    // CS high
      IODIR &= (~CS); // CS output
    }
  } else {
    IOLAT &= (~CS); // BP_CS=0;
    IODIR &= (~CS); // CS output
  }
}

#ifdef BUSPIRATEV4

// checks if voltage is present on VUEXTERN
bool bp_binary_io_pullup_control(uint8_t control_byte) {

  if (mode_configuration.high_impedance == NO) {
    return false;
  }

  /* Disable both pull-ups. */
  bp_disable_3v3_pullup();
  bp_delay_ms(2);

  /* Turn on the ADC. */
  bp_enable_adc();

  /* Is there already an external voltage? */
  bool has_voltage = bp_read_adc(BP_ADC_VPU) > 0x0100;

  /* Turn off the ADC. */
  bp_disable_adc();

  if (has_voltage) {
    return false;
  }

  switch (control_byte) {
  case 0x51:
    /* Turn on the +3.3v pull-up. */
    bp_enable_3v3_pullup();
    break;

  case 0x52:
    /* Turn on the +5v pull-up. */
    bp_enable_5v0_pullup();
    break;

  default:
    /* Turn off both pull-ups. */
    bp_disable_3v3_pullup();
    break;
  }

  return true;
}

#endif /* BUSPIRATEV4 */

/*
 * 00000000 � Enter raw bitbang mode, reset to raw bitbang mode
 * 00000001 � Mode version string (RAW1)
 * 00000010 - I2C style start bit
 * 00000011 - I2C style stop bit
 * 00000100 - CS low (0) (respects hiz setting)
 * 00000101 - CS high (1)
 * 00000110 - Read byte
 * 00000111 - Read bit
 * 00001000 - Peek at input pin
 * 00001001 - Clock tick
 * 00001010 - Clock low
 * 00001011 - Clock high
 * 00001100 - Data low
 * 00001101 - Data high
 *
 * 0001xxxx � Bulk transfer, send 1-16 bytes (0=1byte!)
 *
 * 0010xxxx - Bulk clock ticks, send 1-16 ticks
 * 0011xxxx - Bulk bits, send 1-8 bits of the next byte (0=1bit!)
 * 0100wxyz � Configure peripherals, w=power, x=pullups, y=AUX, z=CS
 * 0101xxxx - Bulk read, read 1-16bytes (0=1byte!)
 * 0110000x � Set speed
 * 1000wxyz � Config, w=output type, x=3wire, y=lsb, z=n/a
 ****************** BPv4 Specific Instructions *********************
 * 11110000 - Return SMPS output voltage
 * 11110001 - Stop SMPS operation
 * 1111xxxx - Start SMPS operation (xxxx and next byte give requested output
 voltage) Lowest possible value is 512 = 0b0010 0000

 */

void binwire(void) {
  static unsigned char i, c, wires, picMode = PIC614;
  static unsigned int cmds, cmdw, cmdr, j;

  mode_configuration.high_impedance = YES;
  mode_configuration.little_endian = NO;
  mode_configuration.speed = 1;
  mode_configuration.numbits = 8;
  // startup in raw2wire mode
  wires = 2;
  // configure for raw3wire mode
  bitbang_setup(2, BITBANG_SPEED_MAXIMUM); // setup the bitbang library, must be
                                           // done before calling bbCS below
  // setup pins (pins are input/low when we start)
  // MOSI output, low
  // clock output, low
  // MISO input
  // CS output, high
  R3WMOSI_TRIS = OUTPUT;
  R3WCLK_TRIS = OUTPUT;
  R3WMISO_TRIS = INPUT;
  bitbang_set_cs(HIGH); // takes care of custom HiZ settings too

  MSG_RAW_MODE_IDENTIFIER;

  bool keep_looping = true;

  while (keep_looping) {
    uint8_t input_byte = user_serial_read_byte();

    switch ((io_command_group)(input_byte >> 4)) {
    case IO_COMMAND_GROUP_GENERIC:
      keep_looping =
          handle_binary_io_command((binary_io_command)input_byte, wires);
      continue;

    case IO_COMMAND_GROUP_BULK_TRANSFER:
      handle_bulk_transfer(input_byte, wires);
      break;

    case IO_COMMAND_GROUP_BULK_CLOCK_TICKS:
      handle_bulk_clock_ticks(input_byte);
      break;

    case IO_COMMAND_GROUP_BULK_BITS:
      handle_bulk_bits(input_byte);
      break;

    case IO_COMMAND_GROUP_PIC:

      switch (input_byte) {
      case 0b10100000:

        picMode = user_serial_read_byte(); // /* JTR usb port; */; //get byte
        user_serial_transmit_character(1); // send 1/OK
        break;
      case 0b10100100: // write
        switch (picMode) {
        case PIC416:

          // get the number of commands that will follow
          cmds =
              user_serial_read_byte(); //  //get byte, reuse rawCommand variable
          cmds = cmds * 3;             // make sure an int
          // get command byte, two data bytes
          for (j = 0; j < cmds; j++) {

            bus_pirate_configuration.terminal_input[j] =
                user_serial_read_byte(); // /* JTR usb port; */;
          }

          for (j = 0; j < cmds; j = j + 3) {
            PIC416Write(bus_pirate_configuration.terminal_input[j],
                        bus_pirate_configuration.terminal_input[j + 1],
                        bus_pirate_configuration.terminal_input[j + 2]);
          }

          user_serial_transmit_character(1); // send 1/OK
          break;
        case PIC424:
          // get the number of commands that will follow
          cmds = user_serial_read_byte(); // /* JTR usb port; */; //get byte,
                                          // reuse rawCommand variable
          cmds = cmds * 4;                // make sure an int
          // get three byte command, 1 byte pre-post NOP
          for (j = 0; j < cmds; j++) {

            bus_pirate_configuration.terminal_input[j] =
                user_serial_read_byte(); // /* JTR usb port; */;
          }

          for (j = 0; j < cmds; j = j + 4) {
            // do any pre instruction NOPs

            // send four bit SIX command (write)
            bitbang_write_bit(0); // send bit
            bitbang_write_bit(0); // send bit
            bitbang_write_bit(0); // send bit
            bitbang_write_bit(0); // send bit

            // send data payload
            bitbang_write_value(
                bus_pirate_configuration.terminal_input[j]); // send byte
            bitbang_write_value(
                bus_pirate_configuration.terminal_input[j + 1]); // send byte
            bitbang_write_value(
                bus_pirate_configuration.terminal_input[j + 2]); // send byte

            // do any post instruction NOPs
            bus_pirate_configuration.terminal_input[j + 3] &= 0x0F;
            for (i = 0; i < bus_pirate_configuration.terminal_input[j + 3];
                 i++) {
              PIC24NOP();
            }
          }
          user_serial_transmit_character(1); // send 1/OK
          break;
        default:
          user_serial_transmit_character(0); // send 1/OK
          break;
        }
        break;
      case 0b10100101: // write x bit command, read x bits and return in 2 bytes
        switch (picMode) {
        case PIC416: {

          // get the number of commands that will follow
          cmds =
              user_serial_read_byte(); // //get byte, reuse rawCommand variable
          // cmds=cmds; //make sure an int
          // get teh command to send on each read....

          uint8_t command = user_serial_read_byte(); // /* JTR usb port; */;

          for (j = 0; j < cmds; j++) {
            // write command
            c = command; // temporary varaible
            for (i = 0; i < 4; i++) {
              if (c & 0b1) {          // send 1
                bitbang_write_bit(1); // send bit
              } else {                // send 0
                bitbang_write_bit(0); // send bit
              }
              c = c >> 1; // pop the LSB off
            }
            bitbang_read_value(); // dummy byte, setup input
            user_serial_transmit_character(bitbang_read_value());
          }
          break;
        }
        case PIC424:
          // get the number of commands that will follow
          cmds =
              user_serial_read_byte(); //  //get byte, reuse rawCommand variable

          for (j = 0; j < cmds; j++) {
            // write command
            PIC424Write_internal(0xBA0B96, 2);
            PIC424Read();

            PIC424Write_internal(0xBADBB6, 2);
            PIC424Write_internal(0xBAD3D6, 2);
            PIC424Read();

            PIC424Write_internal(0xBA0BB6, 2);
            PIC424Read();
          }
          break;
        }
        break;
      case 0b10100111: // x write, y read commands.
        cmdw = user_serial_read_byte();
        cmdr = user_serial_read_byte();

        if (picMode == PIC424) {
          cmds = cmdw * 5 + cmdr;
        } else if (picMode == PIC416) {
          cmds = cmdw * 4 + cmdr * 2;
        } else if (picMode == PIC614) {
          cmds = cmdw * 4 + cmdr * 2;
        } else {
          user_serial_transmit_character(0);
          break;
        }

        for (j = 0; j < cmds; j++) {
          bus_pirate_configuration.terminal_input[j] = user_serial_read_byte();
        }

        if (cmdr != 0)
          user_serial_transmit_character(1); // ACK

        j = 0;
        while (j < cmds) {
          if (bus_pirate_configuration.terminal_input[j] ==
              1) { // write command
            if (picMode == PIC614) {
              PIC614Write(bus_pirate_configuration.terminal_input[j + 1],
                          bus_pirate_configuration.terminal_input[j + 2],
                          bus_pirate_configuration.terminal_input[j + 3]);
              j += 4;
            } else if (picMode == PIC416) {
              PIC416Write(bus_pirate_configuration.terminal_input[j + 1],
                          bus_pirate_configuration.terminal_input[j + 2],
                          bus_pirate_configuration.terminal_input[j + 3]);
              j += 4;
            } else if (picMode == PIC424) {
              PIC424Write(&bus_pirate_configuration.terminal_input[j + 1],
                          bus_pirate_configuration.terminal_input[j + 4]);
              j += 5;
            }
          } else if (bus_pirate_configuration.terminal_input[j] ==
                     2) { // read command
            if (picMode == PIC614) {
              PIC614Read(bus_pirate_configuration.terminal_input[j + 1]);
              j += 2;
            } else if (picMode == PIC416) {
              PIC416Read(bus_pirate_configuration.terminal_input[j + 1]);
              j += 2;
            } else if (picMode == PIC424) {
              PIC424Read();
              j++;
            }
          }
        }

        if (cmdr == 0)
          user_serial_transmit_character(1); // ACK

        break;
      default:
        user_serial_transmit_character(0x00); // send 0/Error
        break;
      }

      break;

      // case 0b0101: //# 0101xxxx - Bulk read, read 1-16bytes (0=1byte!)

    case 0b0100: // configure peripherals w=power, x=pullups, y=AUX, z=CS
      bp_binary_io_peripherals_set(input_byte);
      user_serial_transmit_character(1); // send 1/OK
      break;

#ifdef BUSPIRATEV4
    case 0b0101:
      user_serial_transmit_character(bp_binary_io_pullup_control(input_byte));
      break;
#endif

    case 0b0110:                   // set speed
      input_byte &= (~0b11111100); // clear command portion
      mode_configuration.speed = input_byte;
      bitbang_setup(wires, mode_configuration.speed);
      bitbang_set_cs(1); // takes care of custom HiZ settings too
      user_serial_transmit_character(1);
      break;

    case 0b1000: // set config
      // wxyz //w=HiZ(0)/3.3v(1), x=3wireenable, y=lsb, z=n/a
      mode_configuration.high_impedance = 0;
      if ((input_byte & 0b1000) == 0)
        mode_configuration.high_impedance = 1; // hiz output if this bit is 1

      wires = 2;
      if (input_byte & 0b100)
        wires = 3; // 3wire/2wire toggle

      mode_configuration.little_endian = NO;
      if (input_byte & 0b10)
        mode_configuration.little_endian = YES; // lsb/msb, bit order

      // if(inByte&0b1) //bit unused

      bitbang_setup(
          wires, mode_configuration.speed); // setup the bitbang library, must
                                            // be done before calling bbCS below
      bitbang_set_cs(1); // takes care of custom HiZ settings too
      user_serial_transmit_character(1); // send 1/OK
      break;

#ifdef BP_ENABLE_SMPS_SUPPORT
#ifdef BUSPIRATEV4
    case 0b1111: // SMPS commands
      switch (inByte) {
      case 0xf0:
        smps_adc(); // Send raw ADC reading
        break;
      case 0xf1:
        smps_stop();                       // Stop SMPS operation
        user_serial_transmit_character(1); // Send 1/OK
        break;
      default: {
        unsigned int V_out;

        V_out = inByte & 0x0f;
        V_out <<= 8;
        V_out |= user_serial_read_byte();
        smps_start(V_out);
        user_serial_transmit_character(1); // Send 1/OK
        break;
      }
      }
      break;
#endif /* BUSPIRATEV4 */
#endif /* BP_ENABLE_SMPS_SUPPORT */

    default:
      REPORT_IO_FAILURE();
      break;
    }
  }
}

void PIC614Read(unsigned char c) {
  unsigned char i;

  for (i = 0; i < 6; i++) {
    bitbang_write_bit(c & 0b1); // send bit
    c = c >> 1;                 // pop the LSB off
  }

  user_serial_transmit_character(bitbang_read_value());
  user_serial_transmit_character(bitbang_read_value());
}

void PIC614Write(unsigned char cmd, unsigned char datl, unsigned char dath) {
  unsigned char i, nodata;

  // MSB tells that there is no data output
  nodata = cmd & 0x80;

  for (i = 0; i < 6; i++) {
    bitbang_write_bit(cmd & 0b1); // send bit
    cmd = cmd >> 1;               // pop the LSB off
  }

  if (nodata)
    return;

  bitbang_write_value(datl); // send byte
  bitbang_write_value(dath); // send byte
}

void PIC416Read(unsigned char c) {
  unsigned char i;

  for (i = 0; i < 4; i++) {
    if (c & 0b1) {          // send 1
      bitbang_write_bit(1); // send bit
    } else {                // send 0
      bitbang_write_bit(0); // send bit
    }
    c = c >> 1; // pop the LSB off
  }

  bitbang_read_value(); // dummy byte, setup input
  user_serial_transmit_character(bitbang_read_value());
}

void PIC416Write(unsigned char cmd, unsigned char datl, unsigned char dath) {
  unsigned char i, delay;

  // use upper 2 bits of pic[0] to determine a delay, if any.
  delay = cmd >> 6;

  for (i = 0; i < 4; i++) {

    // hold data for write time
    if (i == 3 && (delay > 0)) {
      bitbang_set_clk(1);
      bp_delay_ms(delay);
      bitbang_set_clk(0);
      continue;
    }

    if (cmd & 0b1) {        // send 1
      bitbang_write_bit(1); // send bit
    } else {                // send 0
      bitbang_write_bit(0); // send bit
    }
    cmd = cmd >> 1; // pop the LSB off
  }

  bitbang_write_value(datl); // send byte
  bitbang_write_value(dath); // send byte
}

void PIC24NOP(void) {
  // send four bit SIX command (write)
  bitbang_write_bit(0); // send bit
  bitbang_write_bit(0); // send bit
  bitbang_write_bit(0); // send bit
  bitbang_write_bit(0); // send bit

  // send data payload
  bitbang_write_value(0x00); // send byte
  bitbang_write_value(0x00); // send byte
  bitbang_write_value(0x00); // send byte
}

void PIC424Write(unsigned char *cmd, unsigned char pn) {
  // send four bit SIX command (write)
  bitbang_write_bit(0); // send bit
  bitbang_write_bit(0); // send bit
  bitbang_write_bit(0); // send bit
  bitbang_write_bit(0); // send bit

  // send data payload
  bitbang_write_value(cmd[0]); // send byte
  bitbang_write_value(cmd[1]); // send byte
  bitbang_write_value(cmd[2]); // send byte

  // do any post instruction NOPs
  pn &= 0x0F;
  while (pn--) {
    PIC24NOP();
  }
}

void PIC424Write_internal(unsigned long cmd, unsigned char pn) {
  unsigned char i;
  // send four bit SIX command (write)
  bitbang_write_bit(0); // send bit
  bitbang_write_bit(0); // send bit
  bitbang_write_bit(0); // send bit
  bitbang_write_bit(0); // send bit

  // send data payload 0xBA0B96 0xBADBB6 0xBA0BB6
  bitbang_write_value(bp_reverse_byte(cmd & 0xFF));
  bitbang_write_value(bp_reverse_byte((cmd >> 8) & 0xFF));
  bitbang_write_value(bp_reverse_byte((cmd >> 16) & 0xFF));

  // do any post instruction NOPs
  pn &= 0x0F;
  for (i = 0; i < pn; i++) {
    PIC24NOP();
  }
}

void PIC424Read(void) {
  unsigned char c;

  // send four bit REGOUT command (read)
  bitbang_write_bit(1); // send bit
  bitbang_write_bit(0); // send bit
  bitbang_write_bit(0); // send bit
  bitbang_write_bit(0); // send bit

  // one byte output
  bitbang_write_value(0x00); // send byte

  // read 2 bytes
  // return bytes in little endian format
  c = bitbang_read_value();
  user_serial_transmit_character(bitbang_read_value());
  user_serial_transmit_character(c);

  // ALWAYS POST nop TWICE after a read
  PIC24NOP();
  PIC24NOP();
}

bool handle_binary_io_command(const binary_io_command command,
                              const size_t wires) {
  switch (command) {
  case IO_COMMAND_EXIT:
    /* @todo: Cleanup? */
    return false;

  case IO_COMMAND_REPORT_ID_STRING:
    MSG_RAW_MODE_IDENTIFIER;
    break;

  case IO_COMMAND_SEND_I2C_START_BIT:
    bitbang_i2c_start(BITBANG_I2C_START_ONE_SHOT);
    REPORT_IO_SUCCESS();
    break;

  case IO_COMMAND_SEND_I2C_STOP_BIT:
    bitbang_i2c_stop();
    REPORT_IO_SUCCESS();
    break;

  case IO_COMMAND_CS_LOW:
    bitbang_set_cs(LOW);
    REPORT_IO_SUCCESS();
    break;

  case IO_COMMAND_CS_HIGH:
    bitbang_set_cs(HIGH);
    REPORT_IO_SUCCESS();
    break;

  case IO_COMMAND_BITBANG_READ_BYTE: {
    uint16_t value =
        (wires == 2) ? bitbang_read_value() : bitbang_read_with_write(0xFF);
    if (mode_configuration.little_endian == YES) {
      value = bp_reverse_integer(value, mode_configuration.numbits);
    }
    user_serial_transmit_character(value & 0xFF);
    break;
  }

  case IO_COMMAND_BITBANG_READ_BIT:
    user_serial_transmit_character(bitbang_read_bit());
    break;

  case IO_COMMAND_PEEK_INPUT_BIT:
    user_serial_transmit_character(bitbang_read_miso());
    break;

  case IO_COMMAND_CLOCK_TICK:
    bitbang_advance_clock_ticks(1);
    REPORT_IO_SUCCESS();
    break;

  case IO_COMMAND_CLOCK_LOW:
    bitbang_set_clk(LOW);
    REPORT_IO_SUCCESS();
    break;

  case IO_COMMAND_CLOCK_HIGH:
    bitbang_set_clk(HIGH);
    REPORT_IO_SUCCESS();
    break;

  case IO_COMMAND_DATA_LOW:
    bitbang_set_mosi(LOW);
    REPORT_IO_SUCCESS();
    break;

  case IO_COMMAND_DATA_HIGH:
    bitbang_set_mosi(HIGH);
    REPORT_IO_SUCCESS();
    break;

  default:
    REPORT_IO_FAILURE();
    break;
  }

  return true;
}

void handle_bulk_transfer(const uint8_t command, const size_t wires) {
  size_t bytes = (command & 0x0F) + 1;
  REPORT_IO_SUCCESS();

  for (size_t counter = 0; counter < bytes; counter++) {
    uint16_t value = user_serial_read_byte();
    if (mode_configuration.little_endian == YES) {
      value = bp_reverse_integer(value, mode_configuration.numbits);
    }

    if (wires == 2) {
      bitbang_write_value(value & 0xFF);
      REPORT_IO_SUCCESS();
    } else {
      value = bitbang_read_with_write(value & 0xFF);
      if (mode_configuration.little_endian == YES) {
        value = bp_reverse_integer(value, mode_configuration.numbits);
      }
      bitbang_write_value(value & 0xFF);
    }
  }
}

void handle_bulk_clock_ticks(const uint8_t command) {
  bitbang_advance_clock_ticks((command & 0x0F) + 1);
  REPORT_IO_SUCCESS();
}

void handle_bulk_bits(const uint8_t command) {
  size_t bits = (command & 0x0F) + 1;
  REPORT_IO_SUCCESS();

  uint8_t byte = user_serial_read_byte();
  for (size_t counter = 0; counter < bits; counter++) {
    bitbang_write_bit((command & 0x80) ? HIGH : LOW);
    byte <<= 1;
  }

  REPORT_IO_SUCCESS();
}
