#if defined(useCPUreading) || defined(useDebugCPUreading)
namespace systemInfo /* CPU loading and RAM availability support section prototype */
{

	static void idleProcess(void);
#if defined(useCPUreading)
	static uint8_t displayHandler(uint8_t cmd, uint8_t cursorPos);
	static void showCPUload(void);
	static void showCPUloading(void);
#endif // defined(useCPUreading)

};

static uint32_t mainStart;
static uint32_t idleTimerLength;

#if defined(useCPUreading)
extern char __bss_end;
extern char *__brkval;

#endif // defined(useCPUreading)
#if defined(useDebugCPUreading)
static uint8_t monitorState;
static uint32_t idleProcessTimerLength;
static uint32_t displayTimerLength;
static uint32_t SWEET64timerLength;

#endif // defined(useDebugCPUreading)
#endif // defined(useCPUreading) || defined(useDebugCPUreading)
#if defined(useSimulatedFIandVSS)
namespace signalSim /* VSS / fuel injector on-board simulator support section prototype */
{

#if defined(useButtonInput)
	static uint8_t displayHandler(uint8_t cmd, uint8_t cursorPos);
	static uint16_t getSignalSimPageFormats(uint8_t formatIdx);
#endif // defined(useButtonInput)
	static void configurePorts(void);
	static void idleProcessFuel(void);
	static void idleProcessVSS(void);

}

static const uint8_t debugVSSflag =				0b00000001;
static const uint8_t debugInjectorFlag =		0b00000010;
static const uint8_t debugFIPready =			0b00000100;
static const uint8_t debugVSSready =			0b00001000;

static const uint8_t debugEnableFlags =			debugVSSflag | debugInjectorFlag;
static const uint8_t debugVSreadyFlags =		debugVSSready | debugVSSflag;
static const uint8_t debugFIreadyFlags =		debugFIPready | debugInjectorFlag;

static const unsigned long debugVSSresetLength = (unsigned long)(ceil)((2ul * F_CPU) / (2ul * 255ul)) - 1; // 2 sec
static const unsigned long debugFIPresetLength = (unsigned long)(ceil)((4ul * F_CPU) / (3ul * 2ul * 255ul)) - 1; // 4/3 sec

#if defined(useButtonInput)
static const uint16_t signalSimPageFormats[4] PROGMEM = {
	 (instantIdx << 8 ) |		(tInjectorTotalTime) 		// Debug
	,(instantIdx << 8 ) |		(tVSStotalTime)
	,(instantIdx << 8 ) |		(tInjectorPulseCount)
	,(instantIdx << 8 ) |		(tVSSpulseCount)
};

static const char debugScreenFuncNames[] PROGMEM = {
	"FI ON   VSS ON" tcEOS
	"FI OFF  VSS ON" tcEOS
	"FI OFF  VSS OFF" tcEOS
	"FI ON   VSS OFF" tcEOS
};

#endif // defined(useButtonInput)
static const uint16_t debugVSSvalues[] PROGMEM = {
 65535
,2258
,1128
,752
,564
,451
,375
,322
,281
,250
,225
,204
,187
,173
,160
,150
,140
,132
,124
,118
,112
,107
,102
,97
,93
,89
,86
,83
,80
,77
,74
,72
,70
,67
,65
,64
,62
,60
,58
,57
,55
,54
,53
,52
,50
,49
,48
,47
,46
,45
,44
};

static const uint16_t debugFIPvalues[] PROGMEM = {
 313
,326
,341
,358
,375
,395
,417
,442
,470
,501
,537
,578
,626
,683
,752
,836
,940
,1075
,1254
,1505
,1881
,2509
,3764
,7528
,18823
,65535
};

static const uint16_t debugFIPWvalues[] PROGMEM = {
 22
,22
,23
,23
,23
,24
,24
,24
,25
,26
,26
,27
,28
,29
,30
,32
,34
,36
,39
,44
,51
,63
,86
,156
,365
,0
};

