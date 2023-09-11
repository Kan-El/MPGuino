#if defined(useAnalogRead)
// bit flags for use with bfAnalogCommand
static const uint8_t acSampleGround =			0b10000000;
static const uint8_t acSampleChannel0 =			0b00100000;
static const uint8_t acSampleChannel1 =			0b00010000;
static const uint8_t acSampleChannel2 =			0b00001000;
static const uint8_t acSampleChannel3 =			0b00000100;
static const uint8_t acSampleChannel4 =			0b00000010;
static const uint8_t acSampleChannel5 =			0b00000001;

static const uint8_t acSampleChannelActive =	0b00111111;
#if defined(useChryslerMAPCorrection)
#if defined(useChryslerBaroSensor)
#if defined(useAnalogButtons)
static const uint8_t acSampleChannelInit =		0b00000111;			// MAP sensor, baro sensor, and button input do not get scanned normally
#else // defined(useAnalogButtons)
static const uint8_t acSampleChannelInit =		0b00001111;			// MAP sensor and baro sensor do not get scanned normally
#endif // defined(useAnalogButtons)
#else // defined(useChryslerBaroSensor)
#if defined(useAnalogButtons)
static const uint8_t acSampleChannelInit =		0b00001111;			// MAP sensor and button input do not get scanned normally
#else // defined(useAnalogButtons)
static const uint8_t acSampleChannelInit =		0b00011111;			// MAP sensor does not get scanned normally
#endif // defined(useAnalogButtons)
#endif // defined(useChryslerBaroSensor)
#else // defined(useChryslerMAPCorrection)
#if defined(useAnalogButtons)
static const uint8_t acSampleChannelInit =		0b00011111;			// button input does not get scanned normally
#else // defined(useAnalogButtons)
static const uint8_t acSampleChannelInit =		0b00111111;			// used to start scanning of all non-critical analog channels
#endif // defined(useAnalogButtons)
#endif // defined(useChryslerMAPCorrection)

#if defined(useChryslerMAPCorrection)
static const uint8_t acSampleMAPchannel =		acSampleChannel0;
#if defined(useChryslerBaroSensor)
static const uint8_t acSampleChrysler =			acSampleChannel0 | acSampleChannel1;
static const uint8_t acSampleBaroChannel =		acSampleChannel1;
#if defined(useAnalogButtons)
static const uint8_t acSampleButtonChannel =	acSampleChannel2;
#endif // defined(useAnalogButtons)
#else // defined(useChryslerBaroSensor)
static const uint8_t acSampleChrysler =			acSampleChannel0;
#if defined(useAnalogButtons)
static const uint8_t acSampleButtonChannel =	acSampleChannel1;
#endif // defined(useAnalogButtons)
#endif // defined(useChryslerBaroSensor)
#else // defined(useChryslerMAPCorrection)
#if defined(useAnalogButtons)
static const uint8_t acSampleButtonChannel =	acSampleChannel0;
#endif // defined(useAnalogButtons)
#endif // defined(useChryslerMAPCorrection)

// bit flags for use with bfAnalogStatus
static const uint8_t asHardwareReady =			0b10000000;
static const uint8_t asReadChannel0 =			0b00100000;
static const uint8_t asReadChannel1 =			0b00010000;
static const uint8_t asReadChannel2 =			0b00001000;
static const uint8_t asReadChannel3 =			0b00000100;
static const uint8_t asReadChannel4 =			0b00000010;
static const uint8_t asReadChannel5 =			0b00000001;
static const uint8_t asReadChannelMask =		0b00111111;

#if defined(useChryslerMAPCorrection)
static const uint8_t asReadMAPchannel =			asReadChannel0;
#if defined(useChryslerBaroSensor)
static const uint8_t asReadBaroChannel =		asReadChannel1;
#if defined(useAnalogButtons)
static const uint8_t asReadButtonChannel =		asReadChannel2;
#endif // defined(useAnalogButtons)
#else // defined(useChryslerBaroSensor)
#if defined(useAnalogButtons)
static const uint8_t asReadButtonChannel =		asReadChannel1;
#endif // defined(useAnalogButtons)
#endif // defined(useChryslerBaroSensor)
#else // defined(useChryslerMAPCorrection)
#if defined(useAnalogButtons)
static const uint8_t asReadButtonChannel =		asReadChannel0;
#endif // defined(useAnalogButtons)
#endif // defined(useChryslerMAPCorrection)

