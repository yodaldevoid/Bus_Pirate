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

#include "base.h"
#include "core.h"

static const uint8_t HEX_PREFIX[] = {'0', 'x'};

static const unsigned char HEXASCII[] = {'0', '1', '2', '3', '4', '5',
                                         '6', '7', '8', '9', 'A', 'B',
                                         'C', 'D', 'E', 'F'};

#if defined(BUSPIRATEV4)
extern BYTE cdc_In_len;
extern BYTE cdc_Out_len;
#endif /* BUSPIRATEV4 */

extern bus_pirate_configuration_t bus_pirate_configuration;
extern mode_configuration_t mode_configuration;

#ifdef BUSPIRATEV3
// Internal FRC OSC = 8MHz
#pragma config FNOSC = FRCPLL
#pragma config OSCIOFNC = ON
#pragma config POSCMOD = NONE
#pragma config I2C1SEL = PRI
#pragma config JTAGEN = OFF
#pragma config GCP = OFF
#pragma config GWRP = OFF
#pragma config FWDTEN = OFF
#pragma config ICS = PGx1
#endif /* BUSPIRATEV3 */

#ifdef BUSPIRATEV4
#pragma config JTAGEN = OFF
#pragma config GCP = OFF
#pragma config GWRP = OFF
#pragma config FWDTEN = OFF
#pragma config ICS = PGx2
#pragma config IESO = OFF
#pragma config FCKSM = CSDCMD
#pragma config OSCIOFNC = ON
#pragma config POSCMOD = HS
#pragma config FNOSC = PRIPLL
#pragma config PLLDIV = DIV3
#pragma config IOL1WAY = ON
#pragma config PLL_96MHZ = ON
#pragma config DISUVREG = OFF
#endif /* BUSPIRATEV4 */

#ifdef BUSPIRATEV4

/**
 * Precomputed table with the reversed bit representation of all possible
 * 8-bits integers.
 *
 * On v4 flash space is not a concern, so we can speed things up a bit without
 * worrying too much about it.
 */
static const uint8_t REVERSED_BITS_TABLE[] = {
    0x00, 0x80, 0x40, 0xC0, 0x20, 0xA0, 0x60, 0xE0, 0x10, 0x90, 0x50, 0xD0,
    0x30, 0xB0, 0x70, 0xF0, 0x08, 0x88, 0x48, 0xC8, 0x28, 0xA8, 0x68, 0xE8,
    0x18, 0x98, 0x58, 0xD8, 0x38, 0xB8, 0x78, 0xF8, 0x04, 0x84, 0x44, 0xC4,
    0x24, 0xA4, 0x64, 0xE4, 0x14, 0x94, 0x54, 0xD4, 0x34, 0xB4, 0x74, 0xF4,
    0x0C, 0x8C, 0x4C, 0xCC, 0x2C, 0xAC, 0x6C, 0xEC, 0x1C, 0x9C, 0x5C, 0xDC,
    0x3C, 0xBC, 0x7C, 0xFC, 0x02, 0x82, 0x42, 0xC2, 0x22, 0xA2, 0x62, 0xE2,
    0x12, 0x92, 0x52, 0xD2, 0x32, 0xB2, 0x72, 0xF2, 0x0A, 0x8A, 0x4A, 0xCA,
    0x2A, 0xAA, 0x6A, 0xEA, 0x1A, 0x9A, 0x5A, 0xDA, 0x3A, 0xBA, 0x7A, 0xFA,
    0x06, 0x86, 0x46, 0xC6, 0x26, 0xA6, 0x66, 0xE6, 0x16, 0x96, 0x56, 0xD6,
    0x36, 0xB6, 0x76, 0xF6, 0x0E, 0x8E, 0x4E, 0xCE, 0x2E, 0xAE, 0x6E, 0xEE,
    0x1E, 0x9E, 0x5E, 0xDE, 0x3E, 0xBE, 0x7E, 0xFE, 0x01, 0x81, 0x41, 0xC1,
    0x21, 0xA1, 0x61, 0xE1, 0x11, 0x91, 0x51, 0xD1, 0x31, 0xB1, 0x71, 0xF1,
    0x09, 0x89, 0x49, 0xC9, 0x29, 0xA9, 0x69, 0xE9, 0x19, 0x99, 0x59, 0xD9,
    0x39, 0xB9, 0x79, 0xF9, 0x05, 0x85, 0x45, 0xC5, 0x25, 0xA5, 0x65, 0xE5,
    0x15, 0x95, 0x55, 0xD5, 0x35, 0xB5, 0x75, 0xF5, 0x0D, 0x8D, 0x4D, 0xCD,
    0x2D, 0xAD, 0x6D, 0xED, 0x1D, 0x9D, 0x5D, 0xDD, 0x3D, 0xBD, 0x7D, 0xFD,
    0x03, 0x83, 0x43, 0xC3, 0x23, 0xA3, 0x63, 0xE3, 0x13, 0x93, 0x53, 0xD3,
    0x33, 0xB3, 0x73, 0xF3, 0x0B, 0x8B, 0x4B, 0xCB, 0x2B, 0xAB, 0x6B, 0xEB,
    0x1B, 0x9B, 0x5B, 0xDB, 0x3B, 0xBB, 0x7B, 0xFB, 0x07, 0x87, 0x47, 0xC7,
    0x27, 0xA7, 0x67, 0xE7, 0x17, 0x97, 0x57, 0xD7, 0x37, 0xB7, 0x77, 0xF7,
    0x0F, 0x8F, 0x4F, 0xCF, 0x2F, 0xAF, 0x6F, 0xEF, 0x1F, 0x9F, 0x5F, 0xDF,
    0x3F, 0xBF, 0x7F, 0xFF,
};

