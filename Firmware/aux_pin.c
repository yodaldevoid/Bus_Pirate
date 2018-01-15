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

/**
 * @file aux_pin.c
 *
 * @brief AUX pins handler implementation file.
 */
#include <stdint.h>

#include "aux_pin.h"
#include "base.h"
#include "proc_menu.h"

// TRISDbits.TRISD5
#define AUXPIN_DIR BP_AUX0_DIR

// 20
#define AUXPIN_RPIN BP_AUX_RPIN

// RPOR10bits.RP20R
#define AUXPIN_RPOUT BP_AUX_RPOUT

extern mode_configuration_t mode_configuration;
extern bool command_error;

/**
 * @brief Possible modes for the AUX pins.
 */
typedef enum {
  /** The AUX pin is set in I/O mode. */
  AUX_MODE_IO = 0,
  /** The AUX pin is set in Frequency Counting mode. */
  AUX_MODE_FREQUENCY,
  /** The AUX pin is set in PWM Signal Generation mode. */
  AUX_MODE_PWM
} __attribute__((packed)) aux_mode_t;

/**
 * @brief Reads the AUX signal for one second, returning the detected frequency.
 *
 * @return the detected frequency on the AUX pin in the second spent sampling,
 * in Hz.
 */
static uint32_t poll_frequency_counter_value(void);

unsigned long bpPeriod_count(unsigned int n);

/**
 * @brief AUX pins manager internal state variables container.
 */
typedef struct {
  /** The PWM frequency in use. */
  uint16_t pwm_frequency;
  /** The PWM duty cycle in use. */
  uint16_t pwm_duty_cycle;
  /** The AUX pin mode. */
  aux_mode_t mode;
} __attribute__((packed)) aux_state_t;

/**
 * @brief AUX pins manager state.
 */
static aux_state_t state = {0};

/**
 * @brief Sets up input clock prescaler and returns an appropriate divisor.
 *
 * Sets up timer #1's input clock prescaler for the given frequency and returns
 * an appropriate divisor for it.
 *
 * @param[in] frequency the given frequency to set things up for.
 *
 * @return the appropriate PWM frequency divisor.
 */
static uint16_t setup_prescaler_divisor(const uint16_t frequency);

/**
 * @brief PWM frequency divisor for 1:256 prescaler.
 */
#define PWM_DIVISOR_PRESCALER_1_256 62

/**
 * @brief PWM frequency divisor for 1:64 prescaler.
 */
#define PWM_DIVISOR_PRESCALER_1_64 250

/**
 * @brief PWM frequency divisor for 1:8 prescaler.
 */
#define PWM_DIVISOR_PRESCALER_1_8 2000

/**
 * @brief PWM frequency divisor for 1:1 prescaler.
 */
#define PWM_DIVISOR_PRESCALER_1_1 16000

uint16_t setup_prescaler_divisor(const uint16_t frequency) {

  /* Use 1:256 prescaler. */

  if (frequency < 4) {
    T2CONbits.TCKPS1 = ON;
    T2CONbits.TCKPS0 = ON;

    return PWM_DIVISOR_PRESCALER_1_256;
  }

  /* Use 1:64 prescaler. */

  if (frequency < 31) {
    T2CONbits.TCKPS1 = ON;
    T2CONbits.TCKPS0 = OFF;

    return PWM_DIVISOR_PRESCALER_1_64;
  }

  /* Use 1:8 prescaler. */

  if (frequency < 245) {
    T2CONbits.TCKPS1 = OFF;
    T2CONbits.TCKPS0 = ON;

    return PWM_DIVISOR_PRESCALER_1_8;
  }

  /* Use 1:1 prescaler. */

  T2CONbits.TCKPS1 = OFF;
  T2CONbits.TCKPS0 = OFF;

  return PWM_DIVISOR_PRESCALER_1_1;
}

inline void bp_update_duty_cycle(const uint16_t duty_cycle) {
  bp_update_pwm(state.pwm_frequency, duty_cycle);
}

