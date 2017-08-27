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

#include "proc_menu.h"		// for the userinteraction subs

//direction registers
#define SPIMOSI_TRIS 	BP_MOSI_DIR	
#define SPICLK_TRIS 	BP_CLK_DIR	
#define SPIMISO_TRIS 	BP_MISO_DIR	
#define SPICS_TRIS 		BP_CS_DIR	

//pin control registers
#define SPIMOSI 		BP_MOSI
#define SPICLK 			BP_CLK	
#define SPIMISO 		BP_MISO	
#define SPICS 			BP_CS	

//open drain control registers for OUTPUT pins
#define SPIMOSI_ODC 		BP_MOSI_ODC	
#define SPICLK_ODC 			BP_CLK_ODC	
#define SPICS_ODC 			BP_CS_ODC	

#define SPICS_RPIN		BP_CS_RPIN

extern mode_configuration_t mode_configuration;
extern command_t last_command;
extern bus_pirate_configuration_t bus_pirate_configuration; //we use the big buffer
extern bool command_error;

static void spiSlaveDisable(void);
static void spiSlaveSetup(void);
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

typedef struct {
    
    /** Clock polarity. */
    uint8_t clock_polarity : 1;
    
    /** Clock edge. */
    uint8_t clock_edge : 1;
    
    /** Data sample timing. */
    uint8_t data_sample_timing : 1;
    
    /** CS line state. */
    uint8_t cs_line_state : 1;
    
} spi_state_t; 

/**
 * The SPI protocol state.
 */
