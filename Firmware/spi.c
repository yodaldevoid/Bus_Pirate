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

#include "spi.h"

#ifdef BP_ENABLE_SPI_SUPPORT

#include "base.h"
#include "core.h"
#include "binary_io.h"

#include "proc_menu.h"

#define SPIMOSI_TRIS 	BP_MOSI_DIR	
#define SPICLK_TRIS 	BP_CLK_DIR	
#define SPIMISO_TRIS 	BP_MISO_DIR	
#define SPICS_TRIS 		BP_CS_DIR	

#define SPIMOSI 		BP_MOSI
#define SPICLK 			BP_CLK	
#define SPIMISO 		BP_MISO	
#define SPICS 			BP_CS	

#define SPIMOSI_ODC 		BP_MOSI_ODC	
#define SPICLK_ODC 			BP_CLK_ODC	
#define SPICS_ODC 			BP_CS_ODC	

#define SPICS_RPIN		BP_CS_RPIN

extern mode_configuration_t mode_configuration;
extern command_t last_command;
extern bus_pirate_configuration_t bus_pirate_configuration;
extern bool command_error;

/**
 * Set up the SPI interfaces to operate in slave mode.
 */
static void spi_slave_enable(void);

/**
 * Gets the SPI interfaces out of slave mode.
 */
static void spi_slave_disable(void);

static void spiSniffer(unsigned char csState, unsigned char termMode);

/**
 * Engages the CS line.
 * 
 * @param[in] write_with_read flag indicating if data writes will have a
 *                            subsequent read operation or not.
 */
static void engage_spi_cs(bool write_with_read);

#ifdef BP_SPI_ENABLE_AVR_EXTENDED_COMMANDS

/**
 * Extended AVR Binary I/O command for no operations.
 */
#define BINARY_IO_SPI_AVR_COMMAND_NOOP 0

/**
 * Extended AVR Binary I/O command for obtaining the protocol version.
 */
#define BINARY_IO_SPI_AVR_COMMAND_VERSION 1

/**
 * Extended AVR Binary I/O command for performing a bulk read.
 */
#define BINARY_IO_SPI_AVR_COMMAND_BULK_READ 2

/**
 * Extended AVR Binary I/O protocol version.
 */
#define BINARY_IO_SPI_AVR_SUPPORT_VERSION 0x0001

#define AVR_FETCH_LOW_BYTE_COMMAND 0x20
#define AVR_FETCH_HIGH_BYTE_COMMAND 0x28

/**
 * Handle an incoming extended binary I/O AVR SPI command.
 */
static void handle_extended_avr_command(void);

#endif /* BP_SPI_ENABLE_AVR_EXTENDED_COMMANDS */

/**
 * SPI bit read happens in the middle of a bit write.
 */
#define SPI_SAMPLING_ON_DATA_OUTPUT_MIDDLE 0

/**
 * SPI bit read happens after a bit write.
 */
#define SPI_SAMPLING_ON_DATA_OUTPUT_END 1

/**
 * SPI bus clock line will idle when low.
 */
#define SPI_CLOCK_IDLE_LOW 0

/**
 * SPI bus clock line will idle when high.
 */
#define SPI_CLOCK_IDLE_HIGH 1

/**
 * SPI data reads and writes happen when the clock goes from idle to active.
 */
#define SPI_TRANSITION_FROM_IDLE_TO_ACTIVE 0

/**
 * SPI data reads and writes happen when the clock goes from active to idle.
 */
#define SPI_TRANSITION_FROM_ACTIVE_TO_IDLE 1

/**
 * SPI protocol state structure.
 */
typedef struct {
    
    /**
     * Clock polarity.
     * 
     * @see SPI_CLOCK_IDLE_LOW
     * @see SPI_CLOCK_IDLE_HIGH
     */
    uint8_t clock_polarity : 1;
    
    /**
     * Clock edge.
     * 
     * @see SPI_TRANSITION_FROM_IDLE_TO_ACTIVE
     * @see SPI_TRANSITION_FROM_ACTIVE_TO_IDLE
     */
    uint8_t clock_edge : 1;
    
    /**
     * Data sample timing.
     * 
     * @see SPI_SAMPLING_ON_DATA_OUTPUT_MIDDLE
     * @see SPI_SAMPLING_ON_DATA_OUTPUT_END
     */
    uint8_t data_sample_timing : 1;
    
    /** CS line state. */
    uint8_t cs_line_state : 1;
    
} spi_state_t; 

