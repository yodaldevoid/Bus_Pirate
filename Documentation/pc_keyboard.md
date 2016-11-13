PC Keyboard Mode
===================
This library interfaces PC keyboards. A PC keyboard operates at 5volts, and has it’s own internal pull-up resistors to 5volts. The keyboard issues a clock signal that drives all transactions, the library includes a time-out (v0h+) so the Bus Pirate won’t freeze if the keyboard doesn’t respond.

  - **Bus:** PC keyboard interface library
  - **Connections:** two pins (KBD/KBC) and ground
  - **Output types:** [open drain/open collector](http://en.wikipedia.org/wiki/High_impedence)
  - **Pull-up resistors:** _NONE_, keyboard has internal pull-ups
  - **Maximum voltage:** 5.5volts (5 volt safe)

Starting PC Keyboard Mode
---------------------

    HiZ>m    <<<open the mode menu
    1. HiZ
    2. …
    8. KEYB
    …

    (1)>8    <<<choose PC keyboard mode
    Clutch disengaged!!!
    To finish setup, start up the power supplies with command 'W'

    Ready
    KEYB>


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
  * r
	  * Read one byte with timeout. (r:1…255 for bulk reads)
  * 0b
	  * Write this binary value.
	  * Format is 0b00000000 for a byte, but partial bytes are also fine: 0b1001.
  * 0x
	  * Write this HEX value.
	  * Format is 0×01. Partial bytes are fine: 0xA. A-F can be lower-case or capital letters.
  * 0-255
	  * Write this decimal value.
	  * Any number not preceded by 0x or 0b is interpreted as a decimal value.
  * ,
	  * Value delimiter.
	  * Use a comma or space to separate numbers.
	  * Any combination is fine, no delimiter is required between non-number values: {0xa6,0, 0 16 5 0b111 0xaF}.
  * &
	  * Delay 1uS. (&:1…255 for multiple delays)
  * (#) 
	  * Run macro, (0) for macro list 

Macros
------------------

| (#) | Description |
|:---:| ----------- |
| 0 | Macro menu |
| 1 | Live keyboard monitor (v0h+). |


Connections
------------------

| Bus Pirate | Dir. | Circuit | Description   |
| ----------:|:----:|:------- | ------------- |
| ???        | ↔    | KBD     |               |
| ???        | →    | KBC     |               |
| GND        | ⏚    | GND     | Signal Ground |