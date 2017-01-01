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

#include "base.h"
#include "core.h"

static const uint8_t HEX_PREFIX[] = {'0', 'x'};

static const unsigned char HEXASCII[] = {'0', '1', '2', '3', '4', '5',
                                         '6', '7', '8', '9', 'A', 'B',
                                         'C', 'D', 'E', 'F'};

extern bus_pirate_configuration_t bus_pirate_configuration;

#if defined(BUSPIRATEV4)
extern BYTE cdc_In_len;
extern BYTE cdc_Out_len;
#endif /* BUSPIRATEV4 */

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
  UART1TX(HEXASCII[(value >> 4) & 0x0F]);
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

unsigned char UART1RXRdy(void) { return U1STAbits.URXDA; }

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
  while (U1STAbits.URXDA == 0)
    ;
  return U1RXREG;
}

void WAITTXEmpty(void) {
  while (U1STAbits.TRMT == 0)
    ;
}
// add byte to buffer, pause if full
// uses PIC 4 byte UART FIFO buffer

void UART1TX(char c) {
  if (bus_pirate_configuration.quiet)
    return;
  while (U1STAbits.UTXBF == 1)
    ; // if buffer is full, wait
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

unsigned char UART1RXRdy(void) {
  if (cdc_Out_len)
    return 1;
  if (getOutReady())
    return 1;
  return 0;
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