#endif /* BUSPIRATEV4 */

/**
 * Clear configuration on mode change.
 */
static void clear_mode_configuration(void);

void clear_mode_configuration(void) {
  mode_configuration.high_impedance = 0;
  mode_configuration.speed = 0;
  mode_configuration.periodicService = 0;
  mode_configuration.alternate_aux = 0;
  mode_configuration.lsbEN = 0;
}

void bp_reset_board_state(void) {
  BP_MOSI_DIR = INPUT;
  BP_CLK_DIR = INPUT;
  BP_MISO_DIR = INPUT;
  BP_CS_DIR = INPUT;
  BP_AUX0_DIR = INPUT;
  BP_LEDMODE = OFF;
  BP_LEDMODE_DIR = OUTPUT;
#ifdef BUSPIRATEV4
  BP_USBLED_OFF();
  BPV4_HWI2CPINS_SETUP();
  BP_BUTTON_SETUP();
  BP_3V3PU_OFF();
  eeprom_initialize();
#endif /* BUSPIRATEv4 */

  BP_AUX_RPOUT = 0; // remove output from AUX pin (PWM/servo modes)

  bus_pirate_configuration.bus_mode = BP_HIZ;
  clear_mode_configuration();
  BP_PULLUP_OFF();
  BP_VREG_OFF();
  // setup voltage monitoring on ADC. see hardwarevx.h!
  BP_ADC_PINSETUP();

  // configure the ADC
  AD1CON1bits.SSRC = 0b111; // SSRC<3:0> = 111 implies internal
                            // counter ends sampling and starts
                            // converting.
  AD1CSSL = 0;
  AD1CON3 = 0x1F02; // Sample time = 31Tad, Tad = 2 Tcy
  AD1CON2 = 0;
}

unsigned int bp_read_adc(unsigned int channel) {

  /* Set channel. */
  AD1CHS = channel;

  /* Trigger sample. */
  AD1CON1bits.SAMP = ON;

  /* Clear "done" flag. */
  AD1CON1bits.DONE = OFF;

  /* Wait for conversion to finish. */
  while (AD1CON1bits.DONE == OFF) {
  }

  /* Return value. */
  return ADC1BUF0;
}