/**
 * The SPI protocol state.
 */
static spi_state_t spi_state = { 0 };

/**
 * Available SPI bus speeds.
 */
static const uint8_t spi_bus_speed[] = {
    0b00000000, /*  31 kHz - Primary prescaler 64:1 / Secondary prescaler 8:1 */
    0b00011000, /* 125 kHz - Primary prescaler 64:1 / Secondary prescaler 2:1 */
    0b00011100, /* 250 kHz - Primary prescaler 64:1 / Secondary prescaler 1:1 */
    0b00011101, /*   1 MHz - Primary prescaler 16:1 / Secondary prescaler 1:1 */
    0b00001100, /*  50 kHz - Primary prescaler 64:1 / Secondary prescaler 5:1 */
    0b00010110, /* 1.3 MHz - Primary prescaler  4:1 / Secondary prescaler 3:1 */
    0b00011010, /*   2 MHz - Primary prescaler  4:1 / Secondary prescaler 2:1 */
    0b00001011, /* 2.6 MHz - Primary prescaler  1:1 / Secondary prescaler 6:1 */
    0b00001111, /* 3.2 MHz - Primary prescaler  1:1 / Secondary prescaler 5:1 */
    0b00011110, /*   4 MHz - Primary prescaler  4:1 / Secondary prescaler 1:1 */
    0b00010111, /* 5.3 MHz - Primary prescaler  1:1 / Secondary prescaler 3:1 */
    0b00011011  /*   8 MHz - Primary prescaler  1:1 / Secondary prescaler 2:1 */
};

void engage_spi_cs(bool write_with_read) {
    mode_configuration.write_with_read = write_with_read;
    SPICS = !spi_state.cs_line_state;
    if (spi_state.cs_line_state) {
        UART1TX('/');
        MSG_SPI_CS_ENABLED;
    } else {
        MSG_SPI_CS_DISABLED;
    }
}

inline void SPIstartr(void) {
    engage_spi_cs(ON);
}

inline void SPIstart(void) {
    engage_spi_cs(OFF);
}

void SPIstop(void) {
    SPICS = spi_state.cs_line_state;
    if (spi_state.cs_line_state) {
        UART1TX('/');
        MSG_SPI_CS_ENABLED;
    } else {
        MSG_SPI_CS_DISABLED;
    }
}

inline unsigned int SPIread(void) {
    return spi_write_byte(0xFF);
}

unsigned int SPIwrite(unsigned int c) {
    unsigned char r;

    r = spi_write_byte(c);
    if (mode_configuration.write_with_read == 1) {
        return r;
    }
    //FIXME what to return if wwr=0? we need an uint here
    return 0; //JTR just to get rid of the warning msg
}

void SPIsettings(void) {
    MSG_SPI_MODE_HEADER_START;
    bp_write_dec_byte(mode_configuration.speed + 1);
    bpSP;
    bp_write_dec_byte(spi_state.clock_polarity);
    bpSP;
    bp_write_dec_byte(spi_state.clock_edge);
    bpSP;
    bp_write_dec_byte(spi_state.data_sample_timing);
    bpSP;
    bp_write_dec_byte(spi_state.cs_line_state);
    bpSP;
    bp_write_dec_byte(mode_configuration.high_impedance);
    MSG_MODE_HEADER_END;
}

