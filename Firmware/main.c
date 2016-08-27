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
    
///////////////////////////////////////////
/////////////[ NOTE ]//////////////////////////////////////
///    The settings and configuration are on [base.h] please open that file if you are new to
/// compiling the Bus Pirate Firmware.
///
///                                                                      DangerousPrototypes.com
////                                                                 For support and information
/////                                                        visit the DangerousPrototypes Forum
///////////
  
#include "busPirateCore.h"
#include "procMenu.h"
#include "selftest.h"
#include "basic.h"

#ifdef BUSPIRATEV4
#include "descriptors.h"

void USBSuspend(void);
void _USB1Interrupt(void);
extern volatile BYTE usb_device_state; // JTR added
#endif /* BUSPIRATEV4 */

#if defined(BUSPIRATEV3)
//set custom configuration for PIC 24F (now always set in bootloader page, not needed here)
// Internal FRC OSC = 8MHz
#pragma config FNOSC     = FRCPLL
#pragma config OSCIOFNC  = ON 
#pragma config POSCMOD   = NONE
#pragma config I2C1SEL   = PRI
// turn off junk we don't need
#pragma config JTAGEN    = OFF
#pragma config GCP       = OFF
#pragma config GWRP      = OFF
#pragma config COE       = OFF
#pragma config FWDTEN    = OFF
#pragma config ICS       = PGx1
#endif /* BUSPIRATEV3 */

#if defined (BUSPIRATEV4)
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

void ISRTable(); //Pseudo function to hold ISR remap jump table
void Initialize(void);

static unsigned char __attribute__((section(".bss.end"))) _buffer[TERMINAL_BUFFER_SIZE];
bus_pirate_configuration_t bpConfig = {.terminalInput = _buffer}; //holds persistant bus pirate settings (see busPirateCore.h)
mode_configuration_t modeConfig; //holds mode info, cleared between modes
command_t bpCommand; //holds the current active command so we don't ahve to put so many variables on the stack

#pragma code
//this loop services user input and passes it to be processed on <enter>

int main(void) {

    Initialize(); //setup bus pirate

    //wait for the USB connection to enumerate
#if defined (BUSPIRATEV4) && !defined (BPV4_DEBUG)
    BP_LEDUSB_DIR = 0;
    //BP_LEDUSB = 1;
    BP_USBLED_ON();
    //BP_VREGEN_DIR = 0;
   // BP_VREGEN = 1;
    //BP_LEDMODE_DIR = 0;
    //BP_LEDMODE = 1;
#ifdef USB_INTERRUPTS
    EnableUsbPerifInterrupts(USB_TRN + USB_SOF + USB_UERR + USB_URST);
    EnableUsbGlobalInterrupt();
#endif    

    do {
#ifndef USB_INTERRUPTS
//        if (!TestGlobalUsbInterruptEnable()) //JTR3 added
            usb_handler(); ////service USB tasks Guaranteed one pass in polling mode even when usb_device_state == CONFIGURED_STATE
#endif 
        //        if ((usb_device_state < DEFAULT_STATE)) { // JTR2 no suspendControl available yet || (USBSuspendControl==1) ){
        //        } else if (usb_device_state < CONFIGURED_STATE) {
        //        }
    } while (usb_device_state < CONFIGURED_STATE); // JTR addition. Do not proceed until device is configured.
    BP_USBLED_OFF();
    usb_register_sof_handler(CDCFlushOnTimeout); // For timeout value see: cdc_config.h -> BPv4 -> CDC_FLUSH_MS

#endif
    serviceuser();
    return 0;
}

//bus pirate initialization
//setup clock, terminal UART, pins, LEDs, and display version info

#ifdef BUSPIRATEV3

//Version | RB3 | RB2
//2go, 3a | 1   |  1
//v3b     | 1   |  0
//v3.5    | 0   |  0

static const uint8_t BPV3_HARDWARE_VERSION_TABLE[] = {
    /* RB3 == 0 && RB2 == 0 */
    '5',
    /* RB3 == 0 && RB2 == 1 */
    '?',
    /* RB3 == 1 && RB2 == 0 */
    'b',
    /* RB3 == 1 && RB2 == 1 */
    'a'
};

#endif /* BUSPIRATEV3 */