void bp_adc_probe(void) {
  /* Turn the ADC on. */
  AD1CON1bits.ADON = ON;

  /* Perform the measurement. */
  bp_write_voltage(bp_read_adc(BP_ADC_PROBE));

  /* Turn the ADC off. */
  AD1CON1bits.ADON = OFF;
}

void bp_adc_continuous_probe(void) {
  unsigned int measurement;

  // bpWline(OUMSG_PS_ADCC);
  BPMSG1042;
  MSG_ANY_KEY_TO_EXIT_PROMPT;
  // bpWstring(OUMSG_PS_ADC_VOLT_PROBE);
  BPMSG1044;
  bp_write_voltage(0);
  // bpWstring(OUMSG_PS_ADC_VOLTS);
  BPMSG1045;

  /* Perform ADC probes until a character is sent to the serial port. */
  while (!UART1RXRdy()) {
    /* Turn the ADC on. */
    AD1CON1bits.ADON = ON;

    /* Perform the measurement. */
    measurement = bp_read_adc(BP_ADC_PROBE);

    /* Turn the ADC off. */
    AD1CON1bits.ADON = OFF;

    /* Erase previous measurement. */
    bp_write_string("\x08\x08\x08\x08\x08");

    /* Print new measurement. */
    bp_write_voltage(measurement);

    BPMSG1045;
  }

  /* Flush the incoming serial buffer. */
  UART1RX();

  bpBR;
}

void bp_write_formatted_integer(unsigned int value) {
  if (mode_configuration.numbits < 16) {
    value &= 0x7FFF >> ((16 - mode_configuration.numbits) - 1);
  }

  switch (bus_pirate_configuration.display_mode) {
  case HEX:
    if (mode_configuration.int16) {
      bp_write_hex_word(value);
    } else {
      bp_write_hex_byte(value);
    }
    break;

  case DEC:
    if (mode_configuration.int16) {
      bp_write_dec_word(value);
    } else {
      bp_write_dec_byte(value);
    }
    break;

  case BIN:
    if (mode_configuration.int16) {
      bp_write_bin_byte(value);
      bpSP;
    }
    bp_write_bin_byte(value);
    break;

  case RAW:
    if (mode_configuration.int16) {
      UART1TX(value >> 8);
    }
    UART1TX(value & 0xFF);
    break;
  }
}

inline uint8_t bp_reverse_byte(const uint8_t value) {
#ifdef BUSPIRATEV4
  return REVERSED_BITS_TABLE[value];
#else
  return bp_reverse_integer(value, 8);
#endif /* BUSPIRATEV4 */
}

inline uint16_t bp_reverse_word(const uint16_t value) {
#ifdef BUSPIRATEV4
  return (REVERSED_BITS_TABLE[value & 0xFF] << 8) |
         REVERSED_BITS_TABLE[value >> 8];
#else
  return bp_reverse_integer(value, 16);
#endif /* BUSPIRATEV4 */
}

uint16_t bp_reverse_integer(const uint16_t value, const uint8_t bits) {
#ifdef BUSPIRATEV4
  return ((REVERSED_BITS_TABLE[value & 0xFF] << 8) |
          REVERSED_BITS_TABLE[value >> 8]) >>
         ((sizeof(uint16_t) * 8) - bits);
#else
  uint16_t reversed;
  uint16_t bitmask;

  reversed = 0;
  bitmask = 1;
  while (bitmask != 0) {
    reversed <<= 1;
    if (value & bitmask) {
      reversed |= 0b00000001;
    }
    bitmask <<= 1;
  }

  return reversed >> ((sizeof(uint16_t) * 8) - bits);
#endif /* BUSPIRATEV4 */
}

void bp_write_buffer(const uint8_t *buffer, size_t length) {
  size_t offset;

  for (offset = 0; offset < length; offset++) {
    UART1TX(buffer[offset]);
  }
}