void SPIsetup(void) {
    bool user_prompt;
    int spi_speed;
    int spi_clock_polarity;
    int spi_clock_edge;
    int spi_data_sampling;
    int spi_enabled;
    int spi_cs_line_state;

    consumewhitechars();
    spi_speed = getint();
    consumewhitechars();
    spi_clock_polarity = getint();
    consumewhitechars();
    spi_clock_edge = getint();
    consumewhitechars();
    spi_data_sampling = getint();
    consumewhitechars();
    spi_cs_line_state = getint();
    consumewhitechars();
    spi_enabled = getint();
    
    user_prompt = !(((spi_speed > 0) && (spi_speed <= 4)) &&
            ((spi_clock_polarity > 0) && (spi_clock_polarity <= 2)) &&
            ((spi_clock_edge > 0) && (spi_clock_edge <= 2)) &&
            ((spi_data_sampling > 0) && (spi_data_sampling <= 2)) &&
            ((spi_cs_line_state > 0) && (spi_cs_line_state <= 2)) &&
            ((spi_enabled > 0) && (spi_enabled <= 2)));
    
    if (user_prompt) {
        command_error = false;

        MSG_SPI_SPEED_PROMPT;
        mode_configuration.speed = getnumber(1, 1, 12, 0) - 1;

        MSG_SPI_POLARITY_PROMPT;
        spi_state.clock_polarity = getnumber(1, 1, 2, 0) - 1;

        MSG_SPI_EDGE_PROMPT;
        spi_state.clock_edge = getnumber(2, 1, 2, 0) - 1;

        MSG_SPI_SAMPLE_PROMPT;
        spi_state.data_sample_timing = getnumber(1, 1, 2, 0) - 1;

        MSG_SPI_CS_MODE_PROMPT;
        spi_state.cs_line_state = getnumber(2, 1, 2, 0) - 1;

        MSG_PIN_OUTPUT_TYPE_PROMPT;
        mode_configuration.high_impedance = ~(getnumber(1, 1, 2, 0) - 1);
    } else {
        mode_configuration.speed = spi_speed - 1;
        spi_state.clock_polarity = spi_clock_polarity - 1;
        spi_state.clock_edge = spi_clock_edge - 1;
        spi_state.data_sample_timing = spi_data_sampling - 1;        
        spi_state.cs_line_state = (spi_cs_line_state - 1);
        mode_configuration.high_impedance = ~(spi_enabled - 1);
        SPIsettings();
    }

    mode_configuration.write_with_read = OFF;
}

void SPIsetup_exc(void)
{
    //do SPI peripheral setup
    spi_setup(spi_bus_speed[mode_configuration.speed]);

    // set cs the way the user wants
    SPICS = spi_state.cs_line_state; 
} 

inline void SPIcleanup(void) {
    spiDisable();
}

void SPImacro(unsigned int macro) {

    switch (macro) {
        case 0:
            //bpWline(OUMSG_SPI_MACRO_MENU);
            BPMSG1192;
            break;
            
        case 1://sniff CS low
            MSG_SNIFFER_MESSAGE;
            MSG_ANY_KEY_TO_EXIT_PROMPT;
            spiSniffer(0, 1); //configure for terminal mode
            break;
            
        case 2://sniff all
            MSG_SNIFFER_MESSAGE;
            MSG_ANY_KEY_TO_EXIT_PROMPT;
            spiSniffer(1, 1); //configure for terminal mode
            break;
            
        case 3: //sniff CS high
            break;
            
        case 10:
            spi_state.clock_polarity = SPI_CLOCK_IDLE_LOW;
            goto SPImacro_settings_cleanup;
            
        case 11:
            spi_state.clock_polarity = SPI_CLOCK_IDLE_HIGH;
            goto SPImacro_settings_cleanup;
            
        case 12:
            spi_state.clock_edge = SPI_TRANSITION_FROM_IDLE_TO_ACTIVE;
            goto SPImacro_settings_cleanup;
            
        case 13:
            spi_state.clock_edge = SPI_TRANSITION_FROM_ACTIVE_TO_IDLE;
            goto SPImacro_settings_cleanup;
            
        case 14:
            spi_state.data_sample_timing = SPI_SAMPLING_ON_DATA_OUTPUT_MIDDLE;
            goto SPImacro_settings_cleanup;
            
        case 15:
            spi_state.data_sample_timing = SPI_SAMPLING_ON_DATA_OUTPUT_END;
            
SPImacro_settings_cleanup:
            SPI1CON1bits.CKP = spi_state.clock_polarity;
            SPI1CON1bits.CKE = spi_state.clock_edge;
            SPI1CON1bits.SMP = spi_state.data_sample_timing;
            SPIsettings();
            break;
            
        default:
            MSG_UNKNOWN_MACRO_ERROR;
            break;
    }
}

void SPIpins(void) {
    MSG_SPI_PINS_STATE;
}