void Initialize(void) {
    volatile unsigned long delay = 0xffff;

    //   volatile unsigned long delay = 0xffff;
    // TBLPAG = 0; // we need to be in page 0 (somehow this isn't set)
#ifdef BUSPIRATEV3
    CLKDIVbits.RCDIV0 = 0; //clock divider to 0
    AD1PCFG = 0xFFFF; // Default all pins to digital
#elif defined (BUSPIRATEV4)
    INTCON1bits.NSTDIS = 1;
    CLKDIV = 0x0000; // Set PLL prescaler (1:1)
    BP_LEDUSB_DIR = 0; // output
    CORCONbits.PSV = 1; // JTR PIC24 fixup ?? PSV not being initialized. May have been done by c_init though.
    PSVPAG = 0; //
    OSCCONbits.SOSCEN = 0;
    AD1PCFGL = 0x7FD8; //BUSPIRATEV4 has five analog pins b0, b1, b2, b5, b15
    AD1PCFGH = 0x2;
    // usb_register_sof_handler(0);
#endif /* BUSPIRATEV3 || BUSPIRATEV4 */

    OSCCONbits.SOSCEN = 0;


    while (delay--);
    //set pin configuration using peripheral pin select
#ifdef BUSPIRATEV3
    BP_TERM_RX = BP_TERM_RX_RP; //Inputs UART1 RX RPINR18bits.U1RXR=4;
    BP_TERM_TX_RP = BP_TERM_TX; // Outputs UART1 TX RPOR1bits.RP3R=U1TX_IO;
#elif defined (BUSPIRATEV4) && defined (BPV4_DEBUG)
    BP_TERM_RX = BP_TERM_RX_RP; //Inputs UART1 RX RPINR18bits.U1RXR=11;//AUX2
    BP_TERM_TX_RP = BP_TERM_TX; // Outputs UART1 TX RPOR1bits.RP2R=U1TX_IO;//AUX1
#endif /* BUSPIRATEV3 || (BUSPIRATEV4 && BPV4_DEBUG) */

    //put startup values in config (do first)
    bpConfig.termSpeed = 8; //default PC side port speed, startup in 115200, or saved state (later)....
    bpConfig.displayMode = HEX;

    bpInit(); //put startup values in config (do first)clean up, exit in HI-Z

#ifdef BUSPIRATEV3
    InitializeUART1(); //init the PC side serial port
#endif /* BUSPIRATEV3 */

#if defined (BUSPIRATEV4) && !defined (BPV4_DEBUG)
    initCDC();
    usb_init(cdc_device_descriptor, cdc_config_descriptor, cdc_str_descs, USB_NUM_STRINGS);
    usb_start();
#endif

#if defined (BUSPIRATEV4) && defined (BPV4_DEBUG)
    InitializeUART1(); //init the PC side serial port
#endif

#if  defined(BUSPIRATEV3)
    /* Turn pullups ON. */
    CNPU1bits.CN6PUE = ON;
    CNPU1bits.CN7PUE = ON;
#endif /* BUSPIRATEV3 */

    bpConfig.dev_type = bpReadFlash(DEV_ADDR_UPPER, DEV_ADDR_TYPE);
    bpConfig.dev_rev = bpReadFlash(DEV_ADDR_UPPER, DEV_ADDR_REV);

#if defined(BUSPIRATEV3)
    /* Get the revision identifier. */
    bpConfig.HWversion = BPV3_HARDWARE_VERSION_TABLE[(PORTB >> 2) & 0b00000011];

    /* Turn pullups OFF. */
    CNPU1bits.CN6PUE = OFF;
    CNPU1bits.CN7PUE = OFF;
#else
    bpConfig.HWversion = 0;
#endif /* BUSPIRATEV3 */

    bpConfig.quiet = 0; // turn output on (default)
    modeConfig.numbits = 8;
    
#ifdef BP_USE_BASIC
    initpgmspace();
#endif

    TBLPAG = 0; // we need to be in page 0 (somehow this isn't set)

#ifndef BUSPIRATEV4
    bpWBR; //send a line feed
    versionInfo(); //prints hardware and firmware version info (base.c)
#endif /* !BUSPIRATEV4 */
}

//Interrupt Remap method 2:  Using Goto and jump table
/*
void __attribute__ ((interrupt,no_auto_psv)) _T1Interrupt(){
        IFS0bits.T1IF = 0;
        IEC0bits.T1IE = 0;
        PR1 = 0xFFFF;
        T1CON = 0;
        irqFlag=1;
}
 */

#ifdef BUSPIRATEV4

void USBSuspend(void) {

}
#pragma interrupt _USB1Interrupt

void __attribute__((interrupt, no_auto_psv)) _USB1Interrupt() {

    //USB interrupt
    //IRQ enable IEC5bits.USB1IE
    //IRQ flag	IFS5bits.USB1IF
    //IRQ priority IPC21<10:8>
    //{
    usb_handler();
    IFS5bits.USB1IF = 0; //	PIR2bits.USBIF = 0;
    //}
}
#pragma code

#endif /* BUSPIRATEV4 */