void bp_update_pwm(const uint16_t frequency, const uint16_t duty_cycle) {
  uint16_t period;
  uint16_t cycle;
  uint16_t divisor;

  state.pwm_frequency = frequency;
  state.pwm_duty_cycle = duty_cycle;

  /* Shut timers down. */
  T2CON = 0;
  T4CON = 0;
  OC5CON = 0;

  /* Detach the AUX pin from the PWM generator if no PWM signal is needed. */
  if (frequency == 0) {
    AUXPIN_RPOUT = 0;
    state.mode = AUX_MODE_IO;
    return;
  }

  divisor = setup_prescaler_divisor(frequency);
  period = (divisor / frequency) - 1;
  PR2 = period;
  cycle = (period * duty_cycle) / 100;

  /* Attach the AUX pin to the PWM generator. */
  AUXPIN_RPOUT = OC5_IO;

  /* Setup the PWM generator. */
  OC5R = cycle;
  OC5RS = cycle;
  OC5CON = 0x06;
  T2CONbits.TON = ON;
  state.mode = AUX_MODE_PWM;
}

// setup the PWM/frequency generator
void bp_pwm_setup(void) {
  unsigned int PWM_period, PWM_dutycycle, PWM_freq, PWM_div;
  int done;
  float PWM_pd;

  // cleanup timers
  T2CON = 0; // clear settings
  T4CON = 0;
  OC5CON = 0;

  if (state.mode == AUX_MODE_PWM) { // PWM is on, stop it
    AUXPIN_RPOUT = 0;               // remove output from AUX pin
    // bpWline(OUMSG_AUX_PWM_OFF);
    BPMSG1028;
    state.mode = AUX_MODE_IO;

    if (cmdbuf[((cmdstart + 1) & CMDLENMSK)] == 0x00) {
      // return if no arguments to function
      return;
    }
  }

  done = 0;

  cmdstart = (cmdstart + 1) & CMDLENMSK;

  // get any compound commandline variables
  consumewhitechars();
  PWM_freq = getint();
  consumewhitechars();
  PWM_pd = getint();

  // sanity check values
  if ((PWM_freq > 0) && (PWM_freq < 4000))
    done++;
  if ((PWM_pd > 0) && (PWM_pd < 100))
    done++;

  // calculate frequency:
  // no command line variables, prompt for PWM frequency
  if (done != 2) {
    command_error = false;
    BPMSG1029;
    BPMSG1030;
    PWM_freq = getnumber(50, 1, 4000, 0);
  }

  // choose proper multiplier for whole range
  PWM_div = setup_prescaler_divisor(PWM_freq);
  PWM_period = (PWM_div / PWM_freq) - 1;

  // if no commandline vairable, prompt for duty cycle
  if (done != 2) {
    BPMSG1033;
    PWM_pd = getnumber(50, 0, 99, 0);
  }

  PWM_pd /= 100;
  PWM_dutycycle = PWM_period * PWM_pd;
  // bpWdec(PWM_dutycycle);

  // assign pin with PPS
  AUXPIN_RPOUT = OC5_IO;
  // should be fine on bpv4

  OC5R = PWM_dutycycle;
  OC5RS = PWM_dutycycle;
  OC5CON = 0x6;
  PR2 = PWM_period;
  T2CONbits.TON = ON;

  BPMSG1034;
  state.mode = AUX_MODE_PWM;
}

