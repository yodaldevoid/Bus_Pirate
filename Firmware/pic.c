/*
 * This file is part of the Bus Pirate project
 * (http://code.google.com/p/the-bus-pirate/).
 *
 * Initially written by Chris van Dongen, 2010.
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

#include "pic.h"

#ifdef BP_ENABLE_PIC_SUPPORT

#include "base.h"

#include "aux_pin.h"
#include "binary_io.h"
#include "bitbang.h"
#include "core.h"
#include "proc_menu.h"

extern bus_pirate_configuration_t bus_pirate_configuration;
extern mode_configuration_t mode_configuration;
extern bool command_error;

static int picmode;
static int piccmddelay;

static void clock_out_zero(void);
static void clock_out_data(const uint16_t data, const uint8_t bits);
static uint16_t clock_in_data(const uint8_t bits);

void pic_setup_prepare(void) {
  int interactive;

  consumewhitechars();
  int mode = getint();
  consumewhitechars();
  int delay = getint();
  interactive = 0;

  if (!((mode > 0) && (mode <= 2))) {
    interactive = 1;
  }

  if ((delay > 0) && (delay <= 2)) {
    piccmddelay = delay;
  } else {
    interactive = 1;
  }

  if (interactive) {
    command_error = false;
    MSG_PIC_MODE_PROMPT;
    mode = getnumber(1, 1, 2, 0);
    MSG_PIC_DELAY_PROMPT;
    delay = getnumber(1, 1, 2, 0);
  }

  switch (mode) {
  case 1:
    picmode = PICMODE6;
    break;

  case 2:
    picmode = PICMODE4;
    break;

  default:
    break;
  }
  piccmddelay = delay;

  if (!interactive) {
    BPMSG1074;
    bp_write_dec_byte(picmode);
    bpSP;
    bp_write_dec_byte(piccmddelay);
    bp_write_line(")");
  }

  /* Allow for different Vcc. */
  mode_configuration.high_impedance = YES;
  mode_configuration.int16 = YES;

  /* Pull both pins low before applying Vcc and Vpp. */
  bitbang_set_pins_low(MOSI | CLK, PICSPEED);
}

void pics_setup_execute(void) { mode_configuration.int16 = YES; }

void pic_cleanup(void) { mode_configuration.int16 = NO; }

void pic_start(void) {
  // command mode
  picmode |= PICCMD;
  // bpWstring("CMD");
  BPMSG1075;
  user_serial_transmit_character(
      0x30 + (picmode & PICMODEMSK)); // display #commandbits
  mode_configuration.int16 = NO;
  bpBR;
}

void pic_stop(void) {
  // data mode
  picmode &= PICMODEMSK;
  mode_configuration.int16 = YES;
  BPMSG1076;
}

uint16_t clock_in_data(const uint8_t bits) {
  uint16_t data = 0;

  for (size_t index = 0; index < bits; index++) {
    bitbang_set_pins_high(CLK, PICSPEED / 2);
    data = (data >> 1) |
           ((bitbang_read_pin(MOSI) == HIGH ? 1 : 0) << (1 << (bits - 1)));
    bitbang_set_pins_low(CLK, PICSPEED / 2);
  }

  bitbang_set_pins_high(CLK, PICSPEED / 2);
  bitbang_set_pins_low(CLK, PICSPEED / 2);
  bitbang_set_pins_low(MOSI, PICSPEED / 5);

  return data;
}

unsigned int pic_read(void) {
  if (picmode & PICCMDMSK) {
    BPMSG1077;
    return 0;
  }

  switch (picmode & PICMODEMSK) {
  case PICMODE6:
    // switch to input
    bitbang_read_pin(MOSI);
    bitbang_set_pins_high(CLK, PICSPEED / 2);
    bitbang_set_pins_low(CLK, PICSPEED / 2);
    return clock_in_data(14);

  case PICMODE4:
    // switch to input
    bitbang_read_pin(MOSI);
    return clock_in_data(16);

  default:
    MSG_PIC_UNKNOWN_MODE;
    return 0;
  }
}

void clock_out_data(const uint16_t data, const uint8_t bits) {
  uint8_t mask = 1;

  for (size_t index = 0; index < bits; index++) {
    bitbang_set_pins_high(CLK, PICSPEED / 4);
    bitbang_set_pins((data & mask) == mask, MOSI, PICSPEED / 4);
    bitbang_set_pins_low(CLK, PICSPEED / 4);
    bitbang_set_pins_low(MOSI, PICSPEED / 4);
    mask <<= 1;
  }
}

void clock_out_zero(void) {
  bitbang_set_pins_high(CLK, PICSPEED / 4);
  bitbang_set_pins_low(MOSI, PICSPEED / 4);
  bitbang_set_pins_low(CLK, PICSPEED / 4);
  bitbang_set_pins_low(CLK, PICSPEED / 4);
}

