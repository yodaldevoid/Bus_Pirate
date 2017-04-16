.global _bp_messages

_bp_messages:

	; BPMSG1001
	.pascii " *next clock (^) will use this value"

	; BPMSG1004
	.pascii "No device, try (ALARM) SEARCH macro first"

	; BPMSG1005
	.pascii "ADDRESS MACRO "

	; BPMSG1006
	.pascii " 0.Macro menu"

	; BPMSG1007
	.pascii "Macro     1WIRE address"

	; BPMSG1008
	.pascii "\r\n   *"

	; BPMSG1009
	.pascii "1WIRE ROM COMMAND MACROs:\r\n 51.READ ROM (0x33) *for single device bus\r\n 85.MATCH ROM (0x55) *followed by 64bit address\r\n 204.SKIP ROM (0xCC) *followed by command\r\n 236.ALARM SEARCH (0xEC)\r\n 240.SEARCH ROM (0xF0)"

	; BPMSG1010
	.pascii "ALARM SEARCH (0xEC)"

	; BPMSG1011
	.pascii "SEARCH (0xF0)"

	; BPMSG1012
	.pascii "Device IDs are available by MACRO, see (0)."

	; BPMSG1013
	.pascii "READ ROM (0x33): "

	; BPMSG1014
	.pascii "MATCH ROM (0x55)"

	; BPMSG1015
	.pascii "SKIP ROM (0xCC)"

	; BPMSG1016
	.pascii "Unknown macro, try ? or (0) for help"

	; BPMSG1017
	.pascii "BUS RESET "

	; BPMSG1019
	.pascii "Warning: "

	; BPMSG1020
	.pascii "*Short or no pull-up "

	; BPMSG1021
	.pascii "*No device detected "

	; BPMSG1022
	.pascii "DS18S20 High Pres Dig Therm"

	; BPMSG1023
	.pascii "DS18B20 Prog Res Dig Therm"

	; BPMSG1024
	.pascii "DS1822 Econ Dig Therm"

	; BPMSG1025
	.pascii "DS2404 Econram time Chip"

	; BPMSG1026
	.pascii "DS2431 1K EEPROM"

	; BPMSG1027
	.pascii "Unknown device"

	; BPMSG1028
	.pascii "PWM disabled"

	; BPMSG1029
	.pascii "1KHz-4,000KHz PWM"

	; BPMSG1030
	.pascii "Frequency in KHz "

	; BPMSG1033
	.pascii "Duty cycle in % "

	; BPMSG1034
	.pascii "PWM active"

	; BPMSG1037
	.pascii "ERROR: PWM active, g to disable"

	; BPMSG1038
	.pascii "AUX Frequency: "

	; BPMSG1039
	.pascii "AUX INPUT/HI-Z"

	; BPMSG1040
	.pascii "AUX HIGH"

	; BPMSG1041
	.pascii "AUX LOW"

	; BPMSG1042
	.pascii "VOLTMETER MODE"

	; BPMSG1044
	.pascii "VOLTAGE PROBE: "

	; BPMSG1045
	.pascii "V"

	; BPMSG1047
	.pascii "Error("

	; BPMSG1048
	.pascii ") @line:"

	; BPMSG1049
	.pascii " @pgmspace:"

	; BPMSG1050
	.pascii " bytes."

	; BPMSG1051
	.pascii "Too long!"

	; BPMSG1052
	.pascii "Syntax error"

	; BPMSG1053
	.pascii "No EEPROM"

	; BPMSG1054
	.pascii "Erasing"

	; BPMSG1055
	.pascii "done"

	; BPMSG1056
	.pascii "Saving to slot "

	; BPMSG1057
	.pascii "Invalid slot"

	; BPMSG1058
	.pascii "Loading from slot "

	; BPMSG1059
	.pascii "ERROR: command has no effect here"

	; BPMSG1064
	.pascii "I2C mode:\r\n 1. Software\r\n 2. Hardware"

	; BPMSG1065
	.pascii "Set speed:\r\n 1. ~5KHz\r\n 2. ~50KHz\r\n 3. ~100KHz\r\n 4. ~400KHz"

	; BPMSG1067
	.pascii "Set speed:\r\n 1. 100KHz\r\n 2. 400KHz\r\n 3. 1MHz"

	; BPMSG1068
	.pascii "I2C (mod spd)=( "

	; BPMSG1069
	.pascii " 0.Macro menu\r\n 1.7bit address search\r\n 2.I2C sniffer\r\n 3.Connect to on-board EEPROM\r\n 4.Enable Writing the on-board EEPROM"

	; BPMSG1070
	.pascii "Searching I2C address space. Found devices at:"

	; BPMSG1071
	.pascii "Sniffer"

	; BPMSG1072
	.pascii "Commandmode?\r\n1. 6b/14b\r\n2. 4b/16b"

	; BPMSG1073
	.pascii "Delay?"

	; BPMSG1074
	.pascii "PIC(mod dly)=("

	; BPMSG1075
	.pascii "CMD"

	; BPMSG1076
	.pascii "DTA"

	; BPMSG1077
	.pascii "no read"

	; BPMSG1078
	.pascii "unknown mode"

	; BPMSG1079
	.pascii "(1) get devID"

	; BPMSG1080
	.pascii "DevID = "

	; BPMSG1081
	.pascii " Rev = "

	; BPMSG1082
	.pascii "Not implemented (yet)"

	; BPMSG1083
	.pascii "Please exit PIC programming mode"

	; BPMSG1084
	.pascii "(BASIC)"

	; BPMSG1085
	.pascii "Ready"

	; BPMSG1086
	.pascii "a/A/@ controls AUX pin"

	; BPMSG1087
	.pascii "a/A/@ controls CS pin"

	; BPMSG1088
	.pascii "Command not used in this mode"

	; BPMSG1089
	.pascii "Pull-up resistors OFF"

	; BPMSG1091
	.pascii "Pull-up resistors ON"

	; BPMSG1092
	.pascii "Self-test in HiZ mode only"

	; BPMSG1093
	.pascii "RESET"

	; BPMSG1094
	.pascii "BOOTLOADER"

	; BPMSG1095
	.pascii "AUX INPUT/HI-Z, READ: "

	; BPMSG1096
	.pascii "POWER SUPPLIES ON"

	; BPMSG1097
	.pascii "POWER SUPPLIES OFF"

	; BPMSG1098
	.pascii "DATA STATE: "

	; BPMSG1099
	.pascii "DELAY "

	; BPMSG1100
	.pascii "us"

	; BPMSG1101
	.pascii "WRITE: "

	; BPMSG1102
	.pascii "READ: "

	; BPMSG1103
	.pascii "CLOCK, 1"

	; BPMSG1104
	.pascii "CLOCK, 0"

	; BPMSG1105
	.pascii "DATA OUTPUT, 1"

	; BPMSG1106
	.pascii "DATA OUTPUT, 0"

	; BPMSG1107
	.pascii " *pin is now HiZ"

	; BPMSG1108
	.pascii "CLOCK TICKS: "

	; BPMSG1109
	.pascii "READ BIT: "

	; BPMSG1110
	.pascii "Syntax error at char "

	; BPMSG1111
	.pascii "x. exit(without change)"

	; BPMSG1112
	.pascii "no mode change"

	; BPMSG1114
	.pascii "Nonexistent protocol!"

	; BPMSG1115
	.pascii "x. exit"

	; BPMSG1117
	.pascii "DEVID:"

	; BPMSG1118
	.pascii "http://dangerousprototypes.com"

	; BPMSG1119
	.pascii "*----------*"

	; BPMSG1120
	.pascii "Open drain outputs (H=Hi-Z, L=GND)"

	; BPMSG1121
	.pascii "Normal outputs (H=3.3v, L=GND)"

	; BPMSG1123
	.pascii "MSB set: MOST sig bit first"

	; BPMSG1124
	.pascii "LSB set: LEAST sig bit first"

	; BPMSG1127
	.pascii " 1. HEX\r\n 2. DEC\r\n 3. BIN\r\n 4. RAW"

	; BPMSG1128
	.pascii "Display format set"

	; BPMSG1133
	.pascii "Set serial port speed: (bps)\r\n 1. 300\r\n 2. 1200\r\n 3. 2400\r\n 4. 4800\r\n 5. 9600\r\n 6. 19200\r\n 7. 38400\r\n 8. 57600\r\n 9. 115200\r\n10. Input Custom BAUD\r\n11. Auto-Baud Detection (Activity Required)"

	; BPMSG1134
	.pascii "Adjust your terminal"

	; BPMSG1135
	.pascii "Are you sure? "

	; BPMSG1136
	.pascii "CFG1:"

	; BPMSG1137
	.pascii " CFG2:"

	; BPMSG1138
	.pascii "(\\-/_\\-)"

	; BPMSG1140
	.pascii "(\\_/-)"

	; BPMSG1142
	.pascii "Select output type:\r\n 1. Open drain (H=Hi-Z, L=GND)\r\n 2. Normal (H=3.3V, L=GND)"

	; BPMSG1143
	.pascii "R2W (spd hiz)=( "

	; BPMSG1144
	.pascii " 0.Macro menu\r\n 1.ISO7816-3 ATR\r\n 2.ISO7816-3 parse only"

	; BPMSG1145
	.pascii "ISO 7816-3 ATR (RESET on CS)\r\nRESET HIGH, CLOCK TICK, RESET LOW"

	; BPMSG1146
	.pascii "ISO 7816-3 reply (uses current LSB setting): "

	; BPMSG1147
	.pascii "Protocol: "

	; BPMSG1148
	.pascii "serial"

	; BPMSG1149
	.pascii "3 wire"

	; BPMSG1150
	.pascii "2 wire"

	; BPMSG1151
	.pascii "RFU"

	; BPMSG1152
	.pascii "unknown"

	; BPMSG1153
	.pascii "Read type: "

	; BPMSG1154
	.pascii "to end"

	; BPMSG1155
	.pascii "variable length"

	; BPMSG1156
	.pascii "Data units: "

	; BPMSG1157
	.pascii "no indication"

	; BPMSG1158
	.pascii "Data unit length (bits): "

	; BPMSG1159
	.pascii "CS ENABLED"

	; BPMSG1160
	.pascii "CS DISABLED"

	; BPMSG1161
	.pascii "R3W (spd csl hiz)=( "

	; BPMSG1162
	.pascii ")\r\n"

	; BPMSG1163
	.pascii "Disconnect any devices\r\nConnect (ADC to +3.3V)"

	; BPMSG1164
	.pascii "Ctrl"

	; BPMSG1165
	.pascii "AUX"

	; BPMSG1166
	.pascii "MODE LED"

	; BPMSG1167
	.pascii "PULLUP H"

	; BPMSG1168
	.pascii "PULLUP L"

	; BPMSG1169
	.pascii "VREG"

	; BPMSG1170
	.pascii "ADC and supply"

	; BPMSG1171
	.pascii "5V"

	; BPMSG1172
	.pascii "VPU"

	; BPMSG1173
	.pascii "3.3V"

	; BPMSG1174
	.pascii "ADC"

	; BPMSG1175
	.pascii "Bus high"

	; BPMSG1176
	.pascii "Bus Hi-Z 0"

	; BPMSG1177
	.pascii "Bus Hi-Z 1"

	; BPMSG1178
	.pascii "MODE, VREG, and USB LEDs should be on!"

	; BPMSG1179
	.pascii "Found "

	; BPMSG1180
	.pascii " errors."

	; BPMSG1181
	.pascii "MOSI"

	; BPMSG1182
	.pascii "CLK"

	; BPMSG1183
	.pascii "MISO"

	; BPMSG1184
	.pascii "CS"

	; BPMSG1185
	.pascii " OK"

	; BPMSG1186
	.pascii " FAIL"

	; BPMSG1187
	.pascii "Set speed:\r\n 1.  30KHz\r\n 2. 125KHz\r\n 3. 250KHz\r\n 4.   1MHz\r\n 5.  50KHz\r\n 6. 1.3MHz\r\n 7.   2MHz\r\n 8. 2.6MHz\r\n 9. 3.2MHz\r\n10.   4MHz\r\n11. 5.3MHz\r\n12.   8MHz"

	; BPMSG1188
	.pascii "Clock polarity:\r\n 1. Idle low *default\r\n 2. Idle high"

	; BPMSG1189
	.pascii "Output clock edge:\r\n 1. Idle to active\r\n 2. Active to idle *default"

	; BPMSG1190
	.pascii "Input sample phase:\r\n 1. Middle *default\r\n 2. End"

	; BPMSG1191
	.pascii "SPI (spd ckp ske smp csl hiz)=( "

	; BPMSG1192
	.pascii " 0.Macro menu\r\n 1.Sniff CS low\r\n 2.Sniff all traffic"

	; BPMSG1194
	.pascii "-p "

	; BPMSG1195
	.pascii "-f "

	; BPMSG1196
	.pascii "*Bytes dropped*"

	; BPMSG1197
	.pascii "FAILED, NO DATA"

	; BPMSG1199
	.pascii "Data bits and parity:\r\n 1. 8, NONE *default \r\n 2. 8, EVEN \r\n 3. 8, ODD \r\n 4. 9, NONE"

	; BPMSG1200
	.pascii "Stop bits:\r\n 1. 1 *default\r\n 2. 2"

	; BPMSG1201
	.pascii "Receive polarity:\r\n 1. Idle 1 *default\r\n 2. Idle 0"

	; BPMSG1202
	.pascii "UART (spd brg dbp sb rxp hiz)=( "

	; BPMSG1203
	.pascii " 0.Macro menu\r\n 1.Transparent bridge\r\n 2.Live monitor\r\n 3.Bridge with flow control\n\r 4.Auto Baud Detection (Activity Needed)"

	; BPMSG1204
	.pascii "UART bridge"

	; BPMSG1206
	.pascii "Raw UART input"

	; BPMSG1207
	.pascii "UART LIVE DISPLAY, } TO STOP"

	; BPMSG1208
	.pascii "LIVE DISPLAY STOPPED"

	; BPMSG1209
	.pascii "WARNING: pins not open drain (HiZ)"

	; BPMSG1210
	.pascii " REVID:"

	; BPMSG1211
	.pascii "\r\nInvalid choice, try again"

	; BPMSG1212
	.pascii "ms"

	; BPMSG1213
	.pascii "RS LOW, COMMAND MODE"

	; BPMSG1214
	.pascii "RS HIGH, DATA MODE"

	; BPMSG1216
	.pascii "This mode requires an adapter"

	; BPMSG1219
	.pascii " 0.Macro menu\r\n 1.LCD Reset\r\n 2.Init LCD\r\n 3.Clear LCD\r\n 4.Cursor position ex:(4) 0\r\n 6.Write test numbers ex:(6) 80\r\n 7.Write test characters ex:(7) 80"

	; BPMSG1220
	.pascii "Display lines:\r\n 1. 1 \r\n 2. Multiple"

	; BPMSG1221
	.pascii "INIT"

	; BPMSG1222
	.pascii "CLEAR"

	; BPMSG1223
	.pascii "CURSOR SET"

	; BPMSG1226
	.pascii "Pinstates:"

	; BPMSG1228
	.pascii "P\tP\tP\tI\tI\t"

	; BPMSG1234
	.pascii "GND\t"

	; BPMSG1237
	.pascii " TIMEOUT"

	; BPMSG1238
	.pascii " 0. Macro menu\r\n 1. Live input monitor"

	; BPMSG1239
	.pascii "Input monitor, any key exits"

	; BPMSG1240
	.pascii " *startbit error"

	; BPMSG1241
	.pascii " *parity error"

	; BPMSG1242
	.pascii " *stopbit error"

	; BPMSG1243
	.pascii " NONE"

	; BPMSG1244
	.pascii " UNKNOWN ERROR"

	; BPMSG1245
	.pascii " autorange "

	; BPMSG1248
	.pascii "Input a custom BAUD rate:"

	; BPMSG1250
	.pascii "Any key to exit"

	; BPMSG1251
	.pascii "Space to continue"

	; BPMSG1252
	.pascii "Number of bits read/write: "

	; BPMSG1253
	.pascii "CS:\r\n 1. CS\r\n 2. /CS *default"

	; BPMSG1254
	.pascii "Position in degrees"

	; BPMSG1255
	.pascii "Servo active"

	; BPMSG1256
	.pascii "#12    \t#11    \t#10    \t#09   \t#08   \t#07   \t#06   \t#05   \t#04   \t#03   \t#02   \t#01   "

	; BPMSG1257
	.pascii "GND\t5.0V\t3.3V\tVPU\tADC\tAUX2\tAUX1\tAUX\t"

	; BPMSG1259
	.pascii "-\t-\t-\tOWD"

	; BPMSG1262
	.pascii "-\t-\tPGC\tPGD"

	; BPMSG1263
	.pascii "a/A/@ controls AUX1 pin"

	; BPMSG1264
	.pascii "a/A/@ controls AUX2 pin"

	; BPMSG1265
	.pascii "EEPROM"

	; BPMSG1266
	.pascii "SCL"

	; BPMSG1267
	.pascii "SDA"

	; BPMSG1268
	.pascii "WP"

	; BPMSG1269
	.pascii "READ&WRITE"

	; BPMSG1270
	.pascii "Vusb"

	; BPMSG1271
	.pascii "Select Vpu (Pullup) Source:\r\n 1) External (or None)\r\n 2) Onboard 3.3v\r\n 3) Onboard 5.0v"

	; BPMSG1272
	.pascii " on-board pullup voltage "

	; BPMSG1273
	.pascii "enabled"

	; BPMSG1274
	.pascii "disabled"

	; BPMSG1280
	.pascii "Waiting activity..."

	; BPMSG1281
	.pascii "** Early Exit!"

	; BPMSG1282
	.pascii "** Baud>16m: The BP cannot measure above 16000000, Done."

	; BPMSG1283
	.pascii "\n\rCalculated: \t"

	; BPMSG1284
	.pascii "\n\rEstimated:  \t"

	; BPMSG1285
	.pascii " bps"

	; HLP1000
	.pascii "General\t\t\t\t\tProtocol interaction"

	; HLP1001
	.pascii "---------------------------------------------------------------------------"

	; HLP1002
	.pascii "?\tThis help\t\t\t(0)\tList current macros"

	; HLP1003
	.pascii "=X/|X\tConverts X/reverse X\t\t(x)\tMacro x"

	; HLP1004
	.pascii "~\tSelftest\t\t\t[\tStart"

	; HLP1005
	.pascii "o\tSet output type\t\t\t]\tStop"

	; HLP1006
	.pascii "$\tJump to bootloader\t\t{\tStart with read"

	; HLP1007
	.pascii "&/%\tDelay 1 us/ms\t\t\t}\tStop"

	; HLP1008
	.pascii "a/A/@\tAUXPIN (low/HI/READ)\t\t\"abc\"\tSend string"

	; HLP1009
	.pascii "b\tSet baudrate\t\t\t123\tSend integer value"

	; HLP1010
	.pascii "c/C/k/K\tAUX assignment (A0/CS/A1/A2)\t0x123\tSend hex value"

	; HLP1011
	.pascii "d/D\tMeasure ADC (once/CONT.)\t0b110\tSend binary value"

	; HLP1012
	.pascii "f\tMeasure frequency\t\tr\tRead"

	; HLP1013
	.pascii "g/S\tGenerate PWM/Servo\t\t/\tCLK hi"

	; HLP1014
	.pascii "h\tCommandhistory\t\t\t\\\tCLK lo"

	; HLP1015
	.pascii "i\tVersioninfo/statusinfo\t\t^\tCLK tick"

	; HLP1016
	.pascii "l/L\tBitorder (msb/LSB)\t\t-\tDAT hi"

	; HLP1017
	.pascii "m\tChange mode\t\t\t_\tDAT lo"

	; HLP1018
	.pascii "e\tSet Pullup Method\t\t.\tDAT read"

	; HLP1019
	.pascii "p/P\tPullup resistors (off/ON)\t!\tBit read"

	; HLP1020
	.pascii "s\tScript engine\t\t\t:\tRepeat e.g. r:10"

	; HLP1021
	.pascii "v\tShow volts/states\t\t;\tBits to read/write e.g. 0x55;2"

	; HLP1022
	.pascii "w/W\tPSU (off/ON)\t\t<x>/<x= >/<0>\tUsermacro x/assign x/list all"

	; MSG_1WIRE_MODE_IDENTIFIER
	.pascii "1W01"

	; MSG_1WIRE_SPEED_PROMPT
	.pascii "Set speed:\r\n 1. Standard (~16.3kbps) \r\n 2. Overdrive (~160kps)"

	; MSG_ACK
	.pascii "ACK"

	; MSG_BASE_CONVERTER_EQUAL_SIGN
	.pascii " = "

	; MSG_BAUD_DETECTION_SELECTED
	.pascii "Baud detection selected.."

	; MSG_BBIO_MODE_IDENTIFIER
	.pascii "BBIO1"

	; MSG_BINARY_NUMBER_PREFIX
	.pascii "0b"

	; MSG_CFG0_FIELD
	.pascii "CFG0: "

	; MSG_CHIP_REVISION_A3
	.pascii "A3"

	; MSG_CHIP_REVISION_A5
	.pascii "A5"

	; MSG_CHIP_REVISION_ID_BEGIN
	.pascii " (24FJ256GB106 "

	; MSG_CHIP_REVISION_UNKNOWN
	.pascii "UNK"

	; MSG_CLUTCH_DISENGAGED
	.pascii "Clutch disengaged!!!"

	; MSG_CLUTCH_ENGAGED
	.pascii "Clutch engaged!!!"

	; MSG_FINISH_SETUP_PROMPT
	.pascii "To finish setup, start up the power supplies with command 'W'"

	; MSG_HEXADECIMAL_NUMBER_PREFIX
	.pascii "0x"

	; MSG_I2C_MODE_IDENTIFIER
	.pascii "I2C1"

	; MSG_I2C_PINS_STATE
	.pascii "-\t-\tSCL\tSDA"

	; MSG_I2C_READ_ADDRESS_END
	.pascii " R) "

	; MSG_I2C_START_BIT
	.pascii "I2C START BIT"

	; MSG_I2C_STOP_BIT
	.pascii "I2C STOP BIT"

	; MSG_I2C_WRITE_ADDRESS_END
	.pascii " W) "

	; MSG_NACK
	.pascii "NACK"

	; MSG_NO_VOLTAGE_ON_PULLUP_PIN
	.pascii "Warning: no voltage on Vpullup pin"

	; MSG_ONBOARD_I2C_EEPROM_WRITE_PROTECT_DISABLED
	.pascii "On-board EEPROM write protect disabled"

	; MSG_PIC_MODE_IDENTIFIER
	.pascii "PIC1"

	; MSG_PIC_UNKNOWN_MODE
	.pascii "unknown mode"

	; MSG_PWM_FREQUENCY_TOO_LOW
	.pascii "Frequencies < 1Hz are not supported."

	; MSG_PWM_HZ_MARKER
	.pascii " Hz"

	; MSG_RAW_BRG_VALUE_INPUT
	.pascii "Enter raw value for BRG"

	; MSG_RAW_MODE_IDENTIFIER
	.pascii "RAW1"

	; MSG_RESET_MESSAGE
	.pascii "RESET"

	; MSG_SPI_COULD_NOT_KEEP_UP
	.pascii "Couldn't keep up"

	; MSG_SPI_MODE_IDENTIFIER
	.pascii "SPI1"

	; MSG_SPI_PINS_STATE
	.pascii "CS\tMISO\tCLK\tMOSI"

	; MSG_UART_MODE_IDENTIFIER
	.pascii "ART1"

	; MSG_UART_NORMAL_TO_EXIT
	.pascii "Normal to exit"

	; MSG_UART_PINS_STATE
	.pascii "-\tRxD\t-\tTxD"

	; MSG_USING_ONBOARD_I2C_EEPROM
	.pascii "Now using on-board EEPROM I2C interface"

	; MSG_VOLTAGE_VPULLUP_ALREADY_PRESENT
	.pascii "Warning: already a voltage on Vpullup pin"

	; MSG_VPU_3V3_MARKER
	.pascii "Vpu=3V3, "

	; MSG_VPU_5V_MARKER
	.pascii "Vpu=5V, "

	; MSG_VREG_TOO_LOW
	.pascii "VREG too low, is there a short?"

	; MSG_XSV1_MODE_IDENTIFIER
	.pascii "XSV1"

