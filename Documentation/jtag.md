JTAG Mode
===================

  - **Bus:** [JTAG](https://en.wikipedia.org/wiki/Joint_Test_Action_Group)
  - **Connections:** 4 connections (TDI, TCK, TDO, TMS) and ground
  - **Output types:** 
    - 3.3volt normal output (high=3.3volts, low=ground)
    - [open collector](http://en.wikipedia.org/wiki/High_impedance) (high=Hi-Z, low=ground), [pull-up resistors](http://dangerousprototypes.com/docs/Practical_guide_to_Bus_Pirate_pull-up_resistors) required. Use open drain/open collector output types with pull-up resistors for multi-voltage interfacing.
  - **Pull-up resistors:** required for open collector output mode (2K – 10K)
  - **Maximum voltage:** 5.5volts (5volt safe)


Starting JTAG Mode
---------------------
    TODO: // this doesn't exist on my bp hw v3.6, fw v7.0


Command Syntax
-------------------

  * A/a/@
	  * Toggle auxiliary pin.
	  * Capital "A" sets AUX high.
	  * Small "a" sets to ground.
	  * @ sets aux to input (high impedance mode) and reads the pin value.
  * D/d (v1+ hardware only)
	  * Measure voltage on the ADC pin.
  * W/w (v1+ hardware only)
	  * Capital 'W' enables the on-board power supplies.
	  * Small 'w' disables them.
  * [
      * Move JTAG state machine (SM) to INSTRUCTION register. Last bit of byte writes is delayed until leaving the INSTRUCTION register. 
  * {
	  * Move JTAG SM to DATA register.
  * ] or }
	  * Move JTAG SM to IDLE register.
  * r
	  * Read one byte. (r:1…255 for bulk reads)
  * 0b
	  * Write this binary value.
	  * Format is 0b00000000 for a byte, but partial bytes are also fine: 0b1001.
  * 0x
	  * Write this HEX value.
	  * Format is 0x01. Partial bytes are fine: 0xA. A-F can be lower-case or capital letters.
  * 0-255
	  * Write this decimal value, check ACK.
	  * Any number not preceded by 0x or 0b is interpreted as a decimal value.
  * , space
	  * Value delimiter.
	  * Use a comma or space to separate numbers.
	  * Any combination is fine, no delimiter is required between non-number values: {0xa6,0, 0 16 5 0b111 0xaF}.
  * &
	  * Delay 1uS. (&:1…255 for multiple delays)
  * (#) 
	  * Run macro, (0) for macro list
  * ^
      * Send one clock tick. (^:1…255 for multiple clock ticks)
  * / or \
      * Toggle clock level high (/) and low (\). Includes clock delay (100uS).
  * - or _
      * Toggle data state high (-) and low (_). Includes data setup delay (20uS).
  * !
      * Read one bit with clock.
  * .
      * Read data pin state (no clock).

Macros
------------------

| (#) | Description |
|:---:| ----------- |
| 0 | Macro menu |
| 1 | Reset chain. |
| 2 | Probe chain. |
| 3 | ~~XSVF player. JTAG programmer.~~ (deprecated) _TODO: VERIFY_ |

Connections
------------------

| Bus Pirate | Dir. | Circuit | Description            |
| ----------:|:----:|:------- | ---------------------- |
| MOSI       | →    | TDI     | Master Out, Slave In   |
| MISO       | ←    | TDO     | Master In, Slave Out   |
| CS         | →    | TMS     | Chip Select            |
| CLK        | →    | TCK     | Clock signal           |
| AUX0       | →    | SRST    | System reset           |
| AUX1       | →    | TRST    | Target reset (v4 only) |
| PGD        | →    | TRST    | Target reset (v3 only) |
| GND        | ⏚    | GND     | Signal Ground          |
