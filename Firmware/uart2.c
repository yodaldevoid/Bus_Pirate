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

#include "uart2.h"

#include "base.h"

/* PPS Settings. */

#define UARTRX_PIN BP_MISO_RPIN
#define UARTTX_PIN BP_MOSI_RPOUT
#define UARTTX_ODC BP_MOSI_ODC

#define DATABITS_AND_PARITY_MASK 0b00000011

void uart2_setup(const uint16_t baud_rate_generator_prescaler,
                 const bool open_drain_output, const bool invert_polarity,
                 const uint8_t databits_and_parity, const bool stop_bits) {
  /* Set open drain output mode. */
  UARTTX_ODC = open_drain_output;

  /* Map RB7 as UART TX. */
  RPINR19bits.U2RXR = UARTRX_PIN;
  UARTTX_PIN = U2TX_IO;

  /* Set UART2 baud rate generator prescaler. */
  U2BRG = baud_rate_generator_prescaler;

  /*
   * Initialise UART2.
   *
   * MSB
   * 0x000x0000000000
   * | ||| ||||||||||
   * | ||| |||||||||+--> STSEL:  One stop bit.
   * | ||| |||||||++---> PDSEL:  8-bit data, no parity.
   * | ||| ||||||+-----> BRGH:   Standard mode (baud clock from Fcy/16).
   * | ||| |||||+------> RXINV:  RX Idle state is '1'.
   * | ||| ||||+-------> ABAUD:  Baud rate measurement disabled.
   * | ||| |||+--------> LPBACK: Loopback mode disabled.
   * | ||| ||+---------> WAKE:   No wake-up enabled.
   * | ||| ++----------> UEN:    CTS and RTS controlled by port latches.
   * | ||+-------------> RTSMD:  RTS pin in Flow Control mode.
   * | |+--------------> IREN:   IrDA encoder/decoder disabled.
   * | +---------------> USIDL:  Continue module operation in Idle mode.
   * +-----------------> UARTEN: Disable port.
   */
  U2MODE = 0;

  /* Enable high speed UART mode. */
  U2MODEbits.BRGH = ON;

  /* Set polarity inversion on RX pin. */
  U2MODEbits.RXINV = invert_polarity;

  /* Set databits and parity. */
  U2MODEbits.PDSEL = databits_and_parity & DATABITS_AND_PARITY_MASK;

  /* Set stop bits. */
  U2MODEbits.STSEL = stop_bits;

  /*
   * Set UART2 status bits.
   *
   * MSB
   * 000x00--000---0-
   * ||| ||  |||   |
   * ||| ||  |||   +---> OERR:    Clear overrun error status bit.
   * ||| ||  ||+-------> ADDEN:   Address detect mode disabled.
   * ||| ||  ++--------> URXISEL: RX Interrupt on every incoming byte.
   * ||| |+------------> UTXEN:   Transmission disabled.
   * ||| +-------------> UTXBRK:  Sync break transmission disabled.
   * +-+---------------> UTXISEL: TX Interrupt on every outgoing byte.
   *  +----------------> UTXINV:  TX Idle state is '1'.
   */
  U2STA = 0;

  /* Set polarity inversion on TX pin. */
  U2STAbits.UTXINV = invert_polarity;
}

void uart2_enable(void) {
  /* Enable UART port. */
  U2MODEbits.UARTEN = ON;

  /* Enable data transmission. */
  U2STAbits.UTXEN = ON;

  /* Clear UART2 interrupt flag. */
  IFS1bits.U2RXIF = OFF;

  /*
   * Tristate bits are ignored by the PPS peripheral, and are set here only to
   * allow the 'v' command to report the right direction.
   */
  BP_MOSI_DIR = OUTPUT;
}

void uart2_disable(void) {
  /* Disable UART port. */
  U2MODEbits.UARTEN = OFF;

  /* Enable data transmission. */
  U2STAbits.UTXEN = OFF;

  /* Map pin RB7 as UART2 RX. */
  RPINR19bits.U2RXR = 0b11111;

  /* Remove PPS assignment. */
  UARTTX_PIN = OFF;
  UARTTX_ODC = OFF;

  /*
   * Tristate bits are ignored by the PPS peripheral, and are set here only to
   * allow the 'v' command to report the right direction.
   */
  BP_MOSI_DIR = INPUT;
}

void uart2_tx(const uint8_t character) {
  /* Wait until the TX buffer is empty. */
  while (U2STAbits.UTXBF == ON) {
  }

  U2TXREG = character;
}

uint8_t uart2_rx(void) {
  /* Wait until the RX buffer is full. */
  while (U2STAbits.URXDA == OFF) {
  }

  return (uint8_t)(U2RXREG & 0xFF);
}

bool uart2_rx_ready(void) { return U2STAbits.URXDA; }
