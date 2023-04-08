#if defined(useAnalogRead)
// this interrupt is called upon completion of an analog to digital conversion
//
// this interrupt is normally initially called by timer0, and will continue as long as there are analog channel requests present
//
// each analog read is immediately preceded by an analog read of the internal ground, to ensure the successive approximation capacitor network is ready to go
//
// based on a 20 MHz clock, and a prescaler of 128, a single A/D conversion should take about 179 microseconds
//
// it is important to note that even if a conversion is complete, and free-wheeling mode is NOT repeat NOT selected at all,
// the AVR hardware will continue to start another conversion and interrupt itself again (!) and this is not mentioned at all in the fucking manual
//
ISR( ADC_vect )
{

	unsigned int rawRead;
	union union_16 * rawValue = (union union_16 *)(&rawRead);
	uint8_t analogChannelIdx;
	uint8_t analogChannelMask;
	uint8_t flag;
#ifdef useDebugCPUreading
	uint8_t a;
	uint8_t b;
	uint16_t c;

	a = TCNT0; // do a microSeconds() - like read to determine interrupt length in cycles
#endif // useDebugCPUreading

	rawValue->u8[0] = ADCL; // (locks ADC sample result register from AtMega hardware)
	rawValue->u8[1] = ADCH; // (releases ADC sample result register to AtMega hardware)

	if (analogCommand & acSampleGround)
	{

		analogCommand &= ~(acSampleGround); // signal that internal ground was read

		flag = 1;
		analogChannelMask = acSampleChannel0; // start with highest priority channel
		analogChannelIdx = 0;

		while ((flag) && (analogChannelMask))
		{

			if (analogCommand & analogChannelMask) flag = 0; // if a commanded analog channel was detected, exit the loop
			else
			{

				analogChannelIdx++; // increment the analog channel index
				analogChannelMask >>= 1; // shift mask right one bit

			}

		}

		if (analogChannelMask)
		{

			analogStatus &= ~(analogChannelMask); // main program really doesn't care that a ground was read, it's not useful, so don't signal it
			analogValueIdx = analogChannelIdx; // save the analog index value
			analogBitmask = analogChannelMask; // save the analog bitmask
			analogCommand &= ~(analogChannelMask); // clear the relevant bit in analog command status
			flag = 1;

		}
		else
		{

			analogCommand &= ~(acSampleChannelActive); // an invalid channel was requested, so ignore it
			flag = 0;

		}

	}
	else
	{

		analogValue[(unsigned int)(analogValueIdx)] = rawRead; // save the value just read in
		analogStatus |= (analogBitmask); // signal to main program that an analog channel was read in
		if (analogCommand & acSampleChannelActive)
		{

			analogCommand |= (acSampleGround); // signal that next read is for internal ground
			analogChannelIdx = analogGroundIdx;
			flag = 1;

		}
		else flag = 0;

	}

	if (flag)
	{

		ADMUX = pgm_read_byte(&analogChannelValue[(unsigned int)(analogChannelIdx)]); // select next analog channel to read
		ADCSRA |= ((1 << ADSC) | (1 << ADIF) | (1 << ADIE)); // start ADC read, enable interrupt, and clear interrupt flag

	}
	else
	{

		analogStatus |= (asHardwareReady);
		analogCommand &= ~(acSampleChannelActive); // an invalid channel was requested, so ignore it
		ADCSRA |= (1 << ADIF);
		ADCSRA &= ~(1 << ADIE); // shut off analog interrupt and clear analog interrupt flag

	}

#ifdef useDebugCPUreading
	b = TCNT0; // do a microSeconds() - like read to determine interrupt length in cycles

	if (b < a) c = 256 - a + b; // an overflow occurred
	else c = b - a;

	volatileVariables[(uint16_t)(vInterruptAccumulatorIdx)] += c;

#endif // useDebugCPUreading
}

#endif // defined(useAnalogRead)
#ifdef useDebugAnalog
 /* ADC voltage display section */

const uint8_t analogReadScreenFormats[4][2] PROGMEM = {
	 {analog0Idx,				0x80 | tAnalogChannel}	// Voltages
	,{analog1Idx,				0x80 | tAnalogChannel}
	,{analog2Idx,				0x80 | tAnalogChannel}
	,{analog3Idx,				0x80 | tAnalogChannel}
};

static uint8_t analogReadViewer::displayHandler(uint8_t cmd, uint8_t cursorPos, uint8_t cursorChanged)
{

	uint8_t retVal = 0;

	switch (cmd)
	{

		case menuExitIdx:
			break;

		case menuEntryIdx:
		case menuCursorUpdateIdx:
			printStatusMessage(analogReadScreenFuncNames, cursorPos); // briefly display screen name

		case menuOutputDisplayIdx:
#ifdef useSpiffyTripLabels
			displayMainScreenFunctions(analogReadScreenFormats, cursorPos, 136, 0, msTripBitPattern);
#else // useSpiffyTripLabels
			displayMainScreenFunctions(analogReadScreenFormats, cursorPos);
#endif // useSpiffyTripLabels
			break;

		default:
			break;

	}

	return retVal;

}

#endif // useDebugAnalog