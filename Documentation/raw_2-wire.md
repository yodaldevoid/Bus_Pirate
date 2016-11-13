Raw 2-Wire Mode
===================
The raw 2-wire library is similar to I2C, but it doesn’t handle acknowledge bits and it has bitwise pin control. I2C could be built using the basic elements in the raw2wire library. 

  - **Bus:** general purpose 2-wire library with bitwise pin control
  - **Connections:** two pins (SDA/SCL) and ground
  - **Output types:** 
    - 3.3volt normal output (high=3.3volts, low=ground)
    - [open collector](http://en.wikipedia.org/wiki/High_impedence) (high=Hi-Z, low=ground), [pull-up resistors](http://dangerousprototypes.com/docs/Practical_guide_to_Bus_Pirate_pull-up_resistors) required
  - **Pull-up resistors:** required for open collector output mode (2K – 10K)
  - **Maximum voltage:** 5.5volts (5 volt safe)
  - **Speed:** low (~5kHz), high (~50kHz), 100kHz, 400kHz

Starting Raw 2-Wire Mode
---------------------

    HiZ>m    <<<open the mode menu
    1. HiZ
    2. …
    6. 2WIRE
    …

    (1)>6    <<<choose 2-wire mode
    Set speed:
     1. ~5KHz
     2. ~50KHz
     3. ~100KHz
     4. ~400KHz

    (1)>2    <<<choose 50KHz speed
    Select output type:
     1. Open drain (H=Hi-Z, L=GND)
     2. Normal (H=3.3V, L=GND)

    (1)>1    <<<choose open drain output type
    Clutch disengaged!!!
    To finish setup, start up the power supplies with command 'W'

    Ready
    2WIRE>

Command Syntax
-------------------

  * A/a/@
	  * Toggle auxiliary pin.
	  * Capital "A" sets AUX high
	  * Small "a" sets to ground.
	  * @ sets aux to input (high impedance mode) and reads the pin value.
  * D/d
	  * Measure voltage on the ADC pin (v1+ hardware only).
  * W/w
	  * Capital 'W' enables the on-board power supplies.
	  * Small 'w' disables them. (v1+ hardware only).
  * { or [
	  * Issue I2C-style start condition.
  * ] or }
	  * Issue I2C-style stop condition.
  * r
	  * Read one byte. (r:1…255 for bulk reads)
  * 0b
	  * Write this binary value.
	  * Format is 0b00000000 for a byte, but partial bytes are also fine: 0b1001.
  * 0x
	  * Write this HEX value.
	  * Format is 0x01. Partial bytes are fine: 0xA. A-F can be lower-case or capital letters.
  * 0-255
	  * Write this decimal value.
	  * Any number not preceded by 0x or 0b is interpreted as a decimal value.
  * , space
	  * Value delimiter.
	  * Use a comma or space to separate numbers.
	  * Any combination is fine, no delimiter is required between non-number values: {0xa6,0, 0 16 5 0b111 0xaF}.
  * &
	  * Delay 1uS. (&:1…255 for multiple delays)
  * (#) 
	  * Run macro, (0) for macro list.
  * ^
      * One clock tick. (^:1…255 for multiple clock ticks)
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
| 1 | ISO7813-3 ATR for smart cards, parses reply bytes. |
| 2 | ISO7813-3 parse only (provide your own ATR command). |

Connections
------------------

| Bus Pirate | Dir. | Circuit | Description         |
| ----------:|:----:|:------- | ------------------- |
| MOSI       | ↔    | SDA     | Serial Data         |
| CLK        | →    | SCL     | Clock signal        |
| GND        | ⏚    | GND     | Signal Ground       |