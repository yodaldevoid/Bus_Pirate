#include "base.h"

extern mode_configuration_t
    mode_configuration; // holds persistant bus pirate settings (see base.h)

void bp_binary_io_peripherals_set(unsigned char inByte) {
  if (inByte & 0b1000) {
    BP_VREG_ON(); // power on
  } else {
    BP_VREG_OFF(); // power off
  }

  if (inByte & 0b100) {
    BP_PULLUP_ON(); // pullups on
  } else {
    BP_PULLUP_OFF();
  }

  // AUX pin, high/low only
  if (inByte & 0b10) {
    BP_AUX0_DIR = 0; // aux output
    BP_AUX0 = 1;     // aux high
  } else {
    BP_AUX0_DIR = 0; // aux output
    BP_AUX0 = 0;     // aux low
  }

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

  // UART1TX(1);//send 1/OK
}

#ifdef BUSPIRATEV4

// checks if voltage is present on VUEXTERN
bool bp_binary_io_pullup_control(uint8_t control_byte) {
  bool result;

  result = true;
  if (mode_configuration.high_impedance == false) {
    result = false;
  } else {
    /* Disable both pull-ups. */
    BP_3V3PU_OFF();
    bp_delay_ms(2);

    /* Turn on the ADC. */
    ADCON();
    if (bp_read_adc(BP_ADC_VPU) > 0x100) {
      /* Is there already an external voltage? */
      result = false;
    }
    /* Turn off the ADC. */
    ADCOFF();
  }

  if (result) {
    switch (control_byte) {
    case 0x51:
      /* Turn on the +3.3v pull-up. */
      BP_3V3PU_ON();
      break;

    case 0x52:
      /* Turn on the +5v pull-up. */
      BP_5VPU_ON();
      break;

    default:
      /* Turn off both pull-ups. */
      BP_3V3PU_OFF();
      break;
    }
  }

  return result;
}

#endif /* BUSPIRATEV4 */