uint16_t pic_write(const uint16_t value) {
  if (picmode & PICCMDMSK) {
    switch (picmode & PICMODEMSK) {
    case PICMODE6:
      clock_out_data(value, 6);
      break;

    case PICMODE4:
      clock_out_data(value, 4);
      break;

    default:
      MSG_PIC_UNKNOWN_MODE;
      return 0;
    }

    bp_delay_ms(piccmddelay);
  } else {
    switch (picmode & PICMODEMSK) {
    case PICMODE6:
      clock_out_zero();
      clock_out_data(value, 14);
      clock_out_zero();
      break;

    case PICMODE4:
      clock_out_data(value, 16);
      break;

    default:
      MSG_PIC_UNKNOWN_MODE;
      return 0;
    }
  }

  return 0x100; // no data to display
}

void pic_run_macro(unsigned int macro) {
  unsigned int temp;

  switch (macro) {
  case 0: // bpWline("(1) get devID");
    BPMSG1079;
    break;

  case 1:
    switch (picmode & PICMODEMSK) {
    case PICMODE6:
      bus_pirate_configuration.quiet = YES;
      pic_start();
      pic_write(0);
      pic_stop();
      pic_write(0); // advance to 0x2006 (devid)
      pic_start();
      for (size_t index = 0; index < 6; index++) {
        pic_write(6);
      }
      pic_write(4);
      pic_stop();
      temp = pic_read();
      bus_pirate_configuration.quiet = NO;
      // bpWstring("DevID = ");
      BPMSG1080;
      bp_write_hex_word(temp >> 5);
      // bpWstring(" Rev = ");
      BPMSG1081;
      bp_write_hex_byte(temp & 0x1f);
      bpBR;
      break;

    case PICMODE4:
    default: // bpWline("Not implemented (yet)");
      BPMSG1082;
    }
    // bpWline("Please exit PIC programming mode");
    BPMSG1083;
    break;

  default:
    MSG_UNKNOWN_MACRO_ERROR;
    break;
  }
}

void pic_print_pins_state(void) { MSG_PIC_PINS_STATE; }

/*
0000 0000	return to main
0000 0001	id=PIC1
0000 0010	6b cmd
0000 0011	4b cmd
0000 01xx	xx ms delay

0000 1xxx	unimplemented

0001 0xyz	PWM|VREG|PULLUP
0001 1xyz   AUX|MISO|CS

01xx xxxx	just send cmd xxxxxxx
10xx xxxx	send cmd xxxxxxx and next two bytes (14/16 bits)
11xx xxxx	send cmd xxxxxxx and read two bytes

*/

void binpic(void) {
  int ok;
  unsigned int temp;

  MSG_PIC_MODE_IDENTIFIER;
  mode_configuration.high_impedance =
      YES; // to allow different Vcc
           // pull both pins to 0 before applying Vcc and Vpp
  bitbang_set_pins_low(MOSI | CLK, PICSPEED);
  picmode = PICMODE6;
  piccmddelay = 2;

  for (;;) {
    uint8_t command = user_serial_read_byte();

    switch (command & 0xC0) {
    case 0x00:
      ok = 1;
      switch (command & 0xF0) {
      case 0x00:
        switch (command) {
        case 0x00:
          return;

        case 0x01:
          MSG_PIC_MODE_IDENTIFIER;
          break;

        case 0x02:
          picmode = PICMODE6;
          break;

        case 0x03:
          picmode = PICMODE4;
          break;

        case 0x04:
        case 0x05:
        case 0x06:
        case 0x07:
          piccmddelay = (command - 0x04);
          break;

        default:
          ok = 0;
        }
        break;

      case 0x10:
        if (command & 0x08) {
          bitbang_set_pins((command & 0x04) == 0x04, AUX, 5);
          bitbang_set_pins((command & 0x02) == 0x02, MISO, 5);
          bitbang_set_pins((command & 0x01) == 0x01, CS, 5);
        } else {
          if (command & 0x04) {
            bp_update_pwm(100, 50);
          } else {
            bp_update_pwm(PWM_OFF, PWM_OFF);
          }
          bp_set_voltage_regulator_state((command & 0x02) == 0x02);
          bp_set_pullup_state((command & 0x01) == 0x01);
        }
        break;

      default:
        ok = 0;
        break;
      }

      user_serial_transmit_character(ok != 0 ? BP_BINARY_IO_RESULT_SUCCESS
                                             : BP_BINARY_IO_RESULT_FAILURE);
      break;

    case 0x40:
      picmode |= PICCMD;
      pic_write(command & 0x3F);
      picmode &= PICMODEMSK;
      user_serial_transmit_character(1);
      break;
    case 0x80:
      picmode |= PICCMD;
      pic_write(command & 0x3F);
      picmode &= PICMODEMSK;
      temp = user_serial_read_byte();
      temp <<= 8;
      temp |= user_serial_read_byte();
      pic_write(temp);
      user_serial_transmit_character(1);
      break;

    case 0xC0:
      picmode |= PICCMD;
      pic_write(command & 0x3F);
      picmode &= PICMODEMSK;
      REPORT_IO_SUCCESS();
      temp = pic_read();
      user_serial_transmit_character(temp >> 8);
      user_serial_transmit_character(temp & 0x0FF);
      break;
    }
  }
}

#endif /* BP_ENABLE_PIC_SUPPORT */