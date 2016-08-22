//
// SUMP LA
//

#include "base.h"
#include "UART.h"
#include "busPirateCore.h"
#include "SUMP.h"
#include "baseIO.h"

/*
 * SUMP command set - taken from http://www.sump.org/projects/analyzer/protocol/
 * With additions taken from http://dangerousprototypes.com/docs/The_Logic_Sniffer's_extended_SUMP_protocol
 * 
 * @TODO: Add commands 0x0F, 0x9E, 0x9F from the extended SUMP protocol?
 * @TODO: Add "Set trigger configuration" command (0xC2, 0xC6, 0xCA, 0xCE).
 */

/**
 * Resets the device.
 * 
 * Should be sent 5 times when the receiver status is unknown. (It could be
 * waiting for up to four bytes of pending long command data.)
 */
#define SUMP_RESET 	0x00

/**
 * Arms the trigger.
 */
#define SUMP_RUN	0x01

/**
 * Asks for device identification.
 * 
 * The device will respond with four bytes. The first three ("SLA") identify
 * the device. The last one identifies the protocol version which is currently
 * either "0" or "1"
 */
#define SUMP_ID		0x02

/**
 * Get metadata.
 * 
 * In response, the device sends a series of 1-byte keys, followed by data
 * pertaining to that key. The series ends with the key 0x00. The system can be
 * extended with new keys as more data needs to be reported.
 * 
 * Type 0 Keys (null-terminated string, UTF-8 encoded):
 * 
 * 0x00 Not used, key means end of metadata
 * 0x01 Device name (e.g. "Openbench Logic Sniffer v1.0", "Bus Pirate v3b")
 * 0x02 Version of the FPGA firmware
 * 0x03 Ancillary version (PIC firmware)
 * 
 * Type 1 Keys (32-bit unsigned integer):
 * 
 * 0x20 Number of usable probes
 * 0x21 Amount of sample memory available (bytes)
 * 0x22 Amount of dynamic memory available (bytes)
 * 0x23 Maximum sample rate (Hz)
 * 0x24 Protocol version (see below) [*]
 * 
 * Type 2 Keys (8-bit unsigned integer):
 * 
 * 0x40 Number of usable probes (short)
 * 0x41 Protocol version (short) 
 * 
 * [*]
 * 
 * The protocol version key holds a 4-stage version, one per byte, where the
 * MSB holds the major version number. As of the first release to support this
 * metadata command, the protocol version should be 2. This would be encoded
 * as 0x00000002. 
 */
#define SUMP_DESC	0x04

/**
 * Put transmitter out of pause mode.
 * 
 * It will continue to transmit captured data if any is pending. This command
 * is being used for xon/xoff flow control.
 */
#define SUMP_XON	0x11

/**
 * Put transmitter in pause mode.
 * 
 * It will stop transmitting captured data. This command is being used for
 * xon/xoff flow control.
 */
#define SUMP_XOFF 	0x13

/**
 * Set Divider.
 * 
 * When x is written, the sampling frequency is set to f = clock / (x + 1)
 * 
 *          LSB                  MSB
 * 10000000 XXXXXXXXXXXXXXXXXXXXXXXX????????
 *          ||||||||||||||||||||||||
 *          ++++++++++++++++++++++++----------- Divider
 */
#define SUMP_DIV 	0x80

/**
 * Set Read & Delay Count.
 * 
 * Read Count is the number of samples (divided by four) to read back from
 * memory and sent to the host computer. Delay Count is the number of samples
 * (divided by four) to capture after the trigger fired. A Read Count bigger
 * than the Delay Count means that data from before the trigger match will be
 * read back. This data will only be valid if the device was running long
 * enough before the trigger matched.
 * 
 *          LSB          MSB LSB          MSB
 * 10000001 XXXXXXXXXXXXXXXX YYYYYYYYYYYYYYYY
 *          |||||||||||||||| ||||||||||||||||
 *          |||||||||||||||| ++++++++++++++++--- Delay Count
 *          ++++++++++++++++-------------------- Read Count
 */
#define SUMP_CNT	0x81