void spi_setup(uint8_t spi_speed) {
    /* Disable interface. */
    SPI1STATbits.SPIEN = OFF;

    /* Set SPI to open drain if high impedance is on. */
    if (mode_configuration.high_impedance == ON) {
        SPIMOSI_ODC = ON;
        SPICLK_ODC = ON;
        SPICS_ODC = ON;
    } else {
        SPIMOSI_ODC = OFF;
        SPICLK_ODC = OFF;
        SPICS_ODC = OFF;
    }
    
    /* Assign pins routing. */

    RPINR20bits.SDI1R = BP_MISO_RPIN;
    BP_MOSI_RPOUT = SDO1_IO;
    BP_CLK_RPOUT = SCK1OUT_IO;
    SPICS = HIGH;
    SPICS_TRIS = OUTPUT;
    SPICLK_TRIS = OUTPUT;
    SPIMISO_TRIS = INPUT;
    SPIMOSI_TRIS = OUTPUT;
    
    /*
     * MSB
     * ---000xx0x1xxxxx
     *    |||||||||||||
     *    |||||||||||++---> PPRE:   Primary prescale bits.
     *    ||||||||+++-----> SPRE:   Secondary prescale bits.
     *    |||||||+--------> MSTEN:  Master mode.
     *    ||||||+---------> CKP:    Clock idle LOW.
     *    |||||+----------> SSEN:   Pin controlled by port function.
     *    ||||+-----------> CKE:    Transition happens from idle to active.
     *    |||+------------> SMP:    Flag indicating when the data is sampled.
     *    ||+-------------> MODE16: Communication is byte-wide.
     *    |+--------------> DISSDO: SDO1 pin is controlled by the module.
     *    +---------------> DISSCK: Internal SPI clock is enabled.
     */
    SPI1CON1 = (spi_speed & 0b11111) | (ON << _SPI1CON1_MSTEN_POSITION) |
            ((spi_state.clock_polarity & 0b1) << _SPI1CON1_CKP_POSITION) |
            ((spi_state.clock_edge & 0b1) << _SPI1CON1_CKE_POSITION) |
            ((spi_state.data_sample_timing & 0b1) << _SPI1CON1_SMP_POSITION);

    /*
     * MSB
     * 000-----------0-
     * |||           |
     * |||           +---> FRMDLY: Frame sync pulse precedes first bit clock.
     * ||+---------------> FRMPOL: Frame sync pulse is active low.
     * |+----------------> SPIFSD: Frame sync pulse output.
     * +-----------------> FRMEN:  Framed SPI1 support disabled.
     */
    SPI1CON2 = 0x0000;
    
    /*
     * MSB
     * 0-0------0----??
     * | |      |
     * | |      +--------> SPIROV:  Overflow flag cleared.
     * | +---------------> SPISIDL: Continue module operation in idle mode.
     * +-----------------> SPIEN:   Module disabled.
     */    
    SPI1STAT = 0x0000;
    
    /* Enable the interface. */
    SPI1STATbits.SPIEN = ON;
}

void spiDisable(void) {
    SPI1STATbits.SPIEN = 0;
    RPINR20bits.SDI1R = 0b11111; //B7 MISO
    
    //PPS Disable
    BP_MOSI_RPOUT=0;
    BP_CLK_RPOUT=0;

    //disable all open drain control register bits
    SPIMOSI_ODC = 0;
    SPICLK_ODC = 0;
    SPICS_ODC = 0;
    //make all input maybe???
}

uint8_t spi_write_byte(const uint8_t value) {
    uint8_t result;
    
    /* Put the value on the bus. */
    SPI1BUF = value;
    
    /* Wait until a byte has been read. */
    while (!IFS0bits.SPI1IF) {
    }
    
    /* Get the byte read from the bus. */
    result = SPI1BUF;
    
    /* Free the SPI interface. */
    IFS0bits.SPI1IF = OFF;
    
    return result;
}

//
//
//	SPI Sniffer 
//
//