void bp_write_string(const char *string) {
  char character;
  while ((character = *string++)) {
    UART1TX(character);
  }
}

void bp_write_line(const char *string) {
  bp_write_string(string);

  UART1TX(0x0D);
  UART1TX(0x0A);
}

void bp_write_bin_byte(uint8_t value) {
  uint8_t mask;
  size_t index;
 
  mask = 0x80;

  MSG_BINARY_NUMBER_PREFIX;

  for (index = 0; index < 8; index++) {
    UART1TX((value & mask) ? '1' : '0');
    mask >>= 1;
  }
}

void bp_write_dec_dword(uint32_t l) {
  unsigned long c, m;
  unsigned char j, k = 0;

  c = 100000000;
  for (j = 0; j < 8; j++) {
    m = l / c;
    if (k || m) {
      UART1TX(m + '0');
      l = l - (m * c);
      k = 1;
    }
    c /= 10;
  }
  UART1TX(l + '0');
}

// userfriendly printing of looooonng ints

void bp_write_dec_dword_friendly(unsigned long l) {
  unsigned long int temp;
  int mld, mil;

  mld = 0;
  mil = 0;
  temp = 0;

  if (l >= 1000000) {
    temp = l / 1000000;
    bp_write_dec_word(temp);
    UART1TX(',');
    l %= 1000000;
    if (l < 1000)
      bp_write_string("000,");
    mld = 1;
    mil = 1;
  }
  if (l >= 1000) {
    temp = l / 1000;
    if (temp >= 100) {
      bp_write_dec_word(temp);
    } else if (mld) {
      if (temp >= 10) {
        UART1TX('0'); // 1 leading zero
      } else {
        bp_write_string("00");
      }
      bp_write_dec_word(temp);
    } else
      bp_write_dec_word(temp);
    UART1TX(',');
    l %= 1000;
    mil = 1;
  }
  if (l >= 100) {
    bp_write_dec_word(l);
  } else if (mil) {
    if (l >= 10) {
      UART1TX('0'); // 1 leading zero
    } else {
      bp_write_string("00");
    }
    bp_write_dec_word(l);
  } else
    bp_write_dec_word(l);
}

// output an 16bit/integer decimal value to the user terminal

void bp_write_dec_word(unsigned int i) {
  unsigned int c, m;
  unsigned char j, k = 0;

  c = 10000;
  for (j = 0; j < 4; j++) {
    m = i / c;
    if (k || m) {
      UART1TX(m + '0');
      i = i - (m * c);
      k = 1;
    }
    c /= 10;
  }
  UART1TX(i + '0');
}

// output an 8bit/byte decimal value to the user terminal

void bp_write_dec_byte(unsigned char c) {
  unsigned char d, j, m, k = 0;

  d = 100;
  for (j = 0; j < 2; j++) {
    m = c / d;
    if (k || m) {
      UART1TX(m + '0');
      c = c - (m * d);
      k = 1;
    }
    d /= 10;
  }
  UART1TX(c + '0');
}

void bp_write_hex_byte(uint8_t value) {
  MSG_HEXADECIMAL_NUMBER_PREFIX;
  UART1TX(HEXASCII[(value >> 4) & 0x0F]);
  UART1TX(HEXASCII[value & 0x0F]);
}

void bp_write_hex_byte_to_ringbuffer(uint8_t value) {
  UARTbuf(HEX_PREFIX[0]);
  UARTbuf(HEX_PREFIX[1]);
  UARTbuf(HEXASCII[(value >> 4) & 0x0F]);
  UARTbuf(HEXASCII[value & 0x0F]);
}

// output a 16bit hex value to the user terminal

void bp_write_hex_word(uint16_t value) {
  MSG_HEXADECIMAL_NUMBER_PREFIX;
  UART1TX(HEXASCII[(value >> 12) & 0x0F]);
  UART1TX(HEXASCII[(value >> 8) & 0x0F]);
  UART1TX(HEXASCII[(value >> 4) & 0x0F]);
  UART1TX(HEXASCII[value & 0x0F]);
}