/**
 * Set Flags.
 * 
 * Sets the following flags:
 * 
 * - demux: Enables the demux input module. (Filter must be off.)
 * - filter: Enables the filter input module. (Demux must be off.)
 * - channel groups: Disable channel group. Disabled groups are excluded from
 *                   data transmissions. This can be used to speed up transfers.
 *                   There are four groups, each represented by one bit.
 *                   Starting with the least significant bit of the channel
 *                   group field channels are assigned as follows: 0-7, 8-15,
 *                   16-23, 24-31
 * - external: Selects the clock to be used for sampling. If set to 0, the
 *             internal clock divided by the configured divider is used, and if
 *             set to 1, the external clock will be used. (filter and demux are
 *             only available with internal clock)
 * - inverted: When set to 1, the external clock will be inverted before being
 *             used. The inversion causes a delay that may cause problems at
 *             very high clock rates. This option only has an effect with
 *             external set to 1.
 * 
 * 10000010 ABCCCCDE ????????????????????????
 *          ||||||||
 *          |||||||+---------------------------- Demux (1: Enable)
 *          ||||||+----------------------------- Filter (1: Enable)
 *          ||++++------------------------------ Channel Groups (1: Disable)
 *          |+---------------------------------- External (1: Enable)
 *          +----------------------------------- Inverted (1: Enable)
 */
#define SUMP_FLAGS	0x82

/**
 * Set Trigger Values.
 * 
 * Defines which trigger values must match. In parallel mode each bit
 * represents one channel, in serial mode each bit represents one of the last
 * 32 samples of the selected channel. The opcodes refer to stage 0-3 in the
 * order given below. (Protocol version 0 only supports stage 0.)
 * 
 *          LSB                          MSB
 * 1100xx00 XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
 *          ||||||||||||||||||||||||||||||||
 *          ++++++++++++++++++++++++++++++++--- Trigger Mask
 */
#define SUMP_TRIG	0xc0

/**
 * Set trigger mask.
 * 
 * Defines which values individual bits must have. In parallel mode each bit
 * represents one channel, in serial mode each bit represents one of the last
 * 32 samples of the selected channel. The opcodes refer to stage 0-3 in the
 * order given above. (Protocol version 0 only supports stage 0.)
 * 
 *          LSB                          MSB
 * 1100xx01 XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
 *          ||||||||||||||||||||||||||||||||
 *          ++++++++++++++++++++++++++++++++--- Trigger Mask
 */
#define SUMP_TRIG_VALS 0xc1

/**
 * Not used, key means end of metadata.
 */
#define SUMP_METADATA_END 0x00

/**
 * Device name (e.g. "Openbench Logic Sniffer v1.0", "Bus Pirate v3b").
 */
#define SUMP_METADATA_DEVICE_NAME 0x01

/**
 * Version of the FPGA firmware.
 */
#define SUMP_METADATA_FPGA_VERSION 0x02

/**
 * Ancillary version (PIC firmware).
 */
#define SUMP_METADATA_ANCILLARY_VERSION 0x03

/**
 * Number of usable probes.
 */
#define SUMP_METADATA_USABLE_PROBES_NUMBER 0x20

/**
 * Amount of sample memory available (bytes).
 */
#define SUMP_METADATA_SAMPLE_MEMORY_AVAILABLE 0x21

/**
 * Amount of dynamic memory available (bytes).
 */
#define SUMP_METADATA_DYNAMIC_MEMORY_AVAILABLE 0x22

/**
 * Maximum sample rate (Hz).
 */
#define SUMP_METADATA_MAXIMUM_SAMPLE_RATE 0x23

/**
 * Protocol version.
 */
#define SUMP_METADATA_PROTOCOL_VERSION 0x24

/**
 * Number of usable probes (short).
 */
#define SUMP_METADATA_USABLE_PROBES_SHORT_NUMBER 0x40

/**
 * Protocol version (short) .
 */
#define SUMP_METADATA_PROTOCOL_SHORT_VERSION 0x41

#define BP_SUMP_SAMPLE_MEMORY_SIZE 4096
#define BP_SUMP_SAMPLE_RATE 1000000
#define BP_SUMP_PROBES_COUNT 5
#define BP_SUMP_PROTOCOL_VERSION 2