// frequency measurement
void bp_frequency_counter_setup(void) {
  // frequency accuracy optimized by selecting measurement method, either
  //   counting frequency or measuring period, to maximize resolution.
  // Note: long long int division routine used by C30 is not open-coded  */
  unsigned long long f, p;

  if (state.mode == AUX_MODE_PWM) {
    // bpWline(OUMSG_AUX_FREQ_PWM);
    BPMSG1037;
    return;
  }

  // bpWstring(OUMSG_AUX_FREQCOUNT);
  BPMSG1038;
  // setup timer
  T4CON = 0; // make sure the counters are off
  T2CON = 0;

  // timer 2 external
  AUXPIN_DIR = 1; // aux input

  RPINR3bits.T2CKR = AUXPIN_RPIN; // assign T2 clock input to aux input
  // should be good on bpv4

  T2CON = 0b111010; //(TCKPS1|TCKPS0|T32|TCS); // prescale to 256

  f = poll_frequency_counter_value(); // all measurements within 26bits (<67MHz)

  // counter only seems to be good til around 6.7MHz,
  // use 4.2MHz (nearest power of 2 without exceeding 6.7MHz) for reliable
  // reading
  if (f > 0x3fff) { // if >4.2MHz prescaler required
    f *= 256;       // adjust for prescaler
  } else {          // get a more accurate reading without prescaler
    // bpWline("Autorange");
    BPMSG1245;
    T2CON = 0b001010; //(TCKPS1|TCKPS0|T32|TCS); prescale to 0
    f = poll_frequency_counter_value();
  }
  // at 4000Hz 1 bit resolution of frequency measurement = 1 bit resolution of
  // period measurement
  if (f >
      3999) { // when < 4 KHz  counting edges is inferior to measuring period(s)
    bp_write_dec_dword_friendly(
        f); // this function uses comma's to seperate thousands.
    MSG_PWM_HZ_MARKER;
  } else if (f > 0) {
    BPMSG1245;
    p = bpPeriod_count(f);
    // don't output fractions of frequency that are less then the frequency
    //   resolution provided by an increment of the period timer count.
    if (p > 400000) { // f <= 40 Hz
      // 4e5 < p <= 1,264,911 (625us tics)
      // 12.61911 < f <= 40 Hz
      // output resolution of 1e-5
      f = 16e11 / p;
      bp_write_dec_dword_friendly(f / 100000);
      UART1TX('.');
      f = f % 100000;
      if (f < 10000)
        UART1TX('0');
      if (f < 1000)
        UART1TX('0');
      if (f < 100)
        UART1TX('0');
      if (f < 10)
        UART1TX('0');
      bp_write_dec_dword(f);
      // at p=126,491.1 frequency resolution is .001
    } else if (p > 126491) { // f <= 126.4911
      // 126,491 < p <= 4e5  (625us tics)
      // 40 < f <= 126.4911 Hz
      // output resolution of .0001
      f = 16e10 / p;
      bp_write_dec_dword_friendly(f / 10000);
      UART1TX('.');
      f = f % 10000;
      if (f < 1000)
        UART1TX('0');
      if (f < 100)
        UART1TX('0');
      if (f < 10)
        UART1TX('0');
      bp_write_dec_word(f);
      // at p=40,000 frequency resolution is .01
    } else if (p > 40000) { // f <= 400 Hz
      // 4e4 < p <= 126,491 (625us tics)
      // 126.4911 < f <= 400 Hz
      // output resolution of .001
      f = 16e9 / p;
      bp_write_dec_dword_friendly(f / 1000);
      UART1TX('.');
      f = f % 1000; // frequency resolution < 1e-2
      if (f < 100)
        UART1TX('0');
      if (f < 10)
        UART1TX('0');
      bp_write_dec_word(f);
      // at p=12,649.11 frequency resolution is .1
    } else if (p > 12649) { // f <= 1264.911
      // 12,649 < p <= 4e4  (625us tics)
      // 400 < f < 1,264.911 Hz
      // output resolution of .01
      f = 16e8 / p;
      bp_write_dec_dword_friendly(f / 100);
      UART1TX('.');
      f = f % 100; // frequency resolution < 1e-1
      if (f < 10)
        UART1TX('0');
      bp_write_dec_byte(f);
      // at p=4,000 frequency resolution is 1
    } else { // 4,000 < p <= 12,649 (625us tics)
      // 1,264.911 < f < 4,000 Hz
      // output resolution of .1
      f = 16e7 / p;
      bp_write_dec_dword_friendly(f / 10);
      UART1TX('.');
      f = f % 10; // frequency resolution < 1
      bp_write_dec_byte(f);
    }
    MSG_PWM_HZ_MARKER;
    // END of IF(f>0)
  } else {
    MSG_PWM_FREQUENCY_TOO_LOW;
  }

  // return clock input to other pin
  RPINR3bits.T2CKR = 0b11111; // assign T2 clock input to nothing
  T4CON = 0;                  // make sure the counters are off
  T2CON = 0;
}

unsigned long bp_measure_frequency(void) {
  // static unsigned int j,k;
  unsigned long l;

  // setup timer
  T4CON = 0; // make sure the counters are off
  T2CON = 0;

  // timer 2 external
  AUXPIN_DIR = 1;                 // aux input
  RPINR3bits.T2CKR = AUXPIN_RPIN; // assign T2 clock input to aux input

  T2CON = 0b111010; //(TCKPS1|TCKPS0|T32|TCS);

  l = poll_frequency_counter_value();
  if (l > 0xff) {     // got count
    l *= 256;         // adjust for prescaler...
  } else {            // no count, maybe it's less than prescaler (256hz)
    T2CON = 0b001010; //(TCKPS1|TCKPS0|T32|TCS); prescale to 0
    l = poll_frequency_counter_value();
  }

  // return clock input to other pin
  RPINR3bits.T2CKR = 0b11111; // assign T2 clock input to nothing
  T4CON = 0;                  // make sure the counters are off
  T2CON = 0;
  return l;
}