// print an ADC measurement in decimal form

void bp_write_voltage(const unsigned int adc) {
  unsigned char c;

  // input voltage is divided by two and compared to 3.3V
  // volt      = adc / 1024 * 3.3V * 2
  // centivolt = adc / 1024 * 3.3 * 2 * 100 = adc * 165 / 256
  // This is approximately (adc * 29 / 45), making the calculation
  // fit in an unsigned int. The error is less than 1mV.
  const unsigned int centivolt = (adc * 29) / 45;

  bp_write_dec_byte(centivolt / 100);

  UART1TX('.');

  c = centivolt % 100;

  if (c < 10) // need extra zero?
    UART1TX('0');

  bp_write_dec_byte(centivolt % 100);
}

// Read the lower 16 bits from programming flash memory

unsigned int bpReadFlash(unsigned int page, unsigned int addr) {
  unsigned int tblold;
  unsigned flash;

  tblold = TBLPAG;
  TBLPAG = page;
  flash = (__builtin_tblrdh(addr) << 8) | __builtin_tblrdl(addr);
  TBLPAG = tblold;

  return flash;
}

#ifdef BUSPIRATEV3

#ifndef BP_ENABLE_UART_SUPPORT
static const uint16_t UART_BRG_SPEED[] = {
    13332, /* 300 bps */
    3332,  /* 1200 bps */
    1666,  /* 2400 bps */
    832,   /* 4800 bps */
    416,   /* 9600 bps */
    207,   /* 19200 bps */
    103,   /* 38400 bps */
    68,    /* 57600 bps */
    34     /* 115200 bps */
};
#else
extern const uint16_t UART_BRG_SPEED[];
#endif /* !BP_ENABLE_UART_SUPPORT */

void InitializeUART1(void) {
  if (bus_pirate_configuration.terminal_speed != 9) {
    U1BRG = UART_BRG_SPEED[bus_pirate_configuration.terminal_speed];
  }
  U1MODE = 0;
  U1MODEbits.BRGH = 1;
  U1STA = 0;
  U1MODEbits.UARTEN = 1;
  U1STAbits.UTXEN = 1;
  IFS0bits.U1RXIF = 0;
}
unsigned char UART1TXEmpty(void) { return U1STAbits.TRMT; }

inline bool UART1RXRdy(void) { return U1STAbits.URXDA; }

// new UART ring buffer
// uses user terminal input buffer to buffer UART output
// any existing user input will be destroyed
// best used for binary mode and sniffers
// static struct _UARTRINGBUF{
static unsigned int writepointer;
static unsigned int readpointer;
//}ringBuf;

void UARTbufSetup(void) {
  // setup ring buffer pointers
  readpointer = 0;
  writepointer = 1;
  bus_pirate_configuration.overflow = 0;
}

void UARTbufService(void) {
  unsigned int i;
  if (U1STAbits.UTXBF == 0) { // check first for free slot

    i = readpointer + 1;
    if (i == BP_TERMINAL_BUFFER_SIZE)
      i = 0; // check for wrap
    if (i == writepointer)
      return; // buffer empty,
    readpointer = i;
    U1TXREG = bus_pirate_configuration
                  .terminal_input[readpointer]; // move a byte to UART
  }
}

void UARTbufFlush(void) {
  unsigned int i;

  while (1) {
    i = readpointer + 1;
    if (i == BP_TERMINAL_BUFFER_SIZE)
      i = 0; // check for wrap
    if (i == writepointer)
      return; // buffer empty,

    if (U1STAbits.UTXBF == 0) { // free slot, move a byte to UART
      readpointer = i;
      U1TXREG = bus_pirate_configuration.terminal_input[readpointer];
    }
  }
}