static const uint8_t SUMP_METADATA[] = {
    /* Device name. */
    
    SUMP_METADATA_DEVICE_NAME,
#ifdef BUSPIRATEV4
    'B', 'P', 'v', '4', '\0',
#elif BUSPIRATEV3
    'B', 'P', 'v', '3', '\0',
#else
#error "Invalid or unknown Bus Pirate version!"
#endif /* BUSPIRATEV4 || BUSPIRATEV3 */
    
    /* Sample memory (4096 bytes). */
    
    SUMP_METADATA_SAMPLE_MEMORY_AVAILABLE,
    (uint8_t) ((uint32_t) BP_SUMP_SAMPLE_MEMORY_SIZE >> 24),
    (uint8_t) (((uint32_t) BP_SUMP_SAMPLE_MEMORY_SIZE >> 16) & 0xFF),
    (uint8_t) (((uint32_t) BP_SUMP_SAMPLE_MEMORY_SIZE >> 8) & 0xFF),
    (uint8_t) ((uint32_t) BP_SUMP_SAMPLE_MEMORY_SIZE & 0xFF),
    
    /* Sample rate (1MHz). */
    
    SUMP_METADATA_MAXIMUM_SAMPLE_RATE,
    (uint8_t) ((uint32_t) BP_SUMP_SAMPLE_RATE >> 24),
    (uint8_t) (((uint32_t) BP_SUMP_SAMPLE_RATE >> 16) & 0xFF),
    (uint8_t) (((uint32_t) BP_SUMP_SAMPLE_RATE >> 8) & 0xFF),
    (uint8_t) ((uint32_t) BP_SUMP_SAMPLE_RATE & 0xFF),

    /* Number of probes (5). */
    
    SUMP_METADATA_USABLE_PROBES_SHORT_NUMBER,
    BP_SUMP_PROBES_COUNT,
    
    /* Protocol version (v2). */
    
    SUMP_METADATA_PROTOCOL_SHORT_VERSION,
    BP_SUMP_PROTOCOL_VERSION,
    
    SUMP_METADATA_END
};

extern struct _bpConfig bpConfig; //holds persistant bus pirate settings (see busPirateCore.h)

static enum _LAstate {
	LA_IDLE = 0,
	LA_ARMED,
} LAstate = LA_IDLE;

#define LA_SAMPLE_SIZE TERMINAL_BUFFER //(see busPirateCore.h)
//static unsigned char samples[LA_SAMPLE_SIZE];
static unsigned char sumpPadBytes;
static unsigned int sumpSamples;


void SUMP(void){

	IODIR|=(AUX+MOSI+CLK+MISO+CS);//AUX, MOSI, CLK, MISO, CS pins to input
	//SUMPlogicCommand(SUMP_RESET);
	SUMPreset();
	SUMPlogicCommand(SUMP_ID);
	while(1){
		if(UART1RXRdy()){
			if(SUMPlogicCommand(UART1RX())) return;
		}
		if(SUMPlogicService())return; //exit at end of sampling
	}
}

void SUMPreset(void){
	BP_LEDMODE=0;//LED
	CNPU1=0; //pullups off
	CNPU2=0;
	CNEN1=0; //all change notice off
	CNEN2=0;
	T4CON=0; //stop count
	IPC4bits.CNIP=0;

	//default speed and samples
	//setup PR register
	PR5=0;//most significant word
	PR4=0x640;//least significant word
	sumpSamples=LA_SAMPLE_SIZE;
	sumpPadBytes=0;
	LAstate=LA_IDLE;
}

