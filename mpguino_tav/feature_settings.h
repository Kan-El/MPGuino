namespace settings /* EEPROM parameter settings menu support section prototype */
{

	static uint8_t menuHandler(uint8_t cmd, uint8_t cursorPos);

};

static const char settingsMenuTitles[] PROGMEM = {	// each title must be no longer than 15 characters
	"Display" tcEOSCR
	"Fuel Injector" tcEOSCR
	"VSS" tcEOSCR
	"Tank Quantity" tcEOSCR
#ifdef useChryslerMAPCorrection
	"Chrysler MAP" tcEOSCR
#endif // useChryslerMAPCorrection
#if defined(useCoastDownCalculator) or defined(useDragRaceFunction)
	"VehicleParam" tcEOSCR
#endif // defined(useCoastDownCalculator) or defined(useDragRaceFunction)
	"Timeout" tcEOSCR
	"Miscellaneous" tcEOSCR
};

const uint8_t nesLoadInitial =			0;
const uint8_t nesLoadValue =			nesLoadInitial + 1;
const uint8_t nesOnChange =				nesLoadValue + 1;
const uint8_t nesSaveParameter =		nesOnChange + 1;

typedef struct
{

	uint8_t callingDisplayIdx; // remember what module called number editor
	uint8_t parameterIdx;
	const char * neStatusMessage;

} numberEditType;

static numberEditType numberEditObj;

namespace parameterEdit /* parameter editor/entry section prototype */
{

	static uint8_t sharedFunctionCall(uint8_t cmd);
	static uint8_t menuHandler(uint8_t cmd, uint8_t cursorPos);
	static void displayHandler(uint8_t cmd, uint8_t cursorPos);
	static void entry(void);
	static void findLeft(void);
	static void findRight(void);
	static void readInitial(void);
	static void readMinValue(void);
	static void readMaxValue(void);
	static void changeDigitUp(void);
	static void changeDigitDown(void);
	static void changeDigit(uint8_t dir);
	static void save(void);
	static void cancel(void);
	static uint8_t onEEPROMchange(const uint8_t * sched, uint8_t parameterIdx);

};

static const char numberEditSaveCanx[] PROGMEM = {
	" OK XX"
};

static const char pseStatusMessages[] PROGMEM = {
	"Param Unchanged" tcEOS
	"Param Changed" tcEOS
	"Param Reverted" tcEOS
};