#if defined(useAnalogButtons)
volatile uint8_t analogButton;

#endif // defined(useAnalogButtons)
volatile uint8_t analogValueIdx;
volatile uint8_t analogBitmask;

#define nextAllowedValue 0
static const uint8_t analog0Idx =					nextAllowedValue;	// highest priority analog channel
static const uint8_t analog1Idx =					analog0Idx + 1;
static const uint8_t analog2Idx =					analog1Idx + 1;
static const uint8_t analog3Idx =					analog2Idx + 1;
static const uint8_t analog4Idx =					analog3Idx + 1;
static const uint8_t analog5Idx =					analog4Idx + 1;
#define nextAllowedValue analog5Idx + 1

static const uint8_t dfMaxValAnalogCount =			nextAllowedValue;

static const uint8_t analogGroundIdx =				nextAllowedValue;
#define nextAllowedValue analogGroundIdx + 1

static const uint8_t dfMaxAnalogCount =				nextAllowedValue;

#if defined(useChryslerMAPCorrection)
static const uint8_t analogMAPchannelIdx =			analog0Idx;
#if defined(useChryslerBaroSensor)
static const uint8_t analogBaroChannelIdx =			analog1Idx;
#if defined(useAnalogButtons)
static const uint8_t analogButtonChannelIdx =		analog2Idx;
#endif // defined(useAnalogButtons)
#else // defined(useChryslerBaroSensor)
#if defined(useAnalogButtons)
static const uint8_t analogButtonChannelIdx =		analog1Idx;
#endif // defined(useAnalogButtons)
#endif // defined(useChryslerBaroSensor)
#else // defined(useChryslerMAPCorrection)
#if defined(useAnalogButtons)
static const uint8_t analogButtonChannelIdx =		analog0Idx;
#endif // defined(useAnalogButtons)
#endif // defined(useChryslerMAPCorrection)
#if defined(useCarVoltageOutput)
#if defined(useAnalogButtons) || defined(useChryslerMAPCorrection)
static const uint8_t analogAlternatorChannelIdx =	analog2Idx;
#else // defined(useAnalogButtons) || defined(useChryslerMAPCorrection)
static const uint8_t analogAlternatorChannelIdx =	analog1Idx;
#endif // defined(useAnalogButtons) || defined(useChryslerMAPCorrection)
#endif // defined(useCarVoltageOutput)

volatile uint16_t analogValue[(uint16_t)(dfMaxValAnalogCount)];