void UARTbuf(char c) {
  if (writepointer == readpointer) {
    BP_LEDMODE = 0; // drop byte, buffer full LED off
    bus_pirate_configuration.overflow = 1;
  } else {
    bus_pirate_configuration.terminal_input[writepointer] = c;
    writepointer++;
    if (writepointer == BP_TERMINAL_BUFFER_SIZE)
      writepointer = 0; // check for wrap
  }
}

// get a byte from UART

unsigned char UART1RX(void) {
  while (U1STAbits.URXDA == 0) {
  }
  return U1RXREG;
}

void WAITTXEmpty(void) {
  while (U1STAbits.TRMT == 0) {
  }
}
// add byte to buffer, pause if full
// uses PIC 4 byte UART FIFO buffer

void UART1TX(char c) {
  if (bus_pirate_configuration.quiet)
    return;
  while (U1STAbits.UTXBF == 1) {
  }
  U1TXREG = c;
}

void UART1Speed(unsigned char brg) { U1BRG = brg; }

unsigned char CheckCommsError(void) {
  return U1STAbits.OERR; // check for user terminal buffer overflow error
}

void ClearCommsError(void) {
  U1STA &= (~0b10); // clear overrun error if exists
}

/* interrupt transfer related stuff */
unsigned char __attribute__((section(".bss.filereg"))) * UART1RXBuf;
unsigned int __attribute__((section(".bss.filereg"))) UART1RXToRecv;
unsigned int __attribute__((section(".bss.filereg"))) UART1RXRecvd;
unsigned char __attribute__((section(".bss.filereg"))) * UART1TXBuf;
unsigned int __attribute__((section(".bss.filereg"))) UART1TXSent;
unsigned int __attribute__((section(".bss.filereg"))) UART1TXAvailable;

void UART1TXInt() {
  if (IEC0bits.U1TXIE == 1)
    return;
  if (UART1TXAvailable == UART1TXSent)
    return;

  while (U1STAbits.UTXBF == 1)
    ; // if buffer is full, wait

  IFS0bits.U1TXIF = 0;

  IEC0bits.U1TXIE = 1;

  U1TXREG = UART1TXBuf[UART1TXSent];
}

// UART1 ISRs

void __attribute__((interrupt, no_auto_psv)) _U1RXInterrupt(void) {
  UART1RXBuf[UART1RXRecvd] = U1RXREG;
  UART1RXRecvd++;

  if (UART1RXRecvd == UART1RXToRecv) {
    // disable UART1 RX interrupt
    IEC0bits.U1RXIE = 0;
  }

  IFS0bits.U1RXIF = 0;
}

void __attribute__((interrupt, no_auto_psv)) _U1TXInterrupt(void) {
  UART1TXSent++;
  if (UART1TXSent == UART1TXAvailable) {
    // if everything is sent  disale interrupts
    IEC0bits.U1TXIE = 0;
  } else {
    // feed next byte
    U1TXREG = UART1TXBuf[UART1TXSent];
  }

  IFS0bits.U1TXIF = 0;
}

#endif /* BUSPIRATEV3 */

#if defined(BUSPIRATEV4)

extern BDentry *CDC_Outbdp, *CDC_Inbdp;

void UART1TX(char c) {
  if (bus_pirate_configuration.quiet)
    return;
  putc_cdc(c);
}

void UARTbuf(char c) { UART1TX(c); }

void WAITTXEmpty(void) { WaitInReady(); }

unsigned char UART1TXEmpty(void) { return 1; }

// is data available?

inline bool UART1RXRdy(void) {
    return cdc_Out_len || getOutReady();
}

// get a byte from UART

unsigned char UART1RX(void) { return getc_cdc(); }

void UARTbufFlush(void) { CDC_Flush_In_Now(); }

unsigned char CheckCommsError(void) {
  return 0; // check for user terminal buffer overflow error
}

void UARTbufSetup(void) {}

void UARTbufService(void) {}

void ClearCommsError(void) {}

void InitializeUART1(void) {}

void UART1Speed(unsigned char brg) {}

#endif /* BUSPIRATEV4 */