uint32_t poll_frequency_counter_value(void) {
  uint32_t counter_low;
  uint32_t counter_high;

  /* Set 32-bits period register for timer #2 (0xFFFFFFFF). */
  PR3 = 0xFFFF;
  PR2 = 0xFFFF;

  /* Clear timer #2 counter. */

  TMR3HLD = 0;
  TMR2 = 0;

  /* Clear timer #4 counter. */

  TMR5HLD = 0;
  TMR4 = 0;

  /* Set timer #4 as 32 bits. */
  T4CONbits.T32 = YES;

  /* Set 32-bits period register for timer #4 (0x00F42400, one second). */

  PR5 = 0x00F4;
  PR4 = 0x2400;

  /* Clear timer #4 interrupt flag (32 bits mode). */
  IFS1bits.T5IF = OFF;

  /* Start timer #4. */
  T4CONbits.TON = ON;

  /* Start timer #2. */
  T2CONbits.TON = ON;

  /* Wait for timer #4 interrupt to occur. */
  while (IFS1bits.T5IF == 0) {
  }

  /* Stop timers. */

  T2CONbits.TON = OFF;
  T4CONbits.TON = OFF;

  /* Timer #2 now contains the frequency value. */

  counter_low = TMR2;
  counter_high = TMR3HLD;

  return (counter_high << 16) + counter_low;
}

// bpPeriod_count function for frequency measurment uses input compare periphers
// because BP v4 and v3 have different IC peripherals the function is
// implemented through #if defs
#if defined(BUSPIRATEV4)
// BPv4 implementation of the bpPeriod_count function
unsigned long bpPeriod_count(unsigned int n) {
  static unsigned int i;
  static unsigned long j, k, l, m, d, s;

  IFS0bits.IC2IF = 0; // clear input capture interrupt flag
  IFS0bits.IC1IF = 0; // clear input capture interrupt flag

  // configure IC1 to RP20 (AUX)
  RPINR7bits.IC2R = AUXPIN_RPIN;
  RPINR7bits.IC1R = AUXPIN_RPIN;

  // timer 4 internal, measures interval
  TMR5HLD = 0x00;
  TMR4 = 0x00;
  T4CON = 0b1000; //.T32=1, bit 3
  // start timer4
  T4CONbits.TON = 1;

  // unimplemented: [15:14]=0b00,
  // ICSIDL:        [13]=0b0, input capture module continues to operate in CPU
  // idle mode
  // ICTSEL[2:0]:   [12:10]=0b010=TMR4, 0b011=TMR5 (unimplemented for 16-bit
  // capture)
  // unimplemented: [9:8]=0b00
  // ICTMR:         [7]=0b0=TMR3, 0b1=TMR2 (unimplemented for 32-bit capture)
  // ICI[1:0]:      [6:5]=0b00, 1 capture per interrupt
  // ICOV,ICBNE:    [4:3]=0b00, read-only buffer overflow and not empty
  // ICM[2:0]:      [2:0]=0b011, capture every rising edge
  IC2CON1 = 0x0C03; // fails with ICM 0 or 3 (0 always read from IC2BUF)

  IC1CON1 = 0x0803;

  // unimplemented: [15:9]=0b0000000
  // IC32:          [8]=0b0
  // ICTRIG:        [7]=0b0, synchronize with SYNCSEL specified source
  // TRIGSTAT:      [6]=0b0, cleared by SW, holds timer in reset when low,
  // trigger chosen by syncsel sets bit and releases timer from reset.
  // unimplemented: [5]=0b0
  // SYNCSEL[4:0]:  [4:0]=0b10100, selects trigger/synchronization source to be
  // IC1.

  IC2CON2 = 0x0014;

  IC1CON2 = 0x0014;

  // read input capture bits n times
  while (IC1CON1bits.ICBNE) // clear buffer
    j = IC1BUF;

  while (IC2CON1bits.ICBNE) // clear buffer
    k = IC2BUF;

  while (!IC1CON1bits.ICBNE)
    ; // wait for ICBNE

  k = IC1BUF;
  m = IC2BUF;
  for (i = s = 0; i < n; i++) {
    while (!IC1CON1bits.ICBNE)
      ; // wait for ICBNE
    j = IC1BUF;
    l = IC2BUF;
    d = ((l - m) << 16) + (j - k);
    s = s + d;
    m = l;
    k = j;
  }

  // turn off input capture modules, reset control to POR state
  IC1CON1 = 0;
  IC1CON2 = 0;
  T4CONbits.TON = 0;

  return s / n;
}
#elif defined(BUSPIRATEV3)
// BPv3(v2) implementation of the bpPeriod_count function
unsigned long bpPeriod_count(unsigned int n) {
  static unsigned int i;
  static unsigned long j, k, l, m, d, s;

  IFS0bits.IC2IF = 0; // clear input capture interrupt flag
  IFS0bits.IC1IF = 0; // clear input capture interrupt flag

  // configure IC1 to RP20 (AUX)
  RPINR7bits.IC2R = AUXPIN_RPIN;
  RPINR7bits.IC1R = AUXPIN_RPIN;

  // timer 4 internal, measures interval
  TMR3HLD = 0x00;
  TMR2 = 0x00;
  T2CON = 0b1000; //.T32=1, bit 3
  // start timer4
  T2CONbits.TON = 1;

  // bit7 determins tmr2/3,
  // bits 0:2 determine IC mode, 3 is Simple capture mode, capture on every
  // rising edge
  IC2CON = 0x0003; // bit7 i 0 - connected to tmr3

  IC1CON = 0x0083; // bit7 is 1 - connected to tmr2,

  // read input capture bits n times
  while (IC1CONbits.ICBNE) // clear buffer
    j = IC1BUF;

  while (IC2CONbits.ICBNE) // clear buffer
    k = IC2BUF;

  while (!IC1CONbits.ICBNE)
    ; // wait for ICBNE

  k = IC1BUF;
  m = IC2BUF;
  for (i = s = 0; i < n; i++) {
    while (!IC1CONbits.ICBNE)
      ; // wait for ICBNE
    j = IC1BUF;
    l = IC2BUF;
    d = ((l - m) << 16) + (j - k);
    s = s + d;
    m = l;
    k = j;
  }

  // turn off input capture modules, reset control to POR state
  IC1CON = 0;
  IC1CON = 0;
  T2CONbits.TON = 0;

  return s / n;
}
#endif