static const uint8_t analogChannelValue[(uint16_t)(dfMaxAnalogCount)] PROGMEM = { // points to the next channel to be read
#if defined(__AVR_ATmega32U4__)
#if defined(useChryslerMAPCorrection)
	 (1 << REFS0)|									(1 << MUX2)|	(1 << MUX1)					// 0 PF6 A1 MAP sensor
#if defined(useChryslerBaroSensor)
	,(1 << REFS0)|									(1 << MUX2)|					(1 << MUX0)	// 1 PF5 A2 baro sensor
#if defined(useAnalogButtons)
	,(1 << REFS0)|									(1 << MUX2)|	(1 << MUX1)|	(1 << MUX0)	// 2 PF7 A0 analog button input
#else // defined(useAnalogButtons)
	,(1 << REFS0)|									(1 << MUX2)|	(1 << MUX1)|	(1 << MUX0)	// 2 PF7 A0
#endif // defined(useAnalogButtons)
#else // defined(useChryslerBaroSensor)
#if defined(useAnalogButtons)
	,(1 << REFS0)|									(1 << MUX2)|	(1 << MUX1)|	(1 << MUX0)	// 1 PF7 A0 analog button input
#else // defined(useAnalogButtons)
	,(1 << REFS0)|									(1 << MUX2)|	(1 << MUX1)|	(1 << MUX0)	// 1 PF7 A0
#endif // defined(useAnalogButtons)
	,(1 << REFS0)|									(1 << MUX2)|					(1 << MUX0)	// 2 PF5 A2 car voltage
#endif // defined(useChryslerBaroSensor)
#else // defined(useChryslerMAPCorrection)
#if defined(useAnalogButtons)
	 (1 << REFS0)|									(1 << MUX2)|	(1 << MUX1)|	(1 << MUX0)	// 0 PF7 A0 analog button input
	,(1 << REFS0)|									(1 << MUX2)|	(1 << MUX1)					// 1 PF6 A1
	,(1 << REFS0)|									(1 << MUX2)|					(1 << MUX0)	// 2 PF5 A2 car voltage
#else // defined(useAnalogButtons)
	 (1 << REFS0)|									(1 << MUX2)|	(1 << MUX1)|	(1 << MUX0)	// 0 PF7 A0
	,(1 << REFS0)|									(1 << MUX2)|	(1 << MUX1)					// 1 PF6 A1
	,(1 << REFS0)|									(1 << MUX2)|					(1 << MUX0)	// 2 PF5 A2 car voltage
#endif // defined(useAnalogButtons)
#endif // defined(useChryslerMAPCorrection)
	,(1 << REFS0)|	(1 << MUX4)|	(1 << MUX3)|	(1 << MUX2)|	(1 << MUX1)|	(1 << MUX0)	// 3 ground
	,(1 << REFS0)|	(1 << MUX4)|	(1 << MUX3)|	(1 << MUX2)|	(1 << MUX1)|	(1 << MUX0)	// 4 ground
	,(1 << REFS0)|	(1 << MUX4)|	(1 << MUX3)|	(1 << MUX2)|	(1 << MUX1)|	(1 << MUX0)	// 5 ground
	,(1 << REFS0)|	(1 << MUX4)|	(1 << MUX3)|	(1 << MUX2)|	(1 << MUX1)|	(1 << MUX0)	// 6 ground
#endif // defined(__AVR_ATmega32U4__)

#if defined(__AVR_ATmega2560__)
#if defined(useChryslerMAPCorrection)
	 (1 << REFS0)|																	(1 << MUX0)	// 0 PF1 A1 MAP sensor
#if defined(useChryslerBaroSensor)
	,(1 << REFS0)|													(1 << MUX1)					// 1 PF2 A2 baro sensor
#if defined(useAnalogButtons)
	,(1 << REFS0)|													(1 << MUX1)|	(1 << MUX0)	// 2 PF3 A3 analog button input
#else // defined(useAnalogButtons)
	,(1 << REFS0)|													(1 << MUX1)|	(1 << MUX0)	// 2 PF3 A3
#endif // defined(useAnalogButtons)
#else // defined(useChryslerBaroSensor)
#if defined(useAnalogButtons)
	,(1 << REFS0)|													(1 << MUX1)|	(1 << MUX0)	// 1 PF3 A3 analog button input
#else // defined(useAnalogButtons)
	,(1 << REFS0)|													(1 << MUX1)|	(1 << MUX0)	// 1 PF3 A3
#endif // defined(useAnalogButtons)
	,(1 << REFS0)|													(1 << MUX1)					// 2 PF2 A2 car voltage
#endif // defined(useChryslerBaroSensor)
#else // defined(useChryslerMAPCorrection)
#if defined(useAnalogButtons)
	 (1 << REFS0)|													(1 << MUX1)|	(1 << MUX0)	// 0 PF3 A3 analog button input
	,(1 << REFS0)|																	(1 << MUX0)	// 1 PF1 A1
	,(1 << REFS0)|													(1 << MUX1)					// 2 PF2 A2 car voltage
#else // defined(useAnalogButtons)
	 (1 << REFS0)|																	(1 << MUX0)	// 0 PF1 A1
	,(1 << REFS0)|													(1 << MUX1)					// 1 PF2 A2 car voltage
	,(1 << REFS0)|													(1 << MUX1)|	(1 << MUX0)	// 2 PF3 A3
#endif // defined(useAnalogButtons)
#endif // defined(useChryslerMAPCorrection)
#if defined(useTWIsupport)
	,(1 << REFS0)|					(1 << MUX3)|	(1 << MUX2)|	(1 << MUX1)|	(1 << MUX0)	// 3 ground
	,(1 << REFS0)|					(1 << MUX3)|	(1 << MUX2)|	(1 << MUX1)|	(1 << MUX0)	// 4 ground
#else // defined(useTWIsupport)
	,(1 << REFS0)|									(1 << MUX2)									// 3 PF4 A4
	,(1 << REFS0)|									(1 << MUX2)|					(1 << MUX0)	// 4 PF5 A5
#endif // defined(useTWIsupport)
	,(1 << REFS0)|					(1 << MUX3)|	(1 << MUX2)|	(1 << MUX1)|	(1 << MUX0)	// 5 ground
	,(1 << REFS0)|					(1 << MUX3)|	(1 << MUX2)|	(1 << MUX1)|	(1 << MUX0)	// 6 ground
#endif // defined(__AVR_ATmega2560__)

#if defined(__AVR_ATmega328P__)
#if defined(useChryslerMAPCorrection)
	 (1 << REFS0)|																	(1 << MUX0)	// 0 PC1 A1 MAP sensor
#if defined(useChryslerBaroSensor)
	,(1 << REFS0)|													(1 << MUX1)					// 1 PC2 A2 baro sensor
#if defined(useAnalogButtons)
	,(1 << REFS0)|													(1 << MUX1)|	(1 << MUX0)	// 2 PC3 A3 analog button input
#else // defined(useAnalogButtons)
	,(1 << REFS0)|													(1 << MUX1)|	(1 << MUX0)	// 2 PC3 A3
#endif // defined(useAnalogButtons)
#else // defined(useChryslerBaroSensor)
#if defined(useAnalogButtons)
	,(1 << REFS0)|													(1 << MUX1)|	(1 << MUX0)	// 1 PC3 A3 analog button input
#else // defined(useAnalogButtons)
	,(1 << REFS0)|													(1 << MUX1)|	(1 << MUX0)	// 1 PC3 A3
#endif // defined(useAnalogButtons)
	,(1 << REFS0)|													(1 << MUX1)					// 2 PC2 A2 car voltage
#endif // defined(useChryslerBaroSensor)
#else // defined(useChryslerMAPCorrection)
#if defined(useAnalogButtons)
	 (1 << REFS0)|													(1 << MUX1)|	(1 << MUX0)	// 0 PC3 A3 analog button input
	,(1 << REFS0)|																	(1 << MUX0)	// 1 PC1 A1
	,(1 << REFS0)|													(1 << MUX1)					// 2 PC2 A2 car voltage
#else // defined(useAnalogButtons)
	 (1 << REFS0)|																	(1 << MUX0)	// 0 PC1 A1
	,(1 << REFS0)|													(1 << MUX1)					// 1 PC2 A2 car voltage
	,(1 << REFS0)|													(1 << MUX1)|	(1 << MUX0)	// 2 PC3 A3
#endif // defined(useAnalogButtons)
#endif // defined(useChryslerMAPCorrection)
#if defined(useTWIsupport)
	,(1 << REFS0)|					(1 << MUX3)|	(1 << MUX2)|	(1 << MUX1)|	(1 << MUX0) // 3 ground
	,(1 << REFS0)|					(1 << MUX3)|	(1 << MUX2)|	(1 << MUX1)|	(1 << MUX0)	// 4 ground
#else // defined(useTWIsupport)
	,(1 << REFS0)|									(1 << MUX2)									// 3 PC4 A4
	,(1 << REFS0)|									(1 << MUX2)|					(1 << MUX0)	// 4 PC5 A5
#endif // defined(useTWIsupport)
	,(1 << REFS0)|					(1 << MUX3)|	(1 << MUX2)|	(1 << MUX1)|	(1 << MUX0)	// 5 ground
	,(1 << REFS0)|					(1 << MUX3)|	(1 << MUX2)|	(1 << MUX1)|	(1 << MUX0)	// 6 ground
#endif // defined(__AVR_ATmega328P__)
};

#endif // defined(useAnalogRead)
#if defined(useDebugAnalog)
namespace analogReadViewer /* ADC voltage display section prototype */
{

	static uint8_t displayHandler(uint8_t cmd, uint8_t cursorPos);
	static uint16_t getAnalogReadPageFormats(uint8_t formatIdx);

}

static const char analogReadDisplayTitles[] PROGMEM = {
	"Voltages" tcEOS
};

#endif // defined(useDebugAnalog)