static spi_state_t spi_state = { 0 };

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
    //bpWstring("SPI (spd ckp ske smp hiz)=( ");
    BPMSG1191;
    bp_write_dec_byte((mode_configuration.speed + 1));
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
    int speed, clkpol, clkedge, sample, output, cslow;

    consumewhitechars();
    speed = getint();
    consumewhitechars();
    clkpol = getint();
    consumewhitechars();
    clkedge = getint();
    consumewhitechars();
    sample = getint();
    consumewhitechars();
    cslow = getint();
    consumewhitechars();
    output = getint();

    // check for userinput (and sanitycheck it!!)
    if ((speed > 0) && (speed <= 4)) {
        mode_configuration.speed = speed - 1;
    } else {
        speed = 0; // when speed is 0 we ask the user
    }

    if ((clkpol > 0) && (clkpol <= 2)) {
        spi_state.clock_polarity = clkpol - 1;
    } else {
        speed = 0; // when speed is 0 we ask the user
    }

    if ((clkedge > 0) && (clkedge <= 2)) {
        spi_state.clock_edge = clkedge - 1;
    } else {
        speed = 0; // when speed is 0 we ask the user
    }

    if ((sample > 0) && (sample <= 2)) {
        spi_state.data_sample_timing = sample - 1;
    } else {
        speed = 0; // when speed is 0 we ask the user
    }

    if ((cslow > 0) && (cslow <= 2)) {
        spi_state.cs_line_state = (cslow - 1);
    } else {
        speed = 0; // when speed is 0 we ask the user
    }


    if ((output > 0) && (output <= 2)) {
        mode_configuration.high_impedance = (~(output - 1));
    } else {
        speed = 0; // when speed is 0 we ask the user
    }


    if (speed == 0) // no (valid) cmdline options found
    {
        command_error = false; // reset errorflag because of no cmdlineinput

        //bpWstring("Set speed:\x0D\x0A 1. 30KHz\x0D\x0A 2. 125KHz\x0D\x0A 3. 250KHz\x0D\x0A 4. 1MHz\x0D\x0A");
        //bpWline(OUMSG_SPI_SPEED);
        BPMSG1187;
        mode_configuration.speed = getnumber(1, 1, 12, 0) - 1;

        //bpWstring("Clock polarity:\x0D\x0A 1. Idle low *default\x0D\x0A 2. Idle high\x0D\x0A");
        //bpWmessage(MSG_OPT_CKP);
        BPMSG1188;
        spi_state.clock_polarity = getnumber(1, 1, 2, 0) - 1;

        //bpWstring("Output clock edge:\x0D\x0A 1. Idle to active\x0D\x0A 2. Active to idle *default\x0D\x0A");
        //bpWmessage(MSG_OPT_CKE);
        BPMSG1189;
        spi_state.clock_edge = getnumber(2, 1, 2, 0) - 1;

        //bpWstring("Input sample phase:\x0D\x0A 1. Middle *default\x0D\x0A 2. End\x0D\x0A");
        //bpWmessage(MSG_OPT_SMP);
        BPMSG1190;
        spi_state.data_sample_timing = getnumber(1, 1, 2, 0) - 1;

        MSG_SPI_CS_MODE_PROMPT;
        spi_state.cs_line_state = getnumber(2, 1, 2, 0) - 1;

        MSG_PIN_OUTPUT_TYPE_PROMPT;
        mode_configuration.high_impedance = (~(getnumber(1, 1, 2, 0) - 1));
    } else {
        SPIsettings();
    }
    mode_configuration.write_with_read = 0;
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
            BPMSG1071; //moved to a more generic message
            BPMSG1250;
            spiSniffer(0, 1); //configure for terminal mode
            break;
            
        case 2://sniff all
            BPMSG1071; //moved to a more generic message
            BPMSG1250;
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
    SPI1STATbits.SPIEN = 0; //disable, just in case...

    //use open drain control register to
    //enable Hi-Z mode on hardware module outputs
    //inputs are already HiZ
    if (mode_configuration.high_impedance == 1) {
        SPIMOSI_ODC = 1;
        SPICLK_ODC = 1;
        SPICS_ODC = 1;
    } else {
        SPIMOSI_ODC = 0;
        SPICLK_ODC = 0;
        SPICS_ODC = 0;
    }

	//PPS Setup
	// Inputs
	RPINR20bits.SDI1R = BP_MISO_RPIN; //B7 MISO
	// Outputs
	BP_MOSI_RPOUT = SDO1_IO; //B9 MOSI
	BP_CLK_RPOUT = SCK1OUT_IO; //B8 CLK
	
    SPICS = 1; //B6 cs high
    SPICS_TRIS = 0; //B6 cs output

    //pps configures pins and this doesn't really matter....
    SPICLK_TRIS = 0; //B8 sck output
    SPIMISO_TRIS = 1; //B7 SDI input
    SPIMOSI_TRIS = 0; //B9 SDO output

    /* CKE=1, CKP=0, SMP=0 */
    //(SPIspeed[modeConfig.speed]);
    SPI1CON1 = spi_speed;  
    SPI1CON1bits.MSTEN = ON;
    SPI1CON1bits.CKP = spi_state.clock_polarity;
    SPI1CON1bits.CKE = spi_state.clock_edge;
    SPI1CON1bits.SMP = spi_state.data_sample_timing;
    SPI1CON2 = 0;
    SPI1STAT = 0; // clear SPI
    SPI1STATbits.SPIEN = 1;
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
    spiSlaveSetup();

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
    spiSlaveDisable();

    spi_setup(spi_bus_speed[mode_configuration.speed]);
}

//configure both SPI units for slave mode on different pins
//use current settings