void bp_aux_pin_set_high_impedance(void) {
#ifdef BUSPIRATEV3
  if (mode_configuration.alternate_aux == 0) {
    BP_AUX0_DIR = INPUT;
  } else {
    BP_CS_DIR = INPUT;
  }
#else
  switch (mode_configuration.alternate_aux) {
  case 0:
    BP_AUX0_DIR = INPUT;
    break;

  case 1:
    BP_CS_DIR = INPUT;
    break;

  case 2:
    BP_AUX1_DIR = INPUT;
    break;

  case 3:
    BP_AUX2_DIR = INPUT;
    break;

  default:
    break;
  }
#endif /* BUSPIRATEV3 */

  BPMSG1039;
}

/* Leaves the selected AUX pin HIGH. */
void bp_aux_pin_set_high(void) {
#ifdef BUSPIRATEV3
  if (mode_configuration.alternate_aux == 0) {
    BP_AUX0_DIR = OUTPUT;
    BP_AUX0 = HIGH;
  } else {
    BP_CS_DIR = OUTPUT;
    BP_CS = HIGH;
  }
#else
  switch (mode_configuration.alternate_aux) {
  case 0:
    BP_AUX0_DIR = OUTPUT;
    BP_AUX0 = HIGH;
    break;

  case 1:
    BP_CS_DIR = OUTPUT;
    BP_CS = HIGH;
    break;

  case 2:
    BP_AUX1_DIR = OUTPUT;
    BP_AUX1 = HIGH;
    break;

  case 3:
    BP_AUX2_DIR = OUTPUT;
    BP_AUX2 = HIGH;
    break;

  default:
    break;
  }
#endif /* BUSPIRATEV3 */

  BPMSG1040;
}

