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
#include "busPirateCore.h"

extern bus_pirate_configuration_t bpConfig; //global config variables
extern mode_configuration_t modeConfig; //mode config variables

void clearModeConfig(void); //private function to clear modeConfig on reset

//private function to clear modeConfig on reset
void clearModeConfig(void){
	//reset the options structure here...
	modeConfig.HiZ=0;
	modeConfig.speed=0; 
	modeConfig.periodicService=0;
	modeConfig.altAUX=0;
	modeConfig.lsbEN=0;
}

//get bus pirate ready after first start
//or clean up after any dirty modules
//modules MUST teardown any PPS functions on the CMD_CLEANUP command
void bpInit(void){
	//do this manually so there aren't problems with hardware
	//that has I/O on different ports
	BP_MOSI_DIR=1;
	BP_CLK_DIR=1; 
	BP_MISO_DIR=1;
	BP_CS_DIR=1; 
	BP_AUX0_DIR=1;
	BP_LEDMODE=0; //mode LED OFF
	BP_LEDMODE_DIR=0;
#ifdef BUSPIRATEV4
	BP_USBLED_OFF();
	BPV4_HWI2CPINS_SETUP();
	BP_BUTTON_SETUP();
	BP_3V3PU_OFF();
	//BP_5VPU_OFF();
	eeprom_initialize();
#endif /* BUSPIRATEv4 */

	BP_AUX_RPOUT = 0;    //remove output from AUX pin (PWM/servo modes)

	bpConfig.busMode=BP_HIZ;

	clearModeConfig(); //reset the mode settings structure
	
		BP_PULLUP_OFF();

	BP_VREG_OFF();//disable the VREG

	//setup voltage monitoring on ADC. see hardwarevx.h!
	BP_ADC_PINSETUP();

	//configure the ADC
	AD1CON1bits.SSRC  = 0b111;// SSRC<3:0> = 111 implies internal
								// counter ends sampling and starts
								// converting.
	AD1CSSL = 0;
	AD1CON3 = 0x1F02; // Sample time = 31Tad,
						// Tad = 2 Tcy
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

//takes a measurement from the ADC probe and prints the result to the user terminal
void bpADCprobe(void){
	AD1CON1bits.ADON = 1; // turn ADC ON
	bpWvolts(bp_read_adc(BP_ADC_PROBE)); //print measurement
	AD1CON1bits.ADON = 0; // turn ADC OFF
}

// measure constantly 
void bpADCCprobe(void)
{	unsigned int temp;

	//bpWline(OUMSG_PS_ADCC);
	BPMSG1042;
	//bpWline(OUMSG_PS_ANY_KEY);
	BPMSG1250; //BPMSG1043;
	//bpWstring(OUMSG_PS_ADC_VOLT_PROBE);
	BPMSG1044;
	bpWvolts(0);						// print dummy (0v)
	//bpWstring(OUMSG_PS_ADC_VOLTS);
	BPMSG1045;
	while(!UART1RXRdy())				// wait for keypress
	{	AD1CON1bits.ADON = 1;			// turn ADC ON
		temp=bp_read_adc(BP_ADC_PROBE);
		AD1CON1bits.ADON = 0;			// turn ADC OFF
		bpWstring("\x08\x08\x08\x08\x08");	// 5x backspace ( e.g. 5.00V )
		//BPMSG1046;
		bpWvolts(temp);					// print measurement
		//bpWstring(OUMSG_PS_ADC_VOLTS);
		BPMSG1045;

		// CvD: wait xx ms??
	}
	UART1RX();
	bpWline("");							// need a linefeed :D
}


//print byte c to the user terminal in the format 
//  specified by the bpConfig.displayMode setting
void bpWbyte(unsigned int c)
{	if(modeConfig.numbits<16)
	{	c&=(0x7FFF>>((16-modeConfig.numbits)-1));
	}
	switch(bpConfig.displayMode){
		case HEX:
			if(modeConfig.int16) bpWinthex(c); else bpWhex(c);
			break;
		case DEC:
			if(modeConfig.int16) bpWintdec(c); else bpWdec(c);
			break;
		case BIN:
			if(modeConfig.int16)
			{	bpWbin(c); bpSP;
			}
			bpWbin(c);
			break;
		case RAW:
			if(modeConfig.int16)
			{	UART1TX(c>>8);
			}
			UART1TX(c&0x0FF);
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
	
	/* When running at 32MHz, it can execute 16 instructions per µS */
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


/*
unsigned char bpRevByte(unsigned char c){
	unsigned char r=0, i;

	for(i=0b1; i!=0; i=i<<1){
		r=r<<1;	
		if(c&i)r|=0b1;
	}

	if(modeConfig.numbits!=8)
	{	r>>=(8-modeConfig.numbits);
	}
	return r;
}
*/

unsigned int bpRevByte(unsigned int c)
{	unsigned int r=0, i;

	for(i=0b1; i!=0; i=i<<1)
	{	r=r<<1;	
		if(c&i)r|=0b1;
	}

	if(modeConfig.numbits!=16)
	{	r>>=(16-modeConfig.numbits);
	}
	return r;
}