unsigned char SUMPlogicCommand(unsigned char inByte){
//	static unsigned char i;
	static unsigned long l;

	static enum _SUMP {
		C_IDLE = 0,
		C_PARAMETERS,
		C_PROCESS,
	} sumpRXstate = C_IDLE;

	static struct _sumpRX {
		unsigned char command[5];
		unsigned char parameters;
		unsigned char parCnt;
	} sumpRX;

	switch(sumpRXstate){ //this is a state machine that grabs the incoming commands one byte at a time

		case C_IDLE:

			switch(inByte){//switch on the current byte
                
				case SUMP_RESET://reset
					SUMPreset();
					return 1;
                    
				case SUMP_ID://SLA0 or 1 backwards: 1ALS
					bpWstring("1ALS");
					break;
                    
				case SUMP_RUN://arm the triger
					BP_LEDMODE=1;//ARMED, turn on LED

					//setup timer
					T4CON=0;	//make sure the counters are off
					//timer 4 internal, measures interval
					TMR5HLD=0x00;
					TMR4=0x00;
					T4CON=0b1000; //.T32=1, bit 3

					//setup change notice interrupt
					//CNEN2=0b111100001; //change notice on all pins
					//CNEN2=0b100000; //change notice on CS pin
					IFS1bits.CNIF=0; //clear interrupt flag
					IEC1bits.CNIE=0; //interrupts DISABLED
					IPC4bits.CNIP=1; //priority to 0, not actual interupt

					LAstate=LA_ARMED;
					break;
                    
				case SUMP_DESC:
                    			bpWriteBuffer(SUMP_METADATA, sizeof(SUMP_METADATA));
					break;
                    
				case SUMP_XON://resume send data
				//	xflow=1;
					break;
                    
				case SUMP_XOFF://pause send data
				//	xflow=0;
					break;
                    
				default://long command
					sumpRX.command[0]=inByte;//store first command byte
					sumpRX.parameters=4; //all long commands are 5 bytes, get 4 parameters
					sumpRX.parCnt=0;//reset the parameter counter
					sumpRXstate=C_PARAMETERS;
					break;
			}
			break;
		case C_PARAMETERS: 
			sumpRX.parCnt++;
			sumpRX.command[sumpRX.parCnt]=inByte;//store each parameter
			if(sumpRX.parCnt<sumpRX.parameters) break; //if not all parameters, quit
		case C_PROCESS: //ignore all long commands for now
			switch(sumpRX.command[0]){

				case SUMP_TRIG: //set CN on these pins
					if(sumpRX.command[1] & 0b10000)	CNEN2|=0b1; //AUX
					if(sumpRX.command[1] & 0b1000)  CNEN2|=0b100000;
					if(sumpRX.command[1] & 0b100)   CNEN2|=0b1000000;
					if(sumpRX.command[1] & 0b10)  	CNEN2|=0b10000000;
					if(sumpRX.command[1] & 0b1) 	CNEN2|=0b100000000;
/*
				case SUMP_FLAGS:
					sumpPadBytes=0;//if user forgot to uncheck chan groups 2,3,4, we can send padding bytes
					if(sumpRX.command[1] & 0b100000) sumpPadBytes++;
					if(sumpRX.command[1] & 0b10000) sumpPadBytes++;
					if(sumpRX.command[1] & 0b1000) sumpPadBytes++;
					break;
*/
				case SUMP_CNT:
					sumpSamples=sumpRX.command[2];
					sumpSamples<<=8;
					sumpSamples|=sumpRX.command[1];
					sumpSamples=(sumpSamples+1)*4;
					//prevent buffer overruns
					if(sumpSamples>LA_SAMPLE_SIZE) sumpSamples=LA_SAMPLE_SIZE;
					break;
				case SUMP_DIV:
					l=sumpRX.command[3];
					l<<=8;
					l|=sumpRX.command[2];
					l<<=8;
					l|=sumpRX.command[1];

					//convert from SUMP 100MHz clock to our 16MIPs
					//l=((l+1)*16)/100;
					l=((l+1)*4)/25; 

					//adjust downwards a bit
					if(l>0x10)
						l-=0x10;
					else //fast as possible
						l=1;

					//setup PR register
					PR5=(l>>16);//most significant word
					PR4=l;//least significant word
					break;
			}

			sumpRXstate=C_IDLE;
			break;					
		}
	return 0;
}

//
//
//	To avoid rewriting interrupt vectors with the bootloader,
//  this firmware currently uses polling to read the trigger and timer
// 	A final version should use interrupts after lots of testing.
//
//
unsigned char SUMPlogicService(void){
	static unsigned int i;
//	static unsigned char j;

	switch(LAstate){//dump data
		case LA_ARMED: //check interrupt flags
			if(IFS1bits.CNIF==0){//no flags
				if(CNEN2) //if no trigger just continue
					break;
			}
	
			//else sample
			T4CONbits.TON=1;//start timer4
			IFS1bits.T5IF=0;//clear interrupt flag//setup timer and wait

			for(i=0;i<sumpSamples;i++){ //take SAMPLE_SIZE samples
				bpConfig.terminalInput[i]=(PORTB>>6); //change to pointer for faster use...
				while(IFS1bits.T5IF==0); //wait for timer4 (timer 5 interrupt)
				IFS1bits.T5IF=0;//clear interrupt flag
			}
			
			CNEN2=0;//change notice off
			T4CON=0; //stop count

			for(i=sumpSamples; i>0; i--){ //send back to SUMP, backwards
				UART1TX(bpConfig.terminalInput[(i-1)]);
				//for(j=0; j<sumpPadBytes; j++) UART1TX(0); //add padding if needed
			}

			SUMPreset();
			return 1;//done, exit SUMP
			//break;
		case LA_IDLE:
			break;
	}
	
	return 0; //not done, keep going

}