void spiSniffer(unsigned char csState, unsigned char termMode) {
    unsigned char c, lastCS;

spiSnifferStart:
    lastCS = 1;

    UARTbufSetup();
    spiDisable();
    spi_slave_enable();

    if (csState == 0) { //mode 0, use CS pin
        SPI1CON1bits.SSEN = 1; //CS pin active
        SPI2CON1bits.SSEN = 1; //CS pin active
    }

    if (csState < 2) { //mode 0 & 1, always on
        SPI1STATbits.SPIEN = 1;
        SPI2STATbits.SPIEN = 1;
    }

    while (1) {

        //detect when CS changes. works independently of the data interrupts
        if (lastCS == 0 && SPICS == 1) {
            UARTbuf(']'); //bpWBR; //cs disabled
            lastCS = 1;
        }

        if (SPI1STATbits.SRXMPT == 0 && SPI2STATbits.SRXMPT == 0) {//rx buffer NOT empty, get and display byte
            c = SPI1BUF;

            if (lastCS == 1) {
                UARTbuf('['); //bpWBR; //CS enabled
                lastCS = 0; //SPICS;
            }

            if (termMode) { //show hex output in terminal mode
                bp_write_hex_byte_to_ringbuffer(c);
            } else { //escaped byte value in binary mode
                UARTbuf('\\');
                UARTbuf(c);
            }

            c = SPI2BUF;

            if (termMode) { //show hex output in terminal mode
                UARTbuf('('); //only show the () in terminal mode
                bp_write_hex_byte_to_ringbuffer(c);
                UARTbuf(')');
            } else { //binary mode
                UARTbuf(c);
            }

        }

        if (SPI1STATbits.SPIROV == 1 || SPI2STATbits.SPIROV == 1 || bus_pirate_configuration.overflow == 1) {//we weren't fast enough, buffer overflow

            if (bus_pirate_configuration.overflow == 0) UARTbufFlush();
            SPI1STAT = 0;
            SPI2STAT = 0;

            if (termMode) {
                MSG_SPI_COULD_NOT_KEEP_UP;
                goto spiSnifferStart;
            }

            BP_LEDMODE = 0;
            break;
        }

        UARTbufService();
        if (UART1RXRdy() == 1) {//any key pressed, exit
            c = UART1RX();
            /* JTR usb port; */;
            if (termMode) bpBR; //fixed in 5.1: also sent br to binmode
            break;
        }
    }
    spi_slave_disable();

    spi_setup(spi_bus_speed[mode_configuration.speed]);
}