static const uint8_t debugVSSlength = ( sizeof(debugVSSvalues) / sizeof(uint16_t) );
static const uint8_t debugFIPlength = ( sizeof(debugFIPvalues) / sizeof(uint16_t) );

static volatile uint16_t debugVSStickLength;
static volatile uint16_t debugFIPtickLength;
static volatile uint16_t debugFIPWtickLength;

static volatile uint16_t debugVSScount;
static volatile uint16_t debugFIPcount;
static volatile uint16_t debugFIPWcount;

static volatile uint8_t debugFlags;

static uint8_t debugVSSidx;
static uint8_t debugFIPidx;

static uint8_t debugVSSstate;
static uint8_t debugFIPstate;

static uint16_t debugFIPWgoodTickLength; // to force debug injector pulse width to a maximum good engine speed-dependent value
static uint16_t debugFIPWreadTickLength;

#endif // defined(useSimulatedFIandVSS)
#if defined(useDebugTerminal)
namespace terminal /* debug terminal section prototype */
{

	static void mainProcess(void);
	static void outputFlags(uint8_t flagRegister, const char * flagStr);
	static void outputTripFunctionValue(uint8_t lineNumber);
	static void outputConstantValue(uint8_t lineNumber);
	static void outputParameterValue(uint8_t lineNumber);
	static void outputParameterExtra(uint8_t lineNumber);
	static void outputVolatileValue(uint8_t lineNumber);
	static void outputMainProgramValue(uint8_t lineNumber);
	static void outputTripVarMeasuredValue(uint8_t lineNumber);
	static void outputTripVarMeasuredExtra(uint8_t lineNumber);
	static void outputDecimalValue(uint8_t lineNumber);
	static void outputDecimalExtra(uint8_t lineNumber);
	static void processMath(uint8_t cmd);
#if defined(useBluetoothAdaFruitSPI)
	static void outputBluetoothResponse(void);
#endif // defined(useBluetoothAdaFruitSPI)

}

static const uint8_t tmByteReadIn =			0x80;
static const uint8_t tmSourceReadIn =		0x40;
static const uint8_t tmTargetReadIn =		0x20;
static const uint8_t tmAddressReadIn =		0x10;
static const uint8_t tmHexInput =			0x08;
static const uint8_t tmDecimalInput =		0x04;
static const uint8_t tmButtonInput =		0x02;
static const uint8_t tmInitInput =			0x01;

static const uint8_t tmInputMask =			(tmHexInput | tmDecimalInput | tmButtonInput | tmInitInput);
static const uint8_t tmInitHex =			(tmInitInput | tmHexInput);
static const uint8_t tmInitDecimal =		(tmInitInput | tmDecimalInput);
static const uint8_t tmInitButton =			(tmInitInput | tmButtonInput);

#if defined(useBluetoothAdaFruitSPI)
static uint8_t tmOutputBluetooth;

