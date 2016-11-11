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

#include "base.h"
#include "core.h"

extern bus_pirate_configuration_t bus_pirate_configuration;
extern mode_configuration_t mode_configuration;

#ifdef BUSPIRATEV3
// Internal FRC OSC = 8MHz
#pragma config FNOSC     = FRCPLL
#pragma config OSCIOFNC  = ON 
#pragma config POSCMOD   = NONE
#pragma config I2C1SEL   = PRI
#pragma config JTAGEN    = OFF
#pragma config GCP       = OFF
#pragma config GWRP      = OFF
#pragma config COE       = OFF
#pragma config FWDTEN    = OFF
#pragma config ICS       = PGx1
#endif /* BUSPIRATEV3 */

#ifdef BUSPIRATEV4
#pragma config JTAGEN    = OFF
#pragma config GCP       = OFF
#pragma config GWRP      = OFF
#pragma config COE       = OFF
#pragma config FWDTEN    = OFF
#pragma config ICS       = PGx2
#pragma config IESO      = OFF
#pragma config FCKSM     = CSDCMD
#pragma config OSCIOFNC  = ON
#pragma config POSCMOD   = HS
#pragma config FNOSC     = PRIPLL
#pragma config PLLDIV    = DIV3
#pragma config IOL1WAY   = ON
#pragma config PLL_96MHZ = ON
#pragma config DISUVREG  = OFF
#endif /* BUSPIRATEV4 */

/**
 * Clear mode configuration on mode change.
 */
static void clear_mode_configuration(void);

void clear_mode_configuration(void) {
    mode_configuration.high_impedance = 0;
    mode_configuration.speed = 0; 
    mode_configuration.periodicService = 0;
    mode_configuration.altAUX = 0;
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

    BP_AUX_RPOUT = 0;    //remove output from AUX pin (PWM/servo modes)

    bus_pirate_configuration.bus_mode = BP_HIZ;
    clear_mode_configuration();
    BP_PULLUP_OFF();
    BP_VREG_OFF();
    //setup voltage monitoring on ADC. see hardwarevx.h!
    BP_ADC_PINSETUP();

    //configure the ADC
    AD1CON1bits.SSRC  = 0b111;// SSRC<3:0> = 111 implies internal
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
    while (AD1CON1bits.DONE == OFF);

    /* Return value. */
    return ADC1BUF0;
}

void bp_adc_probe(void) {
    /* Turn the ADC on. */
    AD1CON1bits.ADON = ON;
    
    /* Perform the measurement. */
    bpWvolts(bp_read_adc(BP_ADC_PROBE));
    
    /* Turn the ADC off. */
    AD1CON1bits.ADON = OFF;
}

void bp_adc_continuous_probe(void) {
    unsigned int measurement;

    //bpWline(OUMSG_PS_ADCC);
    BPMSG1042;
    //bpWline(OUMSG_PS_ANY_KEY);
    BPMSG1250;
    //bpWstring(OUMSG_PS_ADC_VOLT_PROBE);
    BPMSG1044;
    bpWvolts(0);
    //bpWstring(OUMSG_PS_ADC_VOLTS);
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
        bpWvolts(measurement);
        
        //bpWstring(OUMSG_PS_ADC_VOLTS);
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
                bpWinthex(value);
            } else {
                bpWhex(value);
            }
            break;
            
        case DEC:
            if (mode_configuration.int16) {
                bpWintdec(value);
            } else {
                bpWdec(value);
            }
            break;
            
        case BIN:
            if (mode_configuration.int16) {
                bpWbin(value);
                bpSP;
            }
            bpWbin(value);
            break;
            
        case RAW:
            if (mode_configuration.int16) {
                UART1TX(value >> 8);
            }
            UART1TX(value & 0xFF);
            break;
    }
}

void bp_delay_ms(unsigned int milliseconds) {
    unsigned int counter;
    
    for (counter = 0; counter < milliseconds; counter++) {
        bp_delay_us(250);
        bp_delay_us(250);
        bp_delay_us(250);
        bp_delay_us(250);
    }
}

void bp_delay_us(unsigned int microseconds) {
    unsigned int counter;
    
    /* When running at 32MHz, it can execute 16 instructions per uS */
    for (counter = 0; counter < microseconds; counter++) {
        Nop();
        Nop();
        Nop();
        Nop();
        Nop();
        Nop();
        Nop();
        Nop();
        Nop();
        Nop();
        Nop();
        Nop();
        Nop();
        Nop();
        Nop();
        Nop();
    }
}

unsigned int bp_reverse_integer(unsigned int value) {
    unsigned int reversed = 0, bitmask;

    for (bitmask = 0b00000001; bitmask != 0; bitmask = bitmask << 1) {
        reversed = reversed << 1;    
        if (value & bitmask) {
            reversed |= 0b00000001;
        }
    }

    if (mode_configuration.numbits != 16) {
        reversed >>= 16 - mode_configuration.numbits;
    }
    
    return reversed;
}