void spi_slave_enable(void) {
    
    /* Assign slave SPI pin directions. */
    SPICS_TRIS = INPUT;
    SPICLK_TRIS = INPUT;
    SPIMISO_TRIS = INPUT;
    SPIMOSI_TRIS = INPUT;
    
    /* Route SPI pins to the appropriate destinations. */
    RPINR21bits.SS1R = BP_CS_RPIN;
    RPINR23bits.SS2R = BP_CS_RPIN;
    RPINR20bits.SDI1R = BP_MOSI_RPIN;
    RPINR20bits.SCK1R = BP_CLK_RPIN;
    RPINR22bits.SDI2R = BP_MISO_RPIN;
    RPINR22bits.SCK2R = BP_CLK_RPIN;

    /* Prepare SPI interfaces first. */
    
    /*
     * MSB
     * ---0000x0x0xxxxx
     *    |||||||||||||
     *    |||||||||||++---> PPRE:   Primary prescale bits.
     *    ||||||||+++-----> SPRE:   Secondary prescale bits.
     *    |||||||+--------> MSTEN:  Slave mode.
     *    ||||||+---------> CKP:    Clock idle LOW.
     *    |||||+----------> SSEN:   Pin controlled by port function.
     *    ||||+-----------> CKE:    Transition happens from idle to active.
     *    |||+------------> SMP:    Data sampled on data output middle.
     *    ||+-------------> MODE16: Communication is byte-wide.
     *    |+--------------> DISSDO: SDO1 pin is controlled by the module.
     *    +---------------> DISSCK: Internal SPI clock is enabled.
     */
    SPI1CON1 = (spi_bus_speed[mode_configuration.speed] & 0b11111) |
            ((spi_state.clock_polarity & 0b1) << _SPI1CON1_CKP_POSITION) |
            ((spi_state.clock_edge & 0b1) << _SPI1CON1_CKE_POSITION);
    
    /*
     * MSB
     * 000-----------0-
     * |||           |
     * |||           +---> FRMDLY: Frame sync pulse precedes first bit clock.
     * ||+---------------> FRMPOL: Frame sync pulse is active low.
     * |+----------------> SPIFSD: Frame sync pulse output.
     * +-----------------> FRMEN:  Framed SPI1 support disabled.
     */
    SPI1CON2 = 0x0000;
    
    /*
     * MSB
     * 0-0------0----??
     * | |      |
     * | |      +--------> SPIROV:  Overflow flag cleared.
     * | +---------------> SPISIDL: Continue module operation in idle mode.
     * +-----------------> SPIEN:   Module disabled.
     */
    SPI1STAT = 0x0000;

    /*
     * MSB
     * ---00000000xxxxx
     *    |||||||||||||
     *    |||||||||||++---> PPRE:   Primary prescale bits.
     *    ||||||||+++-----> SPRE:   Secondary prescale bits.
     *    |||||||+--------> MSTEN:  Slave mode.
     *    ||||||+---------> CKP:    Clock idle LOW.
     *    |||||+----------> SSEN:   Pin controlled by port function.
     *    ||||+-----------> CKE:    Transition happens from idle to active.
     *    |||+------------> SMP:    Data sampled on data output middle.
     *    ||+-------------> MODE16: Communication is byte-wide.
     *    |+--------------> DISSDO: SDO2 pin is controlled by the module.
     *    +---------------> DISSCK: Internal SPI clock is enabled.
     */
    SPI2CON1 = (spi_bus_speed[mode_configuration.speed] & 0b11111) |
            ((spi_state.clock_polarity & 0b1) << _SPI1CON1_CKP_POSITION) |
            ((spi_state.clock_edge & 0b1) << _SPI1CON1_CKE_POSITION);
    
    /*
     * MSB
     * 000-----------0-
     * |||           |
     * |||           +---> FRMDLY: Frame sync pulse precedes first bit clock.
     * ||+---------------> FRMPOL: Frame sync pulse is active low.
     * |+----------------> SPIFSD: Frame sync pulse output.
     * +-----------------> FRMEN:  Framed SPI2 support disabled.
     */
    SPI2CON2 = 0x0000;
    
    /*
     * MSB
     * 0-0------0----??
     * | |      |
     * | |      +--------> SPIROV:  Overflow flag cleared.
     * | +---------------> SPISIDL: Continue module operation in idle mode.
     * +-----------------> SPIEN:   Module disabled.
     */
    SPI2STAT = 0x0000;

    /* Setup the SPI module to operate in enhanced buffer mode. */
    
    /* Clear RX/TX registers. */
    SPI1BUF = 0x0000;
    SPI2BUF = 0x0000;
    
    /* Set the appropriate mode bits while MSTEN is OFF. */
    SPI1CON1bits.DISSDO = ON;
    SPI2CON1bits.DISSDO = ON;
    
    /* Clear the SMP bits. */
    SPI1CON1bits.SMP = OFF;
    SPI2CON1bits.SMP = OFF;
    
    /* Clear the overflow bits. */
    SPI1STATbits.SPIROV = OFF;
    SPI2STATbits.SPIROV = OFF;
    
    /* Select enhanced buffer mode. */
    SPI1CON2bits.SPIBEN = ON;
    SPI2CON2bits.SPIBEN = ON;
}

void spi_slave_disable(void) {
    
    /* Turn the modules off. */
    SPI1STATbits.SPIEN = OFF;
    SPI2STATbits.SPIEN = OFF;
    
    /* Restore the initial SDOx pin state. */
    SPI1CON1bits.DISSDO = OFF;
    SPI2CON1bits.DISSDO = OFF;
    
    /* Clear pin assignments. */
    RPINR21bits.SS1R = 0b11111;
    RPINR23bits.SS2R = 0b11111;
    RPINR20bits.SDI1R = 0b11111;
    RPINR20bits.SCK1R = 0b11111;
    RPINR22bits.SDI2R = 0b11111;
    RPINR22bits.SCK2R = 0b11111;
}

/*
rawSPI mode:
 * 00000000 � Enter raw bitbang mode, reset to raw bitbang mode
 * 00000001 � SPI mode/rawSPI version string (SPI1)
 * 00000010 � CS low (0)
 * 00000011 � CS high (1)
 * Sniffers
 * 0001xxxx � Bulk SPI transfer, send 1-16 bytes (0=1byte!)
 * 0100wxyz � Configure peripherals, w=power, x=pullups, y=AUX, z=CS
 * 0110xxxx � Set SPI speed, see spi_bus_speed
 * 1000wxyz � SPI config, w=output type, x=idle, y=clock edge, z=sample
 * 00000110 - AVR Extended Commands
 * 00000000 - Null operation - verifies extended commands are available.
 * 00000001 - Return version (2 bytes)
 * 00000010 - Bulk Memory Read from Flash
 */