#endif // defined(useBluetoothAdaFruitSPI)
#if defined(useDebugTerminalHelp)
static const char terminalHelp[] PROGMEM = {
	"    [y].[x]P - list stored parameters, optionally between [y] and [x]" tcEOSCR
	"    [y].[x]V - list volatile variables, optionally between [y] and [x]" tcEOSCR
	"    [y].[x]M - list main program variables, optionally between [y] and [x]" tcEOSCR
	"    [y].[x]T - list terminal trip variable values, optionally between [y] and [x]" tcEOSCR
	"    [y].[x]O - list program constants, optionally between [y] and [x]" tcEOSCR
	"[z]<[y].[x]L - list terminal trip variable function outputs, optionally between [y] and [x]" tcEOSCR
	"                [z] - decimal window length (optional)" tcEOSCR
	"[z]<[y].[x]U - list decimal number sample for output" tcEOSCR
	"                [z] - decimal window length (optional)" tcEOSCR
	"                [y] - decimal digit count (optional)" tcEOSCR
	"                [x] - decimal processing flag (optional)" tcEOSCR
#if defined(useSWEET64trace)
	"              x^L - list SWEET64 source code for trip function" tcEOSCR
#endif // defined(useSWEET64trace)
	"   [z]<[y].x - enters a number x into the 64-bit math accumulator" tcEOSCR
	"                [z] - decimal window length (optional)" tcEOSCR
	"                [y] - decimal digit count (optional)" tcEOSCR
	"          +x - adds x to math accumulator" tcEOSCR
	"          -x - subtracts x from math accumulator" tcEOSCR
	"          *x - multiplies math accumulator by x" tcEOSCR
#if defined(useIsqrt)
	"          |  - finds square root of math accumulator" tcEOSCR
#endif // defined(useIsqrt)
	"          /x - divides math accumulator by x" tcEOSCR
	"          =x - enters a number x into the 64-bit math accumulator" tcCR tcEOSCR
	"  x:Py [y] [y]... - store one or more y values, starting at stored parameter x" tcEOSCR
	"  x:Vy [y] [y]... - store one or more y values, starting at volatile variable x" tcEOSCR
	"  x:My [y] [y]... - store one or more y values, starting at main program variable x" tcEOSCR
	"  x:Ty [y] [y]... - store one or more y values, starting at terminal trip variable x" tcCR tcEOSCR
	"    [y]<[x]R - read trip variable x into trip variable y" tcEOSCR
	"                default for x and y is terminal trip variable" tcEOSCR
	"                if no x or y specified, lists available trip variables" tcEOSCR
#if defined(useDebugButtonInjection)
	"           I - inject button press" tcEOSCR
#if defined(useLegacyButtons)
	"                short (l, c, r)" tcEOSCR
	"                 long (L, C, R)" tcCR tcEOSCR
#else // defined(useLegacyButtons)
	"                short (l, c, r, u, d)" tcEOSCR
	"                 long (L, C, R, U, D)" tcCR tcEOSCR
#endif // defined(useLegacyButtons)
#endif // defined(useDebugButtonInjection)
#if defined(useBluetoothAdaFruitSPI)
	"           Y - sends the rest of the input string to BLEfriend shield" tcEOSCR
#endif // defined(useBluetoothAdaFruitSPI)
	"           S - toggles display status line echo to terminal" tcEOSCR
	"           ? - displays this help" tcEOSCR
	tcEOS
};

#endif // defined(useDebugTerminalHelp)
static uint8_t terminalState;
static uint8_t nextTerminalState;
static uint8_t terminalCmd;
static uint8_t terminalMode;

static uint8_t terminalByte;
static uint8_t terminalSource;
static uint8_t terminalTarget;

static uint8_t peek;
static uint8_t terminalAddress;
static uint8_t terminalLine;
static uint8_t maxLine;
static uint8_t decPlace;
static uint8_t decWindow;
static uint8_t decMode;

ringBufferVariable terminalBuffer;

static const uint8_t tBuffLength = 120;
static volatile uint8_t terminalBuff[(uint16_t)(tBuffLength)];

static const char * labelList;
static const uint8_t * prgmPtr;
static void (* primaryFunc)(uint8_t);
static void (* extraFunc)(uint8_t);

static const char terminalPrimarySeparator[] PROGMEM = {
	": " tcEOS
};

static const char terminalSecondarySeparator[] PROGMEM = {
	" - " tcEOS
};

static const char terminalActivityFlagStr[] PROGMEM = {
	"activityFlags: " tcEOS
	"EngOff" tcOTOG "0" tcEOS
	"VehStop" tcOTOG "0" tcEOS
	"NoButtons" tcOTOG "0" tcEOS
	"Parked" tcOTOG "0" tcEOS
	"Inactive" tcOTOG "0" tcEOS
	"FuelRate" tcOTOG "0" tcEOS
	"1" tcOTOG "0" tcEOS
	"1" tcOTOG "0" tcEOS
};

static const uint8_t peekStatusMessage =		0b10000000;
static const uint8_t peekBluetoothInput =		0b01000000;
static const uint8_t peekBluetoothOutput =		0b00100000;
static const uint8_t peekBLEfriendEcho =		0b00010000;

