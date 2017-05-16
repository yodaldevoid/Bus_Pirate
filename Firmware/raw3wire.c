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

#include "raw3wire.h"

#ifdef BP_ENABLE_RAW_3WIRE_SUPPORT

#include "base.h"
#include "bitbang.h"
#include "proc_menu.h"		// for the userinteraction subs

#define R3WMOSI_TRIS 	BP_MOSI_DIR
#define R3WCLK_TRIS 	BP_CLK_DIR
#define R3WMISO_TRIS 	BP_MISO_DIR
#define R3WCS_TRIS 		BP_CS_DIR


#define R3WMOSI 		BP_MOSI
#define R3WCLK 			BP_CLK 
#define R3WMISO 		BP_MISO 
#define R3WCS 			BP_CS 

extern mode_configuration_t mode_configuration;
extern command_t last_command;
extern bool command_error;

void R3Wsetup_exc(void);

struct _R3W{
//	unsigned char wwr:1;
	unsigned char csl:1;
} r3wSettings;


unsigned int R3Wread(void)
{	return (bitbang_read_with_write(0xff));
}

unsigned int R3Wwrite(unsigned int c)
{	c=bitbang_read_with_write(c);
	if(mode_configuration.write_with_read==1)
	{	return c;
	}
	return 0x00;
}

void R3Wstartr(void)
{	mode_configuration.write_with_read=1;
	if(r3wSettings.csl)
	{	bitbang_set_cs(0);
	}
	else
	{	bitbang_set_cs(1);
	}
	//bpWmessage(MSG_CS_ENABLED);
	if(r3wSettings.csl) UART1TX('/');
	BPMSG1159;
}
void R3Wstart(void)
{	mode_configuration.write_with_read=0;
	if(r3wSettings.csl)
	{	bitbang_set_cs(0);
	}
	else
	{	bitbang_set_cs(1);
	}
	//bpWmessage(MSG_CS_ENABLED);
	if(r3wSettings.csl) UART1TX('/');
	BPMSG1159;
}
void R3Wstop(void)
{	mode_configuration.write_with_read=0;
	if(r3wSettings.csl)
	{	bitbang_set_cs(1);
	}
	else
	{	bitbang_set_cs(0);
	}
	//bpWmessage(MSG_CS_DISABLED);
	if(r3wSettings.csl) UART1TX('/');
	BPMSG1160;
}

void R3Wsettings(void) {
    //bpWstring("R3W (spd hiz)=( ");
	BPMSG1161;
	bp_write_dec_byte(mode_configuration.speed); bpSP;
	bp_write_dec_byte(r3wSettings.csl); bpSP;
	bp_write_dec_byte(mode_configuration.high_impedance); bpSP;
	bp_write_line(")");
}


void R3Wsetup(void)
{	int speed, output, cslow;

	consumewhitechars();
	speed=getint();
	consumewhitechars();
	cslow=getint();
	consumewhitechars();
	output=getint();

	if((speed>0)&&(speed<=4))
	{	mode_configuration.speed=speed-1;
	}
	else	
	{	speed=0;					// when speed is 0 we ask the user
	}
	if((cslow>0)&&(cslow<=2))
	{	r3wSettings.csl=(cslow-1);
	}
	else	
	{	speed=0;					// when speed is 0 we ask the user
	}
	if((output>0)&&(output<=2))
	{	mode_configuration.high_impedance=(~(output-1));
	}
	else	
	{	speed=0;					// when speed is 0 we ask the user
	}

	if(speed==0)
	{	//bpWmessage(MSG_OPT_BB_SPEED);
		BPMSG1065;
		mode_configuration.speed=(getnumber(1,1,4,0)-1);

		//bpWline("CS:\r\n 1. CS\r\n 2. /CS *default");
		BPMSG1253;
		r3wSettings.csl=getnumber(2,1,2,0)-1;

		//bpWmessage(MSG_OPT_OUTPUT_TYPE);
		BPMSG1142;
		mode_configuration.high_impedance=(~(getnumber(1,1,2,0)-1));
		command_error=false;
	}
	else
	{	R3Wsettings();
	}

	//reset the write with read variable
	mode_configuration.write_with_read=0;
	mode_configuration.int16=0; //8 bit
}
void R3Wsetup_exc(void)
{
    bitbang_setup(3, mode_configuration.speed); //setup the bitbang library, must be done before calling bbCS below
	//setup pins (pins are input/low when we start)
	//MOSI output, low
	//clock output, low
	//MISO input
	//CS output, high
	R3WMOSI_TRIS=0;
	R3WCLK_TRIS=0;
	R3WMISO_TRIS=1;

	// set cs the way the user wants
	bitbang_set_cs(r3wSettings.csl);//takes care of custom HiZ settings too
}    

void R3Wpins(void) {
    MSG_SPI_PINS_STATE;
}

#endif /* BP_ENABLE_RAW_3WIRE_SUPPORT */