void spiSlaveSetup(void) {
    //	unsigned char c;
    //SPI1STATbits.SPIEN=0; //SPI module off

    //assign pins for SPI slave mode
    SPICS_TRIS = 1; //B6 cs input
    SPICLK_TRIS = 1; //B8 sck input
    SPIMISO_TRIS = 1; //B7 SDI input
    SPIMOSI_TRIS = 1; //b9 SDO input
    
    //More PPS
    RPINR21bits.SS1R = BP_CS_RPIN; //SPICS_RPIN; //assign CS function to bus pirate CS pin
    RPINR23bits.SS2R = BP_CS_RPIN;
    RPINR20bits.SDI1R = BP_MOSI_RPIN; //B9 MOSI
    RPINR20bits.SCK1R = BP_CLK_RPIN; //SPICLK_RPIN; //assign SPI1 CLK input to bus pirate CLK pin
    RPINR22bits.SDI2R = BP_MISO_RPIN; //B7 MiSo
    RPINR22bits.SCK2R = BP_CLK_RPIN; //SPICLK_RPIN; //assign SPI2 CLK input to bus pirate CLK pin

    //clear old SPI settings first
    SPI1CON1 = (spi_bus_speed[mode_configuration.speed]); // CKE (output edge) active to idle, CKP idle low, SMP data sampled middle of output time.
    SPI1CON1bits.CKP = spi_state.clock_polarity;
    SPI1CON1bits.CKE = spi_state.clock_edge;
    SPI1CON2 = 0;
    SPI1STAT = 0; // clear SPI

    SPI2CON1 = (spi_bus_speed[mode_configuration.speed]); // CKE (output edge) active to idle, CKP idle low, SMP data sampled middle of output time.
    SPI2CON1bits.CKP = spi_state.clock_polarity;
    SPI2CON1bits.CKE = spi_state.clock_edge;
    SPI2CON2 = 0;
    SPI2STAT = 0; // clear SPI

    //To set up the SPI module for the Enhanced Buffer
    //Slave mode of operation:
    //1. Clear the SPIxBUF register.
    SPI1BUF = 0;
    SPI2BUF = 0;
    //3. Write the desired settings to the SPIxCON1 and SPIxCON2 registers with MSTEN (SPIxCON1<5>) = 0.
    //4. Clear the SMP bit.
    SPI1CON1bits.SMP = 0;
    SPI2CON1bits.SMP = 0;

    //	SPI1CON1bits.SSEN=1; //CS pin active
    //	SPI2CON1bits.SSEN=1; //CS pin active

    SPI1CON1bits.DISSDO = 1; //Disable SDO pin in slave mode
    SPI1CON1bits.MSTEN = 0;
    SPI2CON1bits.DISSDO = 1; //Disable SDO pin in slave mode
    SPI2CON1bits.MSTEN = 0;
    //5. If the CKE bit is set, then the SSEN bit must be set, thus enabling the SSx pin.
    //6. Clear the SPIROV bit (SPIxSTAT<6>).
    SPI1STATbits.SPIROV = 0;
    SPI2STATbits.SPIROV = 0;
    //7. Select Enhanced Buffer mode by setting the SPIBEN bit (SPIxCON2<0>).
    SPI1CON2bits.SPIBEN = 1;
    SPI2CON2bits.SPIBEN = 1;
    //8. Enable SPI operation by setting the SPIEN bit(SPIxSTAT<15>).
    //SPI1STATbits.SPIEN=1;

}

void spiSlaveDisable(void) {
    SPI1STATbits.SPIEN = 0; //SPI module off
    SPI1CON1bits.DISSDO = 0; //restore SDO pin
    SPI2STATbits.SPIEN = 0; //SPI module off
    SPI2CON1bits.DISSDO = 0; //restore SDO pin
    
    RPINR21bits.SS1R = 0b11111; //assign CS input to none
    RPINR23bits.SS2R = 0b11111; //assign CS input to none
    RPINR20bits.SDI1R = 0b11111;
    RPINR20bits.SCK1R = 0b11111; //assign CLK input to none
    RPINR22bits.SDI2R = 0b11111;
    RPINR22bits.SCK2R = 0b11111; //assign CLK input to none

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

    //useful default values
    /* CKE=1, CKP=0, SMP=0 */
    mode_configuration.speed = 1;
    spi_state.clock_polarity = SPI_CLOCK_IDLE_LOW;
    spi_state.clock_edge = SPI_TRANSITION_FROM_ACTIVE_TO_IDLE;
    spi_state.data_sample_timing = SPI_SAMPLING_ON_DATA_OUTPUT_MIDDLE;
    mode_configuration.high_impedance = ON;
    spi_setup(spi_bus_speed[mode_configuration.speed]);
    MSG_SPI_MODE_IDENTIFIER;
    
    for (;;) {

        inByte = UART1RX(); /* JTR usb port; */ //grab it
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