void spi_enter_binary_io(void) {
    static unsigned char inByte, rawCommand, i;
    unsigned int j, fw, fr;

    mode_configuration.speed = 1;
    spi_state.clock_polarity = SPI_CLOCK_IDLE_LOW;
    spi_state.clock_edge = SPI_TRANSITION_FROM_ACTIVE_TO_IDLE;
    spi_state.data_sample_timing = SPI_SAMPLING_ON_DATA_OUTPUT_MIDDLE;
    mode_configuration.high_impedance = ON;
    spi_setup(spi_bus_speed[mode_configuration.speed]);
    MSG_SPI_MODE_IDENTIFIER;
    
    for (;;) {
        inByte = UART1RX();
        rawCommand = (inByte >> 4); //get command bits in seperate variable

        switch (rawCommand) {
            case 0://reset/setup/config commands
                switch (inByte) {
                    case 0://0, reset exit
                        spiDisable();
                        return; //exit
                        break;
                    case 1://1 - SPI setup and reply string
                        MSG_SPI_MODE_IDENTIFIER;
                        break;
                    case 2:
                        IOLAT &= (~CS); //SPICS=0; //cs enable/low
                        REPORT_IO_SUCCESS();
                        break;
                    case 3:
                        IOLAT |= CS; //SPICS=1; //cs disable/high
                        REPORT_IO_SUCCESS();
                        break;
                    case 0b1101: //all traffic 13
                        REPORT_IO_SUCCESS();
                        spiSniffer(1, 0);
                        break;
                    case 0b1110://cs low 14
                        REPORT_IO_SUCCESS();
                        spiSniffer(0, 0);
                        break;
                        //case 0b1111://cs high
                        //	spiSniffer(1, 0);
                        //	UART1TX(1);
                        //	break;
                    case 4: //write-then-read, with !CS/CS
                    case 5: //write-then-read, NO CS!
                        //get the number of commands that will follow
                        fw = UART1RX();
                        /* JTR usb port; */; //get byte
                        fw = fw << 8;
                        fw |= UART1RX();
                        /* JTR usb port; */; //get byte

                        //get the number of reads to do
                        fr = UART1RX();
                        /* JTR usb port; */; //get byte
                        fr = fr << 8;
                        fr |= UART1RX();
                        /* JTR usb port; */; //get byte


                        //check length and report error
                        if (fw > BP_TERMINAL_BUFFER_SIZE || fr > BP_TERMINAL_BUFFER_SIZE) {
                            REPORT_IO_FAILURE();
                            break;
                        }

                        //get bytes
                        for (j = 0; j < fw; j++) {
                            bus_pirate_configuration.terminal_input[j] = UART1RX();
                            /* JTR usb port; */;
                        }

                        if (inByte == 4) SPICS = 0;
                        for (j = 0; j < fw; j++) {
                            spi_write_byte(bus_pirate_configuration.terminal_input[j]);
                        }
                        bp_delay_us(1);
                        for (j = 0; j < fr; j++) { //read bulk bytes from SPI
                            bus_pirate_configuration.terminal_input[j] = spi_write_byte(0xff);
                        }
                        if (inByte == 4) SPICS = 1;

                        REPORT_IO_SUCCESS();

                        for (j = 0; j < fr; j++) { //send the read buffer contents over serial
                            UART1TX(bus_pirate_configuration.terminal_input[j]);
                        }

                        break;
                        
#ifdef BP_SPI_ENABLE_AVR_EXTENDED_COMMANDS
                    case 6:
                        handle_extended_avr_command();
						break;
#endif /* BP_SPI_ENABLE_AVR_EXTENDED_COMMANDS */
                        
                    default:
                        REPORT_IO_FAILURE();
                        break;
                }
                break;
            case 0b0001://get x+1 bytes
                inByte &= (~0b11110000); //clear command portion
                inByte++; //increment by 1, 0=1byte
                REPORT_IO_SUCCESS();

                for (i = 0; i < inByte; i++) {
                    UART1TX(spi_write_byte(UART1RX()));
                }

                break;
            case 0b0100: //configure peripherals w=power, x=pullups, y=AUX, z=CS
                bp_binary_io_peripherals_set(inByte);
                REPORT_IO_SUCCESS();
                break;

#ifdef BUSPIRATEV4
				case 0b0101:
					UART1TX(bp_binary_io_pullup_control(inByte));
					break;
#endif /* BUSPIRATEV4 */
                    
            case 0b0110://set speed
                inByte &= (~0b11110000); //clear command portion
                if (inByte > sizeof(spi_bus_speed)) {
                    REPORT_IO_FAILURE();
                    break;
                }
                mode_configuration.speed = inByte;
                spi_setup(spi_bus_speed[mode_configuration.speed]); //resetup SPI
                REPORT_IO_SUCCESS();
                break;
                
                /* Set SPI configuration. */
            case 0b1000:
                spi_state.clock_polarity = SPI_CLOCK_IDLE_LOW;
                spi_state.clock_edge = SPI_TRANSITION_FROM_IDLE_TO_ACTIVE;
                spi_state.data_sample_timing =
                        SPI_SAMPLING_ON_DATA_OUTPUT_MIDDLE;
                mode_configuration.high_impedance = OFF;
                spi_state.clock_polarity =
                        (inByte & 0b0100) ? SPI_CLOCK_IDLE_HIGH :
                            SPI_CLOCK_IDLE_LOW;
                spi_state.clock_edge =
                        (inByte & 0b0010) ? SPI_TRANSITION_FROM_IDLE_TO_ACTIVE :
                            SPI_TRANSITION_FROM_ACTIVE_TO_IDLE;
                spi_state.data_sample_timing =
                        (inByte & 0b0001) ? SPI_SAMPLING_ON_DATA_OUTPUT_END :
                            SPI_SAMPLING_ON_DATA_OUTPUT_MIDDLE;
                mode_configuration.high_impedance =
                        (inByte & 0b1000) == 0 ? ON : OFF;
                spi_setup(spi_bus_speed[mode_configuration.speed]);
                REPORT_IO_SUCCESS();
                break;
                
            default:
                REPORT_IO_FAILURE();
                break;
        }
    }
}