/* Leaves the selected AUX pin LOW. */
void bp_aux_pin_set_low(void) {
#ifdef BUSPIRATEV3
  if (mode_configuration.alternate_aux == 0) {
    BP_AUX0_DIR = OUTPUT;
    BP_AUX0 = LOW;
  } else {
    BP_CS_DIR = OUTPUT;
    BP_CS = LOW;
  }
#else
  switch (mode_configuration.alternate_aux) {
  case 0:
    BP_AUX0_DIR = OUTPUT;
    BP_AUX0 = LOW;
    break;

  case 1:
    BP_CS_DIR = OUTPUT;
    BP_CS = LOW;
    break;

  case 2:
    BP_AUX1_DIR = OUTPUT;
    BP_AUX1 = LOW;
    break;

  case 3:
    BP_AUX2_DIR = OUTPUT;
    BP_AUX2 = LOW;
    break;

  default:
    break;
  }
#endif /* BUSPIRATEV3 */

  BPMSG1041;
}

/* Leaves the selected AUX pin into INPUT/HiZ mode. */
bool bp_aux_pin_read(void) {
#ifdef BUSPIRATEV3
  if (mode_configuration.alternate_aux == 0) {
    BP_AUX0_DIR = INPUT;
    Nop();
    Nop();
    return BP_AUX0;
  }

  BP_CS_DIR = INPUT;
  Nop();
  Nop();
  return BP_CS;
#else
  switch (mode_configuration.alternate_aux & 0b00000011) {
  case 0:
    BP_AUX0_DIR = INPUT;
    Nop();
    Nop();
    return BP_AUX0;

  case 1:
    BP_CS_DIR = INPUT;
    Nop();
    Nop();
    return BP_CS;

  case 2:
    BP_AUX1_DIR = INPUT;
    Nop();
    Nop();
    return BP_AUX1;

  case 3:
    BP_AUX2_DIR = INPUT;
    Nop();
    Nop();
    return BP_AUX2;

  default:
    /* Should not happen. */
    return LOW;
  }
#endif /* BUSPIRATEV3 */
}

void bp_servo_setup(void) {
  unsigned int PWM_period, PWM_dutycycle;
  unsigned char entryloop = 0;
  float PWM_pd;

  // Clear timers
  T2CON = 0; // clear settings
  T4CON = 0;
  OC5CON = 0;

  if (state.mode == AUX_MODE_PWM) { // PWM is on, stop it
    if (cmdbuf[((cmdstart + 1) & CMDLENMSK)] ==
        0x00) {         // no extra data, stop servo
      AUXPIN_RPOUT = 0; // remove output from AUX pin
      BPMSG1028;
      state.mode = AUX_MODE_IO;
      return; // return if no arguments to function
    }
  }

  cmdstart = (cmdstart + 1) & CMDLENMSK;

  // Get servo position from command line or prompt for value
  consumewhitechars();
  PWM_pd = getint();
  if (command_error || (PWM_pd > 180)) {
    command_error = false;
    BPMSG1254;
    PWM_pd = getnumber(90, 0, 180, 0);
    entryloop = 1;
  }

// Setup multiplier for 50 Hz
servoset:
  T2CONbits.TCKPS1 = 1;
  T2CONbits.TCKPS0 = 1;
  PWM_period = 1250;
  PWM_pd /= 3500;
  PWM_dutycycle = (PWM_period * PWM_pd) + 62;

  // assign pin with PPS
  AUXPIN_RPOUT = OC5_IO;
  OC5R = PWM_dutycycle;
  OC5RS = PWM_dutycycle;
  OC5CON = 0x6;
  PR2 = PWM_period;
  T2CONbits.TON = ON;
  BPMSG1255;
  state.mode = AUX_MODE_PWM;

  if (entryloop == 1) {
    PWM_pd = getnumber(-1, 0, 180, 1);
    if (PWM_pd < 0) {
      bpBR;
      return;
    }
    goto servoset;
  }
}

/*1. Set the PWM period by writing to the selected
Timer Period register (PRy).
2. Set the PWM duty cycle by writing to the OCxRS
register.
3. Write the OCxR register with the initial duty cycle.
4. Enable interrupts, if required, for the timer and
output compare modules. The output compare
interrupt is required for PWM Fault pin utilization.
5. Configure the output compare module for one of
two PWM Operation modes by writing to the
Output Compare Mode bits, OCM<2:0>
(OCxCON<2:0>).
6. Set the TMRy prescale value and enable the time
base by setting TON (TxCON<15>) = 1.*/