static const char terminalPeekStr[] PROGMEM = {
	"peek: " tcEOS
	"STATUS" tcOTOG "0" tcEOS
#if defined(useBluetooth)
	"BTinp" tcOTOG "0" tcEOS
	"BTout" tcOTOG "0" tcEOS
#else // defined(useBluetooth)
	"1" tcOTOG "0" tcEOS
	"1" tcOTOG "0" tcEOS
#endif // defined(useBluetooth)
#if defined(useBluetoothAdaFruitSPI)
	"BLEecho" tcOTOG "0" tcEOS
#else // defined(useBluetoothAdaFruitSPI)
	"1" tcOTOG "0" tcEOS
#endif // defined(useBluetoothAdaFruitSPI)
	"1" tcOTOG "0" tcEOS
	"1" tcOTOG "0" tcEOS
	"1" tcOTOG "0" tcEOS
	"1" tcOTOG "0" tcEOS
};

#if defined(useSerial0Port)
static const char serial0ControlFlagsStr[] PROGMEM = {
	"devSerial0.controlFlags: " tcEOS
	"1" tcOTOG "0" tcEOS
	"1" tcOTOG "0" tcEOS
	"CRLF" tcOTOG "0" tcEOS
	"FE0" tcOTOG "0" tcEOS
	"DOR0" tcOTOG "0" tcEOS
	"UPE0" tcOTOG "0" tcEOS
	"DH" tcOTOG "0" tcEOS
	"OUTPUT" tcOTOG "0" tcEOS
};

#endif // defined(useSerial0Port)
#if defined(useSerial1Port)
static const char serial1ControlFlagsStr[] PROGMEM = {
	"devSerial1.controlFlags: " tcEOS
	"1" tcOTOG "0" tcEOS
	"1" tcOTOG "0" tcEOS
	"CRLF" tcOTOG "0" tcEOS
	"FE1" tcOTOG "0" tcEOS
	"DOR1" tcOTOG "0" tcEOS
	"UPE1" tcOTOG "0" tcEOS
	"DH" tcOTOG "0" tcEOS
	"OUTPUT" tcOTOG "0" tcEOS
};

#endif // defined(useSerial1Port)
#if defined(useSerial2Port)
static const char serial2ControlFlagsStr[] PROGMEM = {
	"devSerial2.controlFlags: " tcEOS
	"1" tcOTOG "0" tcEOS
	"1" tcOTOG "0" tcEOS
	"CRLF" tcOTOG "0" tcEOS
	"FE2" tcOTOG "0" tcEOS
	"DOR2" tcOTOG "0" tcEOS
	"UPE2" tcOTOG "0" tcEOS
	"DH" tcOTOG "0" tcEOS
	"OUTPUT" tcOTOG "0" tcEOS
};

#endif // defined(useSerial2Port)
#if defined(useSerial3Port)
static const char serial3ControlFlagsStr[] PROGMEM = {
	"devSerial3.controlFlags: " tcEOS
	"1" tcOTOG "0" tcEOS
	"1" tcOTOG "0" tcEOS
	"CRLF" tcOTOG "0" tcEOS
	"FE3" tcOTOG "0" tcEOS
	"DOR3" tcOTOG "0" tcEOS
	"UPE3" tcOTOG "0" tcEOS
	"DH" tcOTOG "0" tcEOS
	"OUTPUT" tcOTOG "0" tcEOS
};

#endif // defined(useSerial3Port)
#endif // defined(useDebugTerminal)
#if defined(useTestButtonValues)
namespace buttonView /* Button input value viewer section prototype */
{

	static uint8_t displayHandler(uint8_t cmd, uint8_t cursorPos);

}

#endif // defined(useTestButtonValues)
#if defined(useActivityLED)
namespace activityLED /* Activity status LED support section prototype */
{

	static void init(void);
	static void shutdown(void);
	static void output(uint8_t val);

};

#endif // defined(useActivityLED)