#ifdef BP_SPI_ENABLE_AVR_EXTENDED_COMMANDS

void handle_extended_avr_command(void) {
    uint8_t command;
    
    /* Acknowledge extended command. */
    REPORT_IO_SUCCESS();
    
    command = UART1RX();
    switch (command) {
        case BINARY_IO_SPI_AVR_COMMAND_NOOP:
            REPORT_IO_SUCCESS();
            break;
            
        case BINARY_IO_SPI_AVR_COMMAND_VERSION:
            REPORT_IO_SUCCESS();
            UART1TX(HI8(BINARY_IO_SPI_AVR_SUPPORT_VERSION));
            UART1TX(LO8(BINARY_IO_SPI_AVR_SUPPORT_VERSION));
            break;
            
        case BINARY_IO_SPI_AVR_COMMAND_BULK_READ: {
            uint32_t address;
            uint32_t length;
            
            address = (uint32_t)((((uint32_t) UART1RX()) << 24) |
                    (((uint32_t) UART1RX()) << 16) |
                    (((uint32_t) UART1RX()) << 8) |
                    UART1RX());
            length = (uint32_t)((((uint32_t) UART1RX()) << 24) |
                    (((uint32_t) UART1RX()) << 16) |
                    (((uint32_t) UART1RX()) << 8) |
                    UART1RX());
            
            /* @todo: avoid (address + length) integer overflow. */
            
            if ((address > 0xFFFF) || (length > 0xFFFF) ||
                    ((address + length) > 0xFFFF)) {
                REPORT_IO_FAILURE();
                return;
            }
            
            REPORT_IO_SUCCESS();
            while (length > 0) {
                /* Fetch low byte from word. */
                spi_write_byte(AVR_FETCH_LOW_BYTE_COMMAND);
                spi_write_byte((address >> 8) & 0xFF);
                spi_write_byte(address & 0xFF);
                UART1TX(spi_write_byte(0x00));
                length--;
                
                if (length > 0) {
                    /* Fetch high byte from word. */
                    spi_write_byte(AVR_FETCH_HIGH_BYTE_COMMAND);
                    spi_write_byte((address >> 8) & 0xFF);
                    spi_write_byte(address & 0xFF);
                    UART1TX(spi_write_byte(0x00));
                    length--;
                }
                
                address++;
            }
            
            break;
        }
        
        default:
            REPORT_IO_FAILURE();
            break;
    }
}

#endif /* BP_SPI_ENABLE_AVR_EXTENDED_COMMANDS */

#endif /* BP_ENABLE_SPI_SUPPORT */
