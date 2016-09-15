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

#define BELL	0x07

#define CMDLENMSK	(BP_COMMAND_BUFFER_SIZE-1)

extern char cmdbuf[BP_COMMAND_BUFFER_SIZE];
extern unsigned int cmdend;
extern unsigned int cmdstart;
extern bool command_error;

//prints version, used internally and in main.c
void print_version_info(void); 

void serviceuser(void);

int getint(void);
int getrepeat(void);
int getnumbits(void);
unsigned char changeReadDisplay(void);
void consumewhitechars(void);
void changemode(void);
#ifdef BP_ENABLE_COMMAND_HISTORY
int cmdhistory(void);
#endif /* BP_ENABLE_COMMAND_HISTORY */

int getnumber(int def, int min, int max, int x);	
#if defined(BUSPIRATEV4)
long getlong(long def, int min, long max, int x);
#endif /* BUSPIRATEV4 */

