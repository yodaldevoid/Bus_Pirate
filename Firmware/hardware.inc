;
; This file is part of the Bus Pirate project
; (https://github.com/BusPirate/Firmware).
;
; Written and maintained by the Bus Pirate project.
;
; To the extent possible under law, the project has waived all copyright and
; related or neighboring rights to Bus Pirate. This work is published from
; United States.
;
; For details see: http://creativecommons.org/publicdomain/zero/1.0/.
;
; This program is distributed in the hope that it will be useful, but WITHOUT
; ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
; FOR A PARTICULAR PURPOSE.
;
    
.ifdef __24FJ256GB106
.include "p24FJ256GB106.inc"
	.equ	CONFIGURED,	1
	.equ	BUSPIRATEV4,	1
.endif ; __24FJ256GB106

.ifdef __24FJ64GA002
	.include "p24FJ64GA002.inc"
	.equ	CONFIGURED,	1
	.equ	BUSPIRATEV3,	1
.endif ; __24FJ64GA002
	
.ifndef CONFIGURED
	.error "Not configured?"
.endif ; CONFIGURED

.ifdef BUSPIRATEV3

.endif ; BUSPIRATEV3

.ifdef BUSPIRATEV4
	
	.equ	AUX2,		#0b00000001
	.equ	MOSI,		#0b00000010
	.equ	CLK,		#0b00000100
	.equ	MISO,		#0b00001000
	.equ	CS,		#0b00010000
	.equ	AUX0,		#0b10000000
	
	.equ	IOPOR,		PORTD
	.equ	IOLAT,	    

#define IOPOR                   PORTD
#define IOLAT                   LATD
#define IODIR                   TRISD

/**
 * Bitmask for all available the I/O pins.
 */
#define ALLIO                   (MOSI + CLK + MISO + CS)
#define AUX                     AUX0

//these macros set the pin IO used in each module
#define BP_MOSI                 PORTDbits.RD1
#define BP_CLK                  PORTDbits.RD2
#define BP_MISO                 PORTDbits.RD3
#define BP_CS                   PORTDbits.RD4
#define BP_AUX0                 PORTDbits.RD5 //moved pin5 (AUX) to RB10
#define BP_LEDMODE              PORTBbits.RB8 //single MODE led on v2a
#define BP_VREGEN               PORTBbits.RB9 //single vreg enable pin on v2a
#define BP_PULLUP               PORTEbits.RE4 //Pull-up pin on V2a
#define BP_PGD                  PORTBbits.RB7 //PGD pin on programming header

//new in v4
#define BP_LEDUSB               PORTBbits.RB10
#define BP_BUTTON               PORTCbits.RC14
#define BP_AUX1                 PORTDbits.RD8 
#define BP_AUX2                 PORTDbits.RD0 
#define BP_EE_SDA               PORTDbits.RD9 
#define BP_EE_SCL               PORTDbits.RD10 
#define BP_EE_WP                PORTDbits.RD11
#define BP_PUVSEL33             PORTFbits.RF4
#define BP_PUVSEL50             PORTFbits.RF5

.endif ; BUSPIRATEV4
