// this ISR gets called every time timer 0 overflows.
//
// f(fast PWM) = f(system clock) / (N * 256)
//
// N - prescaler, which is 64
//
// so this ISR gets called every 256 * 64 / (system clock) seconds
//   - 20 MHz clock -> once every 0.8192 ms
//   - 16 MHz clock -> once every 1.024 ms
//
ISR( TIMER0_OVF_vect ) // system timer interrupt handler
{

	static uint32_t inputTimeoutCount;
	static uint32_t parkTimeoutCount;
	static uint32_t activityTimeoutCount;
	static uint32_t swapFEwithFCRcount;
#if defined(useCoastDownCalculator)
	static uint32_t coastdownCount;
#endif // defined(useCoastDownCalculator)
#if defined(useBarFuelEconVsTime)
	static uint32_t FEvTimeCount;
#endif // defined(useBarFuelEconVsTime)
#if defined(useButtonInput)
	static uint16_t buttonLongPressCount;
#endif // defined(useButtonInput)
	static uint16_t cursorCount;
	static uint16_t loopCount;
#if defined(useJSONoutput)
	static uint16_t JSONtimeoutCount;
#endif // defined(useJSONoutput)
#if defined(useBluetoothAdaFruitSPI)
	static uint16_t BLEtimeoutCount;
#endif // defined(useBluetoothAdaFruitSPI)
	static uint8_t previousActivity;
#if defined(useButtonInput)
	static uint8_t internalFlags;
#endif // defined(useButtonInput)
#if defined(useAnalogButtons)
	static uint16_t analogSampleCount;
#endif // defined(useAnalogButtons)
#if defined(useBluetooth)
	static uint16_t bluetoothPeriodCount;
#endif // defined(useBluetooth)
#if defined(useTWIbuttons)
	static uint8_t TWIsampleCount;
	static uint8_t TWIsampleState;
#endif // defined(useTWIbuttons)
	uint32_t thisTime;

	if (timer0Command & t0cResetTimer)
	{

		timer0Command &= ~(t0cResetTimer);
		timer0_overflow_count = 0; // initialize timer 0 overflow counter
		timer0DelayFlags = 0;
		timer0DisplayDelayFlags = 0;
		thisTime = TCNT0;
		timer0Status = 0;
		loopCount = delay0TickSampleLoop;
		awakeFlags = 0;
#if defined(useBluetoothAdaFruitSPI)
		BLEtimeoutCount = 0;
#endif // defined(useBluetoothAdaFruitSPI)
#if defined(useButtonInput)
		internalFlags = 0;
#endif // defined(useButtonInput)
		mainLoopHeartBeat = 1;
		dirty &= ~(dGoodVehicleDrive);
		activityTimeoutCount = volatileVariables[(uint16_t)(vActivityTimeoutIdx)];
		activityFlags = (afActivityCheckFlags | afSwapFEwithFCR);
		previousActivity = (afActivityCheckFlags);
#if defined(useTWIbuttons)
		TWIsampleCount = delay0TickTWIsample;
		TWIsampleState = 0;
#endif // defined(useTWIbuttons)
#if defined(useBluetooth)
		bluetoothPeriodCount = delay0TickSampleLoop;
#endif // defined(useBluetooth)
#if defined(useAnalogRead)
		analogStatus = asHardwareReady;
#if defined(useAnalogButtons)
		analogSampleCount = delay0TickAnalogSample;
#endif // defined(useAnalogButtons)
#endif // defined(useAnalogRead)
#if defined(useLegacyButtons)
		buttonDebounceCount = 0;
#endif // defined(useLegacyButtons)
#if defined(useBarFuelEconVsTime)
		timer0Command |= (t0cResetFEvTime);
#endif // defined(useBarFuelEconVsTime)
#if defined(useButtonInput)
		buttonLongPressCount = 0;
#endif // defined(useButtonInput)
		inputTimeoutCount = 0;
		parkTimeoutCount = 0;
		swapFEwithFCRcount = 0;

	}
	else
	{

		timer0_overflow_count += 256; // update TOV count
		thisTime = timer0_overflow_count | TCNT0; // calculate current cycle count

#if defined(useCPUreading)
		volatileVariables[(uint16_t)(vSystemCycleIdx)]++; // update systemcycles

#endif // defined(useCPUreading)
#if defined(useSoftwareClock)
		// update clockcycles - if clockcycles goes past day length in timer0 ticks, roll back to 0
		if ((++volatileVariables[(uint16_t)(vClockCycleIdx)]) >= volatileVariables[(uint16_t)(vClockCycleDayLengthIdx)]) volatileVariables[(uint16_t)(vClockCycleIdx)] = 0;

#endif // defined(useSoftwareClock)
	}

	if (awakeFlags & aAwakeOnInjector) // if MPGuino is awake on detected fuel injector event
	{

		if (watchdogInjectorCount) // if the fuel injector watchdog timer is running on minimum good engine speed
		{

			watchdogInjectorCount--; // cycle fuel injector watchdog timer down

#if defined(useChryslerMAPCorrection)
			if (dirty & dSampleADC) // if injector monitor commanded an analog engine sensor read
			{

				dirty &= ~(dSampleADC); // acknowledge the command
				analogCommand |= (acSampleChrysler);

			}

#endif // defined(useChryslerMAPCorrection)
		}
		else // fuel injector watchdog timer has timed out
		{

			awakeFlags &= ~(aAwakeOnInjector); // signal that MPGuino is not awake any more due to no detected injector event during injector watchdog period
			dirty &= ~(dGoodEngineRun); // reset all fuel injector measurement flags
			watchdogInjectorCount = volatileVariables[(uint16_t)(vEngineOffTimeoutIdx)]; // start the fuel injector watchdog for engine off mode

		}

	}
	else // MPGuino is no longer awake due to no detected fuel injector events
	{

		if (awakeFlags & aAwakeEngineRunning) // if MPGuino is still awake due to running engine
		{

			if (watchdogInjectorCount) watchdogInjectorCount--; // cycle fuel injector watchdog timer down for engine off flag mode
			else
			{

				activityFlags |= (afEngineOffFlag); // flag engine as being off
				awakeFlags &= ~(aAwakeEngineRunning); // MPGuino is no longer awake due to engine running

			}

		}

	}

	if (VSScount) // if there is a VSS debounce countdown in progress
	{

		VSScount--; // bump down the VSS count
		if (VSScount == 0) heart::updateVSS(thisTime); // if count has reached zero, go update VSS

	}

	if (awakeFlags & aAwakeOnVSS) // if MPGuino is awake on detected VSS pulse event
	{

		if (watchdogVSSCount) // if the VSS watchdog timer is running on minimum good vehicle speed
		{

			watchdogVSSCount--;

		}
		else // VSS watchdog timer has timed out on minimum good vehicle speed
		{

			awakeFlags &= ~(aAwakeOnVSS); // signal that MPGuino is no longer awake due to no detected VSS pulse event during VSS watchdog period
			dirty &= ~(dGoodVehicleMotion); // reset all VSS measurement flags
			watchdogVSSCount = volatileVariables[(uint16_t)(vVehicleStopTimeoutIdx)]; // start the VSS watchdog for vehicle stopped mode
			swapFEwithFCRcount = delay0Tick3000ms; // reset swap timer counter

		}

	}
	else // MPGuino is no longer awake due to no detected VSS pulse events
	{

		if (awakeFlags & aAwakeVehicleMoving) // if MPGuino is awake due to detected vehicle movement
		{

			if (watchdogVSSCount) watchdogVSSCount--;// cycle VSS watchdog timer down for vehicle stopped flag mode
			else
			{

				activityFlags |= (afVehicleStoppedFlag); // flag vehicle as stopped
				awakeFlags &= ~(aAwakeVehicleMoving); // vehicle is no longer awake on detected vehicle movement

#if defined(useDragRaceFunction)
				if (accelerationFlags & accelTestActive) // if accel test function is active
				{

					accelerationFlags &= ~(accelTestClearFlags); // reset accel test capture flags
					accelerationFlags |= (accelTestCompleteFlags); // signal that accel test is cancelled
					timer0Status |= (t0sAccelTestFlag);

				}

#endif // defined(useDragRaceFunction)
#if defined(useCoastDownCalculator)
				if (coastdownFlags & cdTestInProgress) // if coastdown test has started
				{

					coastdownFlags &= ~(cdTestClearFlags); // signal that coastdown test is no longer active
					coastdownFlags |= (cdTestCompleteFlags); // signal that coastdown test is cancelled
					timer0Status |= (t0sCoastdownTestFlag);

				}

#endif // defined(useCoastDownCalculator)
			}

		}

		if ((activityFlags & afSwapFEwithFCR) == 0) // if not yet showing fuel consumption rate instead of fuel economy
		{

			if (swapFEwithFCRcount) swapFEwithFCRcount--; // cycle down fuel display watchdog until it zeroes out
			else activityFlags |= (afSwapFEwithFCR); // output fuel consumption rate function instead of fuel economy

		}

	}

#if defined(useBarFuelEconVsTime)
	if (timer0Command & t0cResetFEvTime) FEvTperiodIdx = FEvsTimeIdx; // initialize fuel econ vs time trip index variable
	else
	{

		if (FEvTimeCount) FEvTimeCount--;
		else
		{

			timer0Command |= (t0cResetFEvTime);
			FEvTperiodIdx++;
			if (FEvTperiodIdx > FEvsTimeEndIdx) FEvTperiodIdx -= bgDataSize;

		}

	}

	if (timer0Command & t0cResetFEvTime)
	{

		timer0Command &= ~(t0cResetFEvTime);
		tripVar::reset(FEvTperiodIdx); // reset source trip variable
		FEvTimeCount = volatileVariables[(uint16_t)(vFEvsTimePeriodTimeoutIdx)];

	}

#endif // defined(useBarFuelEconVsTime)
#if defined(useCoastDownCalculator)
	if (coastdownFlags & cdTestTriggered) // if coastdown test has been requested
	{

		timer0Status |= (t0sCoastdownTestFlag); // signal to main program that coastdown flags have changed
		coastdownFlags &= ~(cdTestTriggered | cdTestSampleTaken); // clear coastdown test state
		coastdownFlags |= (cdTestActive); // mark coastdown test as active
		coastdownCount = volatileVariables[(uint16_t)(vCoastdownPeriodIdx)]; // reset coastdown timer
		coastdownState = vCoastdownMeasurement1Idx; // reset coastdown state

	}

	if (coastdownFlags & cdTestSampleTaken)
	{

		timer0Status |= (t0sCoastdownTestFlag); // signal to main program that coastdown flags have changed
		coastdownFlags &= ~(cdTestSampleTaken);
		coastdownState++;

		if (coastdownState < vCoastdownPeriodIdx) // if coastdown state is still valid
		{

			coastdownCount = volatileVariables[(uint16_t)(vCoastdownPeriodIdx)]; // reset coastdown timer

		}
		else // otherwise, signal that coastdown test ended normally
		{

			coastdownFlags &= ~(cdTestActive); // make coastdown test no longer active
			coastdownFlags |= cdTestFinished; // signal that coastdown test finished normally

		}

	}

	if (coastdownFlags & cdTestActive) // if coastdown test is active
	{

		if (coastdownCount) coastdownCount--; // if coastdown timer hasn't elapsed
		else if ((coastdownFlags & cdTestSampleTaken) == 0) coastdownFlags |= (cdTestTakeSample); // otherwise, signal VSS handler to take a coastdown sample

	}

#endif // defined(useCoastDownCalculator)
#if defined(useTWIbuttons)
	if (TWIsampleCount)
	{

		TWIsampleCount--;

		if ((twiStatusFlags & twiOpenMain) == twiBlockMainProgram) // if TWI section is finished processing
			switch (TWIsampleState)
			{

				case 0:
					TWI::openChannel(buttonAddress, TW_WRITE); // open TWI as master transmitter
#if defined(useAdafruitRGBLCDshield)
					TWI::writeByte(MCP23017_B1_GPIOA); // specify bank A GPIO pin register address
#endif // defined(useAdafruitRGBLCDshield)
					TWI::transmitChannel(TWI_REPEAT_START); // go write out read request, with repeated start to set up for read

					TWIsampleState++; // advance to waiting for TWI sample request to finish

					break;

				case 1:
					TWI::openChannel(buttonAddress, TW_READ); // open TWI as master receiver
					TWI::transmitChannel(TWI_STOP); // go commit to read, send stop when read is finished

					TWIsampleState++; // advance to waiting for TWI sample to complete

					break;

				case 2:
					if ((twiStatusFlags & twiErrorFlag) == 0)
					{

						thisButtonState = (twiDataBuffer[0] & buttonMask); // fetch button state that was just read in
						timer0Command |= (t0cProcessButton); // send timer0 notification that a button was just read in

					}

				default:
					twiStatusFlags &= ~(twiBlockMainProgram); // allow main program to utilize TWI
					break;

		}

	}
	else
	{

		TWIsampleCount = delay0TickTWIsample;

		if (twiStatusFlags & twiAllowISRactivity)
		{

			twiStatusFlags |= (twiBlockMainProgram); // block main program from making any TWI requests
			TWIsampleState = 0; // initialize TWI button read state machine

		}
		else twiStatusFlags &= ~(twiBlockMainProgram);

	}

#endif // defined(useTWIbuttons)
#if defined(useAnalogButtons)
	if (analogSampleCount) analogSampleCount--;
	else
	{

		analogSampleCount = delay0TickAnalogSample;
		if (timer0Command & t0cEnableAnalogButtons) analogCommand |= (acSampleButtonChannel); // go sample analog button channel

	}

#endif // defined(useAnalogButtons)
#if defined(useLegacyButtons)
	if (buttonDebounceCount) // if there is a button press debounce countdown in progress
	{

		buttonDebounceCount--;

		if (buttonDebounceCount == 0)
		{

			thisButtonState = (lastPINxState & buttonMask) ^ buttonMask; // strip out all but relevant button bits
			timer0Command |= (t0cProcessButton); // send timer0 notification that a button was just read in

		}

	}

#endif // defined(useLegacyButtons)
#if defined(useButtonInput)
	if (buttonLongPressCount)
	{

		buttonLongPressCount--; // bump down the button long-press count by one

		if (buttonLongPressCount == 0)
		{

			buttonPress |= longButtonBit; // signal that a "long" button press has been detected
			internalFlags |= (internalOutputButton);

		}

	}

	if (timer0Command & t0cProcessButton) // if button hardware reports reading in a button
	{

		timer0Command &= ~(t0cProcessButton); // ack
		if (thisButtonState != lastButtonState) // if there was a button state change since the last button was read in
		{

			if (thisButtonState == buttonsUp) // if it's all buttons being released
			{

				if (internalFlags & internalProcessButtonsUp) internalFlags |= (internalOutputButton);

			}
			else
			{

				buttonPress = thisButtonState;
				internalFlags |= (internalProcessButtonsUp);
				buttonLongPressCount = delay0Tick1000ms; // start the button long-press timer

			}

		}

		lastButtonState = thisButtonState;

	}

	if (internalFlags & internalOutputButton)
	{

		internalFlags &= ~(internalOutputButton);
		internalFlags &= ~(internalProcessButtonsUp);
		awakeFlags |= (aAwakeOnInput); // set awake status on button pressed
		timer0DelayFlags &= ~(timer0DisplayDelayFlags); // reset all display delays in progress
		timer0DisplayDelayFlags = 0;
		if (activityFlags & afActivityTimeoutFlag) timer0Status |= (t0sUpdateDisplay); // simply update the display if MPGuino was asleep
		else timer0Status |= (t0sReadButton | t0sShowCursor | t0sUpdateDisplay); // otherwise, force cursor show bit, and signal that keypress was detected
		buttonLongPressCount = 0; // reset button long-press timer
		cursorCount = delay0Tick500ms; // reset cursor count
		activityFlags &= ~(afUserInputFlag | afActivityTimeoutFlag);
		inputTimeoutCount = volatileVariables[(uint16_t)(vButtonTimeoutIdx)];

	}

#endif // defined(useButtonInput)
#if defined(useJSONoutput)
	if (JSONtimeoutCount) JSONtimeoutCount--;
	else
	{

		timer0Status |= t0sJSONchangeSubtitle; // signal to JSON output routine to display next round of subtitles
		JSONtimeoutCount = delay0Tick1600ms; // restart JSON output timeout count

	}

#endif // defined(useJSONoutput)
#if defined(useBluetoothAdaFruitSPI)
	if (timer0Command & t0cEnableBLEsample)
	{

		if (BLEtimeoutCount) BLEtimeoutCount--;
		else
		{

			BLEtimeoutCount = delay0Tick100ms;
			awakeFlags |= (aAwakeSampleBLEfriend);

		}

	}

#endif // defined(useBluetoothAdaFruitSPI)
	if (loopCount) loopCount--;
	else
	{

#if defined(useDataLoggingOutput) || defined(useJSONoutput) || defined(useBluetooth)
		timer0Status |= (t0sUpdateDisplay | t0sTakeSample | t0sOutputLogging); // signal to main program that a sampling should occur, and to update display
#else // defined(useDataLoggingOutput) || defined(useJSONoutput) || defined(useBluetooth)
		timer0Status |= (t0sUpdateDisplay | t0sTakeSample); // signal to main program that a sampling should occur, and to update display
#endif // defined(useDataLoggingOutput) || defined(useJSONoutput) || defined(useBluetooth)
		loopCount = delay0TickSampleLoop; // restart loop count
		mainLoopHeartBeat <<= 1; // cycle the heartbeat bit
		if (mainLoopHeartBeat == 0) mainLoopHeartBeat = 1; // wrap around the heartbeat bit, if necessary
#if defined(useAnalogRead)
		analogCommand |= (acSampleChannelInit); // go sample all non-critical channels
#endif // useAnalogRead

	}

	if (cursorCount) cursorCount--;
	else
	{

		cursorCount = delay0Tick500ms; // reset cursor count
		timer0Status ^= t0sShowCursor; // toggle cursor show bit

	}

	if (timer0DelayFlags & 0x01)
	{

		if (timer0DelayCount[0]) timer0DelayCount[0]--; // bump timer delay value down by one tick
		else
		{

			timer0DelayFlags &= ~(0x01); // signal to main program that delay timer has completed main program request
			if (timer0DisplayDelayFlags & 0x01) // if this was a display delay
			{

				timer0DisplayDelayFlags &= ~(0x01); // clear display delay flag
				if (timer0DisplayDelayFlags == 0) timer0Status |= (t0sUpdateDisplay); // signal to main program to update display

			}

		}

	}

	if (timer0DelayFlags & 0x02)
	{

		if (timer0DelayCount[1]) timer0DelayCount[1]--; // bump timer delay value down by one tick
		else
		{

			timer0DelayFlags &= ~(0x02); // signal to main program that delay timer has completed main program request
			if (timer0DisplayDelayFlags & 0x02) // if this was a display delay
			{

				timer0DisplayDelayFlags &= ~(0x02); // clear display delay flag
				if (timer0DisplayDelayFlags == 0) timer0Status |= (t0sUpdateDisplay); // signal to main program to update display

			}

		}

	}

	if (timer0DelayFlags & 0x04)
	{

		if (timer0DelayCount[2]) timer0DelayCount[2]--; // bump timer delay value down by one tick
		else
		{

			timer0DelayFlags &= ~(0x04); // signal to main program that delay timer has completed main program request
			if (timer0DisplayDelayFlags & 0x04) // if this was a display delay
			{

				timer0DisplayDelayFlags &= ~(0x04); // clear display delay flag
				if (timer0DisplayDelayFlags == 0) timer0Status |= (t0sUpdateDisplay); // signal to main program to update display

			}

		}

	}

	if (timer0DelayFlags & 0x08)
	{

		if (timer0DelayCount[3]) timer0DelayCount[3]--; // bump timer delay value down by one tick
		else
		{

			timer0DelayFlags &= ~(0x08); // signal to main program that delay timer has completed main program request
			if (timer0DisplayDelayFlags & 0x08) // if this was a display delay
			{

				timer0DisplayDelayFlags &= ~(0x08); // clear display delay flag
				if (timer0DisplayDelayFlags == 0) timer0Status |= (t0sUpdateDisplay); // signal to main program to update display

			}

		}

	}

	if (timer0DelayFlags & 0x10)
	{

		if (timer0DelayCount[4]) timer0DelayCount[4]--; // bump timer delay value down by one tick
		else
		{

			timer0DelayFlags &= ~(0x10); // signal to main program that delay timer has completed main program request
			if (timer0DisplayDelayFlags & 0x10) // if this was a display delay
			{

				timer0DisplayDelayFlags &= ~(0x10); // clear display delay flag
				if (timer0DisplayDelayFlags == 0) timer0Status |= (t0sUpdateDisplay); // signal to main program to update display

			}

		}

	}

	if (timer0DelayFlags & 0x20)
	{

		if (timer0DelayCount[5]) timer0DelayCount[5]--; // bump timer delay value down by one tick
		else
		{

			timer0DelayFlags &= ~(0x20); // signal to main program that delay timer has completed main program request
			if (timer0DisplayDelayFlags & 0x20) // if this was a display delay
			{

				timer0DisplayDelayFlags &= ~(0x20); // clear display delay flag
				if (timer0DisplayDelayFlags == 0) timer0Status |= (t0sUpdateDisplay); // signal to main program to update display

			}

		}

	}

	if (timer0DelayFlags & 0x40)
	{

		if (timer0DelayCount[6]) timer0DelayCount[6]--; // bump timer delay value down by one tick
		else
		{

			timer0DelayFlags &= ~(0x40); // signal to main program that delay timer has completed main program request
			if (timer0DisplayDelayFlags & 0x40) // if this was a display delay
			{

				timer0DisplayDelayFlags &= ~(0x40); // clear display delay flag
				if (timer0DisplayDelayFlags == 0) timer0Status |= (t0sUpdateDisplay); // signal to main program to update display

			}

		}

	}

	if (timer0DelayFlags & 0x80)
	{

		if (timer0DelayCount[7]) timer0DelayCount[7]--; // bump timer delay value down by one tick
		else
		{

			timer0DelayFlags &= ~(0x80); // signal to main program that delay timer has completed main program request
			if (timer0DisplayDelayFlags & 0x80) // if this was a display delay
			{

				timer0DisplayDelayFlags &= ~(0x80); // clear display delay flag
				if (timer0DisplayDelayFlags == 0) timer0Status |= (t0sUpdateDisplay); // signal to main program to update display

			}

		}

	}

	if (timer0Command & t0cInputReceived)
	{

		timer0Command &= ~(t0cInputReceived);
		awakeFlags |= (aAwakeOnInput);
		inputTimeoutCount = volatileVariables[(uint16_t)(vButtonTimeoutIdx)];
		activityFlags &= ~(afUserInputFlag | afActivityTimeoutFlag);

	}

	if (awakeFlags & aAwakeOnInput)
	{

		if (inputTimeoutCount) inputTimeoutCount--;
		else
		{

			awakeFlags &= ~(aAwakeOnInput);
			activityFlags |= (afUserInputFlag);

		}

	}

	if ((activityFlags & afParkCheckFlags) == afNotParkedFlags) // if MPGuino has engine stop and vehicle stop flags set, but is not yet parked
	{

		if (parkTimeoutCount) parkTimeoutCount--; // run down park watchdog timer until it expires
		else activityFlags |= (afParkFlag); // set vehicle parked flag

	}

	if ((activityFlags & afValidFlags) == afActivityCheckFlags) // if there is no activity but the activity watchdog hasn't timed out yet
	{

		if (activityTimeoutCount) activityTimeoutCount--; // cycle down the activity timeout watchdog
		else activityFlags |= (afActivityTimeoutFlag); // signal that MPGuino is in a period of inactivity

	}

	previousActivity ^= (activityFlags & afValidFlags); // detect any activity change since last timer0 tick

	if (previousActivity) activityChangeFlags |= (previousActivity); // if there was any activity change at all, signal that the display needs updating

	// reset activity timeout watchdog if any of the fuel injector, VSS pulse, button press, or park flags have changed
	if (previousActivity & afActivityCheckFlags) activityTimeoutCount = volatileVariables[(uint16_t)(vActivityTimeoutIdx)];

	// reset park timeout watchdog if any of the fuel injector or VSS pulse flags have changed
	if (previousActivity & afNotParkedFlags) parkTimeoutCount = volatileVariables[(uint16_t)(vParkTimeoutIdx)];

	previousActivity = (activityFlags & afValidFlags); // save for next timer0 tick

#if defined(useAnalogRead)
	if (analogCommand & acSampleChannelActive)
	{

		if (analogStatus & asHardwareReady)
		{

			analogCommand |= (acSampleGround); // signal to ADC interrupt that the last requested conversion was for internal ground
			analogStatus &= ~(asHardwareReady);

			ADMUX = pgm_read_byte(&analogChannelValue[(uint16_t)(analogGroundIdx)]);
			ADCSRA |= ((1 << ADSC) | (1 << ADIF) | (1 << ADIE)); // start ADC read, enable interrupt, and clear interrupt flag, because this crappy hardware allows the ADC interrupt to alway do free running mode

		}

	}

#endif // useAnalogRead
#if defined(useDebugCPUreading)
	volatileVariables[(uint16_t)(vInterruptAccumulatorIdx)] += TCNT0;

#endif // defined(useDebugCPUreading)
}

#if defined(useTimer1Interrupt)
// this ISR gets called every time timer 1 overflows.
//
// f(phase correct PWM) = f(system clock) / (N * 510)
//
// N - prescaler, which is 1
//
// so this ISR gets called every 510 * 1 / (system clock) seconds (for 20 MHz clock, that is every 25.5 us)
ISR( TIMER1_OVF_vect ) // LCD delay interrupt handler
{

#if defined(useBluetoothAdaFruitSPI)
	static uint16_t responseDelay;
	static uint16_t chipSelectDelay;
	static uint16_t resetDelay;
	static uint8_t chipSelectState;
	static uint8_t resetState;
#endif // defined(useBluetoothAdaFruitSPI)
#if defined(use4BitLCD)
	static uint8_t value;
#endif // defined(use4BitLCD)
#if defined(useSimulatedFIandVSS)
	static uint32_t debugVSSresetCount;
	static uint32_t debugFIPresetCount;
#endif // defined(useSimulatedFIandVSS)
#if defined(useDebugCPUreading)
	uint8_t a;
	uint8_t b;
	uint16_t c;

	a = TCNT0; // do a microSeconds() - like read to determine interrupt length in cycles
#endif // defined(useDebugCPUreading)

	if (timer1Command & t1cResetTimer)
	{

		timer1Command &= ~(t1cResetTimer);
		timer1Status = 0;
#if defined(useBluetoothAdaFruitSPI)
		chipSelectState = 0;
		resetState = 0;
#endif // defined(useBluetoothAdaFruitSPI)
#if defined(useSimulatedFIandVSS)
		debugVSScount = 0;
		debugFIPcount = 0;
		debugFIPWcount = 0;
		debugVSSresetCount = 0;
		debugFIPresetCount = 0;
#endif // defined(useSimulatedFIandVSS)

	}

#if defined(useBluetoothAdaFruitSPI)
	if (bleStatus & bleReset) // if main program requests bluetooth hardware reset
	{

		bleStatus &= ~(bleReset | bleAssertFlags | blePacketWaitFlags); // clear any in-progress lesser waiting tasks
		resetState = 1; // initialize reset state machine
		chipSelectState = 0; // halt CS state machine

	}

	if (bleStatus & bleAssert) // if main program requests to assert /CS
	{

		if ((bleStatus & bleResetting) == 0) // wait until reset is complete
		{

			bleStatus &= ~(bleAssert); // acknowledge main program command
			chipSelectState = 1; // initialize CS state machine

		}

	}

	if (bleStatus & blePacketWait) // if main program requests waiting for a SDEP packet wait delay
	{

		if ((bleStatus & bleResetting) == 0) // wait until reset is complete
		{

			bleStatus &= ~(blePacketWait); // acknowledge main program command
			responseDelay = delay1Tick250ms; // initialize response delay wait counter

		}

	}

	if (bleStatus & bleResetting) // if hardware reset is in progress
	{

		switch (resetState)
		{

			case 1: // release /CS pin and assert /RST pin
				blefriend::releaseCS();
#if defined(__AVR_ATmega32U4__)
				PORTD &= ~(1 << PORTD4);
#endif // defined(__AVR_ATmega32U4__)
#if defined(__AVR_ATmega2560__)
				PORTG &= ~(1 << PORTG5);
#endif // defined(__AVR_ATmega2560__)
#if defined(__AVR_ATmega328P__)
				PORTD &= ~(1 << PORTD4);
#endif // defined(__AVR_ATmega328P__)
				resetDelay = delay1Tick10ms; // cause MPGuino to assert /RST for 10 ms
				resetState++;
				break;

			case 3: // release /RST pin
#if defined(__AVR_ATmega32U4__)
				PORTD |= (1 << PORTD4);
#endif // defined(__AVR_ATmega32U4__)
#if defined(__AVR_ATmega2560__)
				PORTG |= (1 << PORTG5);
#endif // defined(__AVR_ATmega2560__)
#if defined(__AVR_ATmega328P__)
				PORTD |= (1 << PORTD4);
#endif // defined(__AVR_ATmega328P__)
				resetDelay = delay1Tick1s; // cause MPGuino to wait on just-reset BLE hardware for 1 sec
				resetState++;
				break;

			case 4:
			case 2: // perform /RST delay
				if (resetDelay) resetDelay--;
				else resetState++;
				break;

			default: // catch invalid reset states
				bleStatus &= ~(bleResetting); // mark hardware reset as completed
				break;

		}

	}

	if (bleStatus & bleAsserting) // if /CS assertion is in progress
	{

		switch (chipSelectState)
		{

			case 1: // release /CS pin if it's asserted
				if (blefriend::isCSreleased()) chipSelectState += 2; // if /CS is not asserted, skip ahead
				else // otherwise, release /CS and wait
				{

					blefriend::releaseCS();
					chipSelectDelay = delay1Tick50us; // reset /CS delay timer
					chipSelectState++;
					break;

				}

			case 3: // assert /CS pin
				blefriend::assertCS();
				chipSelectDelay = delay1Tick100us; // reset CS delay timer
				chipSelectState++;
				break;

			case 4:
			case 2: // perform /CS delay
				if (chipSelectDelay) chipSelectDelay--;
				else chipSelectState++;
				break;

			default: // catch invalid chip select states
				bleStatus &= ~(bleAsserting); // mark chip select assert as completed
				break;

		}

	}

	if (bleStatus & blePacketWaiting) // if response delay is in progress
	{

		if (responseDelay) responseDelay--; // if response delay counter still valid, bump down by one
		else
		{

			bleStatus &= ~(blePacketWaiting); // otherwise, signal that response delay timed out
			blefriend::releaseCS();

		}

	}

#endif // defined(useBluetoothAdaFruitSPI)
#if defined(useSimulatedFIandVSS)
	if (timer1Command & t1cEnableDebug)
	{

		if ((debugFlags & debugVSreadyFlags) == debugVSreadyFlags) // if VSS simulator is ready to output
		{

			if (debugVSScount) debugVSScount--;
			else
			{

				debugVSScount = debugVSStickLength;
#if defined(__AVR_ATmega32U4__)
				PORTB ^= (1 << PORTB7); // generate VSS pin interrupt
#endif // defined(__AVR_ATmega32U4__)
#if defined(__AVR_ATmega2560__)
				PORTK ^= (1 << PORTK0); // generate VSS pin interrupt
#endif // defined(__AVR_ATmega2560__)
#if defined(__AVR_ATmega328P__)
				PORTC ^= (1 << PORTC0); // generate VSS pin interrupt
#endif // defined(__AVR_ATmega328P__)

			}

		}

		if (debugFlags & debugVSSflag) // if VSS simulator is enabled
		{

			if (debugVSSresetCount) debugVSSresetCount--;
			else
			{

				debugVSSresetCount = debugVSSresetLength;
				timer1Status |= (t1sDebugUpdateVSS);

			}

		}

		if ((debugFlags & debugFIreadyFlags) == debugFIreadyFlags) // if fuel injector simulator is ready to output
		{

			if (debugFIPcount)
			{

				debugFIPcount--;

				if (debugFIPWcount) debugFIPWcount--;
				else
				{

#if defined(__AVR_ATmega32U4__)
					PORTD |= ((1 << PORTD3) | (1 << PORTD2)); // drive injector sense pin high to generate injector closed interrupt
#endif // defined(__AVR_ATmega32U4__)
#if defined(__AVR_ATmega2560__)
					PORTE |= ((1 << PORTE4) | (1 << PORTE5)); // drive injector sense pin high to generate injector closed interrupt
#endif // defined(__AVR_ATmega2560__)
#if defined(__AVR_ATmega328P__)
					PORTD |= ((1 << PORTD3) | (1 << PORTD2)); // drive injector sense pin high to generate injector closed interrupt
#endif // defined(__AVR_ATmega328P__)

				}

			}
			else
			{

				debugFIPcount = debugFIPtickLength;
				debugFIPWcount = debugFIPWtickLength;
				if (debugFIPWtickLength) // if DFCO is not commanded
				{

#if defined(__AVR_ATmega32U4__)
					PORTD &= ~((1 << PORTD3) | (1 << PORTD2)); // drive injector sense pin low to generate injector open interrupt
#endif // defined(__AVR_ATmega32U4__)
#if defined(__AVR_ATmega2560__)
					PORTE &= ~((1 << PORTE4) | (1 << PORTE5)); // drive injector sense pin low to generate injector open interrupt
#endif // defined(__AVR_ATmega2560__)
#if defined(__AVR_ATmega328P__)
					PORTD &= ~((1 << PORTD3) | (1 << PORTD2)); // drive injector sense pin low to generate injector open interrupt
#endif // defined(__AVR_ATmega328P__)

				}

			}

		}

		if (debugFlags & debugInjectorFlag) // if injector simulator is enabled
		{

			if (debugFIPresetCount) debugFIPresetCount--;
			else
			{

				debugFIPresetCount = debugFIPresetLength;
				timer1Status |= (t1sDebugUpdateFIP);

			}

		}

	}

#endif // defined(useSimulatedFIandVSS)
#if defined(useLCDoutput)
	if (timer1Command & t1cDelayLCD)
	{

		if (lcdDelayCount) lcdDelayCount--;
#if defined(useLCDbufferedOutput)
		else
		{

			if (ringBuffer::isBufferNotEmpty(rbIdxLCD)) // if there's at least one nybble in the LCD send buffer
			{

#if defined(useTWI4BitLCD)
				// if buffer is not empty and TWI hardware is ready
				if ((twiStatusFlags & twiOpenMain) == 0)
				{

					timer1Status &= ~(t1sDoOutputTWI); // reset TWI master transmission in progress flag
					timer1Status |= (t1sLoopFlag); // set loop flag

					do
					{

						value = ringBuffer::pull(rbIdxLCD); // pull a buffered LCD nybble

						if (value & lcdSendNybble) // if this nybble is to be sent out
						{

							if ((timer1Status & t1sDoOutputTWI) == 0) // if this is the first nybble to be output
							{

								TWI::openChannel(lcdAddress, TW_WRITE); // open TWI as master transmitter
#if defined(useAdafruitRGBLCDshield)
								TWI::writeByte(MCP23017_B1_OLATB); // specify bank B output latch register address
#endif // defined(useAdafruitRGBLCDshield)
								timer1Status |= (t1sDoOutputTWI); // signal to complete TWI master transmission

							}

						}

						LCD::outputNybble(value); // output the nybble and set timing

						if (value & lcdSendNybble) // if this nybble is to be sent out
						{

							if ((value & lcdSendFlags) == lcdSendNybble) // if sending an ordinary data nybble, check if we can continue looping
							{

								if ((twiDataBufferSize - twiDataBufferLen) < 5) timer1Status &= ~(t1sLoopFlag); // if TWI send buffer is getting low, signal end of loop
								if (ringBuffer::isBufferEmpty(rbIdxLCD)) timer1Status &= ~(t1sLoopFlag); // if LCD send buffer is empty, signal end of loop

							}
							else timer1Status &= ~(t1sLoopFlag); // otherwise, this is a special (command or reset) nybble, so signal end of loop

						}
						else timer1Status &= ~(t1sLoopFlag); // otherwise, this is just a delay request, so signal end of loop

					}
					while (timer1Status & t1sLoopFlag);

					if (timer1Status & t1sDoOutputTWI) TWI::transmitChannel(TWI_STOP); // commit LCD port expander write, if required

				}

#endif // defined(useTWI4BitLCD)
#if defined(usePort4BitLCD)
				value = ringBuffer::pull(rbIdxLCD); // pull a buffered LCD byte

				LCD::outputNybble(value); // output byte

#endif // defined(usePort4BitLCD)
			}
			else timer1Command &= ~(t1cDelayLCD); // turn off LCD delay

		}

#else // defined(useLCDbufferedOutput)
		else timer1Command &= ~(t1cDelayLCD); // turn off LCD delay

#endif // defined(useLCDbufferedOutput)
	}

#endif // defined(useLCDoutput)
#if defined(useDebugCPUreading)
	b = TCNT0; // do a microSeconds() - like read to determine interrupt length in cycles

	if (b < a) c = 256 - a + b; // an overflow occurred
	else c = b - a;

	volatileVariables[(uint16_t)(vInterruptAccumulatorIdx)] += c;

#endif // defined(useDebugCPUreading)
}

#endif // defined(useTimer1Interrupt)
volatile unsigned long thisInjectorOpenStart;
volatile unsigned long thisEnginePeriodOpen; // engine speed measurement based on fuel injector open event
volatile unsigned long thisEnginePeriodClose; // engine speed measurement based on fuel injector close event

// fuel injector monitor interrupt pair
//
// this pair is responsible to measure fuel injector open pulse width, and engine speed
//
// the fuel injector monitor also performs a few sanity checks
//
// sanity check 1 - the engine revolution period measured must be less than the calculated period corresponding to the minimum acceptable engine RPM speed
//                  if this measured period is greater, then the fuel injector is assumed to be de-energized
//                   - the fuel injector pulse measurement is abandoned
//                   - the engine is also assumed to be turned off (for EOC mode)
//
// sanity check 2 - if a successful pulse measurement is made, the (measured pulse + injector open delay + injector close delay) must be less than the measured engine revolution period
//                  if this is not the case, the fuel injector is operating past its design duty cycle (typically 85% at 7000 RPM or something)
//                   - MPGuino may no longer be able to reliably measure fuel consumption
//                   - the main program is informed

// injector opening event handler
//
// this measures the start of the fuel injector pulse, and is used to calculate engine speed
//
#if defined(__AVR_ATmega32U4__)
ISR( INT2_vect )
#endif // defined(__AVR_ATmega32U4__)
#if defined(__AVR_ATmega2560__)
ISR( INT4_vect )
#endif // defined(__AVR_ATmega2560__)
#if defined(__AVR_ATmega328P__)
ISR( INT0_vect )
#endif // defined(__AVR_ATmega328P__)
{

	static unsigned long lastInjectorOpenStart;
	unsigned int a;
#if defined(useDebugCPUreading)
	unsigned int b;
#endif // defined(useDebugCPUreading)

	a = (unsigned int)(TCNT0); // do a microSeconds() - like read to determine loop length in cycles
	if (TIFR0 & (1 << TOV0)) a = (unsigned int)(TCNT0) + 256; // if overflow occurred, re-read with overflow flag taken into account

	thisInjectorOpenStart = timer0_overflow_count + (unsigned long)(a);

	if (dirty & dGoodEngineRotationOpen) thisEnginePeriodOpen = heart::findCycle0Length(lastInjectorOpenStart, thisInjectorOpenStart); // calculate length between fuel injector pulse starts
	else thisEnginePeriodOpen = 0;

#if defined(useChryslerMAPCorrection)
	dirty |= (dGoodEngineRotationOpen | dInjectorReadInProgress | dSampleADC);
#else // defined(useChryslerMAPCorrection)
	dirty |= (dGoodEngineRotationOpen | dInjectorReadInProgress);
#endif // defined(useChryslerMAPCorrection)

	lastInjectorOpenStart = thisInjectorOpenStart;

	watchdogInjectorCount = volatileVariables[(uint16_t)(vDetectEngineOffIdx)]; // reset minimum engine speed watchdog timer

#if defined(useDebugCPUreading)
	b = (unsigned int)(TCNT0); // do a microSeconds() - like read to determine loop length in cycles
	if (TIFR0 & (1 << TOV0)) b = (unsigned int)(TCNT0) + 256; // if overflow occurred, re-read with overflow flag taken into account

	volatileVariables[(uint16_t)(vInterruptAccumulatorIdx)] += b - a;

#endif // defined(useDebugCPUreading)
}

// injector opening event handler
//
// this measures the end of the fuel injector pulse, and is used to calculate engine speed
//
// if a fuel injector pulse width measurement is in progress, this also performs the measurement and stores raw fuel consumption data
//
// it will either store one of the good existing engine period measurements or an average of both if both are good
//
#if defined(__AVR_ATmega32U4__)
ISR( INT3_vect )
#endif // defined(__AVR_ATmega32U4__)
#if defined(__AVR_ATmega2560__)
ISR( INT5_vect )
#endif // defined(__AVR_ATmega2560__)
#if defined(__AVR_ATmega328P__)
ISR( INT1_vect )
#endif // defined(__AVR_ATmega328P__)
{

	static unsigned long lastInjectorCloseStart;

	uint8_t b;
	unsigned int a;
#if defined(useDebugCPUreading)
	unsigned int c;
#endif // defined(useDebugCPUreading)
	unsigned long thisInjectorCloseStart;
	unsigned long engineRotationPeriod;
	unsigned long thisInjectorPulseLength;
	unsigned long goodInjectorPulseLength;

	a = (unsigned int)(TCNT0); // do a microSeconds() - like read to determine loop length in cycles
	if (TIFR0 & (1 << TOV0)) a = (unsigned int)(TCNT0) + 256; // if overflow occurred, re-read with overflow flag taken into account

	thisInjectorCloseStart = timer0_overflow_count + (unsigned long)(a);

	if (dirty & dGoodEngineRotationClose) thisEnginePeriodClose = heart::findCycle0Length(lastInjectorCloseStart, thisInjectorCloseStart); // calculate length between fuel injector pulse starts
	else thisEnginePeriodClose = 0;

	if (dirty & dInjectorReadInProgress) // if there was a fuel injector open pulse detected, there's now a fuel injector pulse width to be measured
	{

		dirty &= ~(dInjectorReadInProgress);

		b = (dirty & dGoodEngineRotation);

		switch (b)
		{

			case (dGoodEngineRotationClose):
				engineRotationPeriod = thisEnginePeriodClose;
				b = dGoodInjectorRead;
				break;

			case (dGoodEngineRotationOpen):
				engineRotationPeriod = thisEnginePeriodOpen;
				b = dGoodInjectorRead;
				break;

			case (dGoodEngineRotation):
				engineRotationPeriod = thisEnginePeriodClose + thisEnginePeriodOpen;
				engineRotationPeriod++; // perform pre-emptive rounding up from averaging operation
				engineRotationPeriod >>= 1; // perform average of two measurements
				b = dGoodInjectorRead;
				break;

			default:
				b = 0;
				break;

		}

		// calculate fuel injector pulse length
		thisInjectorPulseLength = heart::findCycle0Length(thisInjectorOpenStart, thisInjectorCloseStart) - volatileVariables[(uint16_t)(vInjectorOpenDelayIdx)]; // strip off injector open delay time

		// if this pulse is larger than the maximum good pulse that could happen at the minimum valid engine speed, reject it
		// 1 - pulse could be narrower than vInjectorOpenDelayIdx
		// 2 - pulse could be wider than the maximum allowable pulse width for minimum good engine speed
		if (thisInjectorPulseLength > volatileVariables[(uint16_t)(vInjectorValidMaxWidthIdx)]) dirty &= ~(dGoodInjectorWidth | dGoodInjectorRead);
		else dirty |= (dGoodInjectorWidth);

		if (b) // if we have an engine rotation period measurement
		{

			// calculate good maximum fuel injector open time for injector pulse width sanity check
			goodInjectorPulseLength = engineRotationPeriod - volatileVariables[(uint16_t)(vInjectorOpenDelayIdx)];

			if (thisInjectorPulseLength > goodInjectorPulseLength) dirty &= ~(dGoodInjectorRead); // if measured pulse is larger than largest good pulse, signal that last injector read may be bad
			else dirty |= (dGoodInjectorRead); // signal that last injector read is good

			// if measured engine speed is greater than the specified minimum good engine speed
			if (engineRotationPeriod < volatileVariables[(uint16_t)(vMaximumEnginePeriodIdx)])
			{

				activityFlags &= ~(afEngineOffFlag | afParkFlag | afActivityTimeoutFlag); // signal that engine is running, and vehicle is therefore no longer parked
				awakeFlags |= (aAwakeEngineRunning); // MPGuino is awake due to engine running

			}

#if defined(trackIdleEOCdata)
			if (awakeFlags & aAwakeVehicleMoving) // if vehicle is moving
				// add to raw fuel injector total cycle accumulator
				tripVar::update64(collectedEngCycleCount, engineRotationPeriod, curRawTripIdx);
			else // if vehicle is not moving
				// add to raw idle fuel injector total cycle accumulator
				tripVar::update64(collectedEngCycleCount, engineRotationPeriod, curRawEOCidleTripIdx);

#else // defined(trackIdleEOCdata)
			// add to raw fuel injector total cycle accumulator
			tripVar::update64(collectedEngCycleCount, engineRotationPeriod, curRawTripIdx);

#endif // defined(trackIdleEOCdata)
#if defined(useDragRaceFunction)
			if (accelerationFlags & accelTestActive)
			{

				// add to raw accel test distance fuel injector total cycle accumulator
				if (accelerationFlags & accelTestDistance) tripVar::update64(collectedEngCycleCount, engineRotationPeriod, dragRawDistanceIdx);

				// add to raw accel test full speed fuel injector total cycle accumulator
				if (accelerationFlags & accelTestFullSpeed) tripVar::update64(collectedEngCycleCount, engineRotationPeriod, dragRawFullSpeedIdx);

				// add to raw accel test half speed fuel injector total cycle accumulator
				if (accelerationFlags & accelTestHalfSpeed) tripVar::update64(collectedEngCycleCount, engineRotationPeriod, dragRawHalfSpeedIdx);

			}

#endif // defined(useDragRaceFunction)
		}

		// if the injector pulse width is valid
		if (dirty & dGoodInjectorWidth)
		{

			awakeFlags |= (aAwakeOnInjector); // signal that MPGuino is awake due to detected injector

#if defined(useChryslerMAPCorrection)
			thisInjectorPulseLength *= volatileVariables[(uint16_t)(vInjectorCorrectionIdx)]; // multiply by differential fuel pressure correction factor numerator
			thisInjectorPulseLength >>= 12; // divide by differential fuel pressure correction factor denominator

#endif // defined(useChryslerMAPCorrection)
#if defined(trackIdleEOCdata)
			if (awakeFlags & aAwakeVehicleMoving) // if vehicle is moving
				// update fuel injector open cycle accumulator, and fuel injector pulse count
				tripVar::update64(collectedInjCycleCount, collectedInjPulseCount, thisInjectorPulseLength, curRawTripIdx);
			else // if vehicle is not moving
				// update idle fuel injector open cycle accumulator, and idle fuel injector pulse count
				tripVar::update64(collectedInjCycleCount, collectedInjPulseCount, thisInjectorPulseLength, curRawEOCidleTripIdx); 

#else // defined(trackIdleEOCdata)
			// update fuel injector open cycle accumulator, and fuel injector pulse count
			tripVar::update64(collectedInjCycleCount, collectedInjPulseCount, thisInjectorPulseLength, curRawTripIdx);

#endif // defined(trackIdleEOCdata)
#if defined(useDragRaceFunction)
			if (accelerationFlags & accelTestActive)
			{

				// update raw accel test distance fuel injector open cycle accumulator, and raw accel test distance fuel injector pulse count
				if (accelerationFlags & accelTestDistance)
					tripVar::update64(collectedInjCycleCount, collectedInjPulseCount, thisInjectorPulseLength, dragRawDistanceIdx);

				// update raw accel test full speed fuel injector open cycle accumulator, and raw accel test full speed fuel injector pulse count
				if (accelerationFlags & accelTestFullSpeed)
					tripVar::update64(collectedInjCycleCount, collectedInjPulseCount, thisInjectorPulseLength, dragRawFullSpeedIdx);

				// update raw accel test half speed fuel injector open cycle accumulator, and raw accel test half speed fuel injector pulse count
				if (accelerationFlags & accelTestHalfSpeed)
					tripVar::update64(collectedInjCycleCount, collectedInjPulseCount, thisInjectorPulseLength, dragRawHalfSpeedIdx);

			}

#endif // defined(useDragRaceFunction)
		}

	}

	dirty |= (dGoodEngineRotationClose);
	lastInjectorCloseStart = thisInjectorCloseStart;

	watchdogInjectorCount = volatileVariables[(uint16_t)(vDetectEngineOffIdx)]; // reset minimum engine speed watchdog timer

#if defined(useDebugCPUreading)
	c = (unsigned int)(TCNT0); // do a microSeconds() - like read to determine loop length in cycles
	if (TIFR0 & (1 << TOV0)) c = (unsigned int)(TCNT0) + 256; // if overflow occurred, re-read with overflow flag taken into account

	volatileVariables[(uint16_t)(vInterruptAccumulatorIdx)] += c - a;

#endif // defined(useDebugCPUreading)
}

#if defined(__AVR_ATmega32U4__)
ISR( PCINT0_vect )
#endif // defined(__AVR_ATmega32U4__)
#if defined(__AVR_ATmega2560__)
ISR( PCINT2_vect )
#endif // defined(__AVR_ATmega2560__)
#if defined(__AVR_ATmega328P__)
ISR( PCINT1_vect )
#endif // defined(__AVR_ATmega328P__)
{

	uint8_t p;
	uint8_t q;

	unsigned int a;
#if defined(useDebugCPUreading)
	unsigned int c;
#endif // defined(useDebugCPUreading)
	unsigned long thisTime;

	a = (unsigned int)(TCNT0); // do a microSeconds() - like read to determine loop length in cycles
	if (TIFR0 & (1 << TOV0)) a = (unsigned int)(TCNT0) + 256; // if overflow occurred, re-read with overflow flag taken into account

	thisTime = timer0_overflow_count + (unsigned long)(a);

#if defined(__AVR_ATmega32U4__)
	p = PINB; // read current input pin
#endif // defined(__AVR_ATmega32U4__)
#if defined(__AVR_ATmega2560__)
	p = PINK; // read current input pin
#endif // defined(__AVR_ATmega2560__)
#if defined(__AVR_ATmega328P__)
	p = PINC; // read current input pin
#endif // defined(__AVR_ATmega328P__)
	q = p ^ lastPINxState; // detect any changes from the last time this ISR is called

#if defined(__AVR_ATmega32U4__)
	if (q & (1 << PINB7)) // if a VSS pulse is received
#endif // defined(__AVR_ATmega32U4__)
#if defined(__AVR_ATmega2560__)
	if (q & (1 << PINK0)) // if a VSS pulse is received
#endif // defined(__AVR_ATmega2560__)
#if defined(__AVR_ATmega328P__)
	if (q & (1 << PINC0)) // if a VSS pulse is received
#endif // defined(__AVR_ATmega328P__)
	{

		if (VSSpause) VSScount = VSSpause; // if there is a VSS debounce count defined, set VSS debounce count and let system timer handle the debouncing
		else heart::updateVSS(thisTime); // otherwise, go process VSS pulse

	}

#if defined(useLegacyButtons)
	if (q & buttonMask) buttonDebounceCount = delay0Tick50ms; // if a button change was detected, set button press debounce count, and let system timer handle the debouncing

#endif // defined(useLegacyButtons)
	lastPINxState = p; // remember the current input pin state for the next time this ISR gets called

#if defined(useDebugCPUreading)
	c = (unsigned int)(TCNT0); // do a microSeconds() - like read to determine loop length in cycles
	if (TIFR0 & (1 << TOV0)) c = (unsigned int)(TCNT0) + 256; // if overflow occurred, re-read with overflow flag taken into account

	volatileVariables[(uint16_t)(vInterruptAccumulatorIdx)] += c - a;

#endif // defined(useDebugCPUreading)
}

#if defined(useBuffering)
static void ringBuffer::init(void)
{

	uint8_t oldSREG;

	oldSREG = SREG; // save interrupt flag status
	cli(); // disable interrupts

	for (uint8_t x = 0; x < rbIdxCount; x++)
	{

		ringBufferDef[(uint16_t)(x)].data = (uint8_t *)(pgm_read_word(&ringBufferDefList[(uint16_t)(x)].data));
		ringBufferDef[(uint16_t)(x)].size = pgm_read_word(&ringBufferDefList[(uint16_t)(x)].size);
		ringBufferDef[(uint16_t)(x)].start = 0;
		ringBufferDef[(uint16_t)(x)].end = 0;
		ringBufferDef[(uint16_t)(x)].status = (bufferIsEmpty);

	}

	SREG = oldSREG; // restore interrupt flag status

}

static uint8_t ringBuffer::isBufferEmpty(uint8_t ringBufferIdx)
{

	return (ringBufferDef[(uint16_t)(ringBufferIdx)].status & bufferIsEmpty);

}

static uint8_t ringBuffer::isBufferNotEmpty(uint8_t ringBufferIdx)
{

	return ((ringBufferDef[(uint16_t)(ringBufferIdx)].status & bufferIsEmpty) == 0);

}

static uint8_t ringBuffer::isBufferFull(uint8_t ringBufferIdx)
{

	return (ringBufferDef[(uint16_t)(ringBufferIdx)].status & bufferIsFull);

}

static void ringBuffer::pushMain(uint8_t ringBufferIdx, uint8_t value)
{

	uint8_t oldSREG;

	while (ringBufferDef[(uint16_t)(ringBufferIdx)].status & bufferIsFull) idleProcess(); // wait for calling routine's buffer to become not full

	oldSREG = SREG; // save interrupt flag status
	cli(); // disable interrupts

	push(ringBufferIdx, value);

	SREG = oldSREG; // restore interrupt flag status

}

static uint8_t ringBuffer::pullMain(uint8_t ringBufferIdx)
{

	uint8_t value;
	uint8_t oldSREG;

	oldSREG = SREG; // save interrupt flag status
	cli(); // disable interrupts

	value = pull(ringBufferIdx);

	SREG = oldSREG; // restore interrupt flag status

	return value;

}

static uint16_t ringBuffer::lengthMain(uint8_t ringBufferIdx)
{

	uint16_t i;
	uint8_t oldSREG;

	oldSREG = SREG; // save interrupt flag status
	cli(); // disable interrupts

	i = length(ringBufferIdx);

	SREG = oldSREG; // restore interrupt flag status

	return i;

}

static uint16_t ringBuffer::freeMain(uint8_t ringBufferIdx)
{

	uint16_t i;
	uint8_t oldSREG;

	oldSREG = SREG; // save interrupt flag status
	cli(); // disable interrupts

	i = free(ringBufferIdx);

	SREG = oldSREG; // restore interrupt flag status

	return i;

}

static void ringBuffer::flush(uint8_t ringBufferIdx)
{

	while ((ringBufferDef[(uint16_t)(ringBufferIdx)].status & bufferIsEmpty) == 0) idleProcess(); // wait for calling routine's buffer to become empty

}

static void ringBuffer::empty(uint8_t ringBufferIdx)
{

	uint8_t oldSREG;

	oldSREG = SREG; // save interrupt flag status
	cli(); // disable interrupts

	ringBufferDef[(uint16_t)(ringBufferIdx)].end = ringBufferDef[(uint16_t)(ringBufferIdx)].start;
	ringBufferDef[(uint16_t)(ringBufferIdx)].status = bufferIsEmpty;

	SREG = oldSREG; // restore interrupt flag status

}

static void ringBuffer::push(uint8_t ringBufferIdx, uint8_t value)
{

	ringBufferDef[(uint16_t)(ringBufferIdx)].data[ringBufferDef[(uint16_t)(ringBufferIdx)].start++] = value; // save a buffered character

	if (ringBufferDef[(uint16_t)(ringBufferIdx)].status & bufferIsEmpty) ringBufferDef[(uint16_t)(ringBufferIdx)].status &= ~(bufferIsEmpty); // mark buffer as no longer empty
	if (ringBufferDef[(uint16_t)(ringBufferIdx)].start == ringBufferDef[(uint16_t)(ringBufferIdx)].size) ringBufferDef[(uint16_t)(ringBufferIdx)].start = 0; // handle wrap-around
	if (ringBufferDef[(uint16_t)(ringBufferIdx)].start == ringBufferDef[(uint16_t)(ringBufferIdx)].end) ringBufferDef[(uint16_t)(ringBufferIdx)].status |= (bufferIsFull); // test if buffer is full

}

static uint8_t ringBuffer::pull(uint8_t ringBufferIdx)
{

	uint8_t value;

	if (ringBufferDef[(uint16_t)(ringBufferIdx)].status & bufferIsEmpty) value = 0; // if buffer is empty, return a NULL
	else
	{

		value = ringBufferDef[(uint16_t)(ringBufferIdx)].data[ringBufferDef[(uint16_t)(ringBufferIdx)].end++]; // pull a buffered character

		if (ringBufferDef[(uint16_t)(ringBufferIdx)].status & bufferIsFull) ringBufferDef[(uint16_t)(ringBufferIdx)].status &= ~(bufferIsFull); // mark buffer as no longer full
		if (ringBufferDef[(uint16_t)(ringBufferIdx)].end == ringBufferDef[(uint16_t)(ringBufferIdx)].size) ringBufferDef[(uint16_t)(ringBufferIdx)].end = 0; // handle wrap-around
		if (ringBufferDef[(uint16_t)(ringBufferIdx)].end == ringBufferDef[(uint16_t)(ringBufferIdx)].start) ringBufferDef[(uint16_t)(ringBufferIdx)].status |= (bufferIsEmpty); // test if buffer is empty

	}

	return value;

}

static uint16_t ringBuffer::length(uint8_t ringBufferIdx)
{

	uint16_t i;

	if (ringBufferDef[(uint16_t)(ringBufferIdx)].status & bufferIsFull) i = ringBufferDef[(uint16_t)(ringBufferIdx)].size;
	else if (ringBufferDef[(uint16_t)(ringBufferIdx)].status & bufferIsEmpty) i = 0;
	else if (ringBufferDef[(uint16_t)(ringBufferIdx)].end < ringBufferDef[(uint16_t)(ringBufferIdx)].start) i = (ringBufferDef[(uint16_t)(ringBufferIdx)].start - ringBufferDef[(uint16_t)(ringBufferIdx)].end);
	else
	{

		i = ringBufferDef[(uint16_t)(ringBufferIdx)].size - ringBufferDef[(uint16_t)(ringBufferIdx)].end;
		i += ringBufferDef[(uint16_t)(ringBufferIdx)].start;

	}

	return i;

}

static uint16_t ringBuffer::free(uint8_t ringBufferIdx)
{

	uint16_t i;

	if (ringBufferDef[(uint16_t)(ringBufferIdx)].status & bufferIsFull) i = 0;
	else if (ringBufferDef[(uint16_t)(ringBufferIdx)].status & bufferIsEmpty) i = ringBufferDef[(uint16_t)(ringBufferIdx)].size;
	else if (ringBufferDef[(uint16_t)(ringBufferIdx)].end > ringBufferDef[(uint16_t)(ringBufferIdx)].start) i = (ringBufferDef[(uint16_t)(ringBufferIdx)].end - ringBufferDef[(uint16_t)(ringBufferIdx)].start);
	else
	{

		i = ringBufferDef[(uint16_t)(ringBufferIdx)].size - ringBufferDef[(uint16_t)(ringBufferIdx)].start;
		i += ringBufferDef[(uint16_t)(ringBufferIdx)].end;

	}

	return i;

}

#endif // defined(useBuffering)
/* core MPGuino system support section */

static void heart::updateVSS(uint32_t thisVSStime)
{

	static uint32_t lastVSStime;
#if defined(useDragRaceFunction)
	static uint32_t accelTestDistanceCount;
	static uint32_t accelTestVSStime;

#endif // defined(useDragRaceFunction)
	static uint32_t cycleLength;

	if (dirty & dGoodVSSsignal) // if a valid VSS signal had previously been read in
	{

		dirty |= (dGoodVSSRead); // mark valid VSS pulse measurement
		awakeFlags |= (aAwakeOnVSS); // MPGuino is awake on valid VSS pulse measurement

		cycleLength = heart::findCycle0Length(lastVSStime, thisVSStime); // calculate VSS pulse length

		if (cycleLength < volatileVariables[(uint16_t)(vMaximumVSSperiodIdx)]) // if VSS period is less than that for minimum good vehicle speed
		{

			if (activityFlags & afVehicleStoppedFlag) // if vehicle has been previously flagged as not moving
			{

				activityFlags &= ~(afVehicleStoppedFlag | afSwapFEwithFCR | afParkFlag | afActivityTimeoutFlag); // signal that vehicle is moving, and vehicle is therefore no longer parked
				awakeFlags |= (aAwakeVehicleMoving); // MPGuino is awake on vehicle movement

			}

		}

#if defined(trackIdleEOCdata)
		if (awakeFlags & aAwakeEngineRunning) // if the engine is running
			// update raw VSS cycle accumulator, and raw VSS pulse count
			tripVar::update64(collectedVSScycleCount, collectedVSSpulseCount, cycleLength, curRawTripIdx);
		else // if the engine is not running
			// update raw EOC VSS cycle accumulator, and raw EOC VSS pulse count
			tripVar::update64(collectedVSScycleCount, collectedVSSpulseCount, cycleLength, curRawEOCidleTripIdx);

#else // defined(trackIdleEOCdata)
		// update raw VSS cycle accumulator, and raw VSS pulse count
		tripVar::update64(collectedVSScycleCount, collectedVSSpulseCount, cycleLength, curRawTripIdx);

#endif // defined(trackIdleEOCdata)
#if defined(useCoastDownCalculator)
		if (coastdownFlags & cdTestTakeSample) // if coastdown test is active, and a sample is requested
		{

			coastdownFlags &= ~(cdTestTakeSample); // acknowledge sample request
			coastdownFlags |= (cdTestSampleTaken); // signal that a sample has been taken
			volatileVariables[(uint16_t)(coastdownState)] = cycleLength; // take sample

		}

#endif // defined(useCoastDownCalculator)
#if defined(useVehicleParameters)
		if (awakeFlags & aAwakeVehicleMoving) // if vehicle is considered to be moving
		{

#if defined(useDragRaceFunction)
			if (accelerationFlags & accelTestTriggered) // if accel test function is triggered
			{

				accelerationFlags &= ~(accelTestTriggered); // switch status from 'triggered' to 'active'
				accelerationFlags |= (accelTestActive);
				timer0Status |= (t0sAccelTestFlag);

				// initialize trap distance variables
				accelTestDistanceCount = volatileVariables[(uint16_t)(vAccelDistanceValueIdx)];
				accelTestVSStime = 0;

			}

			if (accelerationFlags & accelTestActive) // if accel test function is active
			{

				if (accelerationFlags & accelTestDistance)
				{

					if (accelTestDistanceCount)
					{

						accelTestDistanceCount--; // count down drag distance setpoint in VSS pulses

						// update raw accel test distance VSS cycle accumulator, and raw accel test distance VSS pulse count
						tripVar::update64(collectedVSScycleCount, collectedVSSpulseCount, cycleLength, dragRawDistanceIdx);

						if (accelTestVSStime) // fetch largest instantaneous speed
						{

							if (cycleLength < accelTestVSStime) accelTestVSStime = cycleLength;

						}
						else accelTestVSStime = cycleLength;

					}
					else
					{

						accelerationFlags &= ~(accelTestDistance); // otherwise, mark drag function distance measurement as complete
						timer0Status |= (t0sAccelTestFlag);
						volatileVariables[(uint16_t)(vDragRawInstantSpeedIdx)] = accelTestVSStime; // store maximum recorded speed
						volatileVariables[(uint16_t)(vDragRawTrapSpeedIdx)] = cycleLength; // store trap speed

					}

				}

				if (accelerationFlags & accelTestHalfSpeed)
				{

					if (cycleLength < volatileVariables[(uint16_t)(vAccelHalfPeriodValueIdx)])
					{

						accelerationFlags &= ~(accelTestHalfSpeed); // mark drag function half speed measurement as complete
						timer0Status |= (t0sAccelTestFlag);

					}
					else
						// update raw accel test half speed VSS cycle accumulator, and raw accel test half speed VSS pulse count
						tripVar::update64(collectedVSScycleCount, collectedVSSpulseCount, cycleLength, dragRawHalfSpeedIdx);

				}

				if (accelerationFlags & accelTestFullSpeed)
				{

					if (cycleLength < volatileVariables[(uint16_t)(vAccelFullPeriodValueIdx)])
					{

						accelerationFlags &= ~(accelTestFullSpeed); // mark drag function full speed measurement as complete
						timer0Status |= (t0sAccelTestFlag);

					}
					else
						// update raw accel test full speed VSS cycle accumulator, and raw accel test full speed VSS pulse count
						tripVar::update64(collectedVSScycleCount, collectedVSSpulseCount, cycleLength, dragRawFullSpeedIdx);

				}

				if ((accelerationFlags & accelTestMeasurementFlags) == 0) // if all drag measurements have completed, mark drag function as complete
				{

					accelerationFlags &= ~(accelTestActive); // switch status from 'active' to 'finished'
					accelerationFlags |= (accelTestFinished);
					timer0Status |= (t0sAccelTestFlag);

				}

			}

#endif // defined(useDragRaceFunction)
		}

#endif // defined(useVehicleParameters)
	}

	dirty |= dGoodVSSsignal; // annotate that a valid VSS pulse has been read
	watchdogVSSCount = volatileVariables[(uint16_t)(vDetectVehicleStopIdx)]; // reset minimum engine speed watchdog timer
	lastVSStime = thisVSStime;

}

static void heart::initCore(void)
{

	uint8_t oldSREG;

	oldSREG = SREG; // save interrupt flag status
	cli(); // disable interrupts

	// timer0 is the taskmaster driving MPGuino's measurement functionality
#if defined(__AVR_ATmega32U4__)
	// turn on timer0 module
	PRR0 &= ~(1 << PRTIM0);

	// set timer 0 to fast PWM mode, TOP = 0xFF
	TCCR0A |= ((1 << WGM01) | (1 << WGM00));
	TCCR0B &= ~(1 << WGM02);

	// set timer 0 prescale factor to 64
	TCCR0B &= ~(1 << CS02);
	TCCR0B |= ((1 << CS01) | (1 << CS00));

	// set OC0A to disabled
	TCCR0A &= ~((1 << COM0A1) | (1 << COM0A0));

	// set OC0B to disabled
	TCCR0A &= ~((1 << COM0B1) | (1 << COM0B0));

	// clear timer 0 output compare force bits for OC0A and OC0B
	TCCR0B &= ~((1 << FOC0A) | (1 << FOC0B));

	// disable timer 0 output compare interrupts
	TIMSK0 &= ~((1 << OCIE0B) | (1 << OCIE0A));

	// enable timer 0 overflow interrupt to generate ~1 ms tick
	TIMSK0 |= (1 << TOIE0);

	// clear timer 0 interrupt flags
	TIFR0 |= ((1 << OCF0B) | (1 << OCF0A) | (1 << TOV0));

	// disable digital inputs for all ADC capable pins to reduce power consumption
	DIDR0 |= ((ADC7D) | (1 << ADC6D) | (1 << ADC5D) | (1 << ADC4D) | (1 << ADC1D) | (1 << ADC0D));
	DIDR1 |= (1 << AIN0D);
	DIDR2 |= ((1 << ADC13D) | (1 << ADC12D) | (1 << ADC11D) | (1 << ADC10D) | (1 << ADC9D) | (1 << ADC8D));

	// shut off on-board peripherals to reduce power consumption
	PRR0 |= ((1 << PRTWI) | (1 << PRTIM1) | (1 << PRSPI) | (1 << PRADC));
	PRR1 |= ((1 << PRUSB) | (1 << PRTIM4) | (1 << PRTIM3) | (1 << PRUSART1));

#endif // defined(__AVR_ATmega32U4__)
#if defined(__AVR_ATmega2560__)
	// turn on timer0 module
	PRR0 &= ~(1 << PRTIM0);

	// set timer 0 to fast PWM mode, TOP = 0xFF
	TCCR0A |= ((1 << WGM01) | (1 << WGM00));
	TCCR0B &= ~(1 << WGM02);

	// set timer 0 prescale factor to 64
	TCCR0B &= ~(1 << CS02);
	TCCR0B |= ((1 << CS01) | (1 << CS00));

	// set OC0A to disabled
	TCCR0A &= ~((1 << COM0A1) | (1 << COM0A0));

	// set OC0B to disabled
	TCCR0A &= ~((1 << COM0B1) | (1 << COM0B0));

	// clear timer 0 output compare force bits for OC0A and OC0B
	TCCR0B &= ~((1 << FOC0A) | (1 << FOC0B));

	// disable timer 0 output compare interrupts
	TIMSK0 &= ~((1 << OCIE0B) | (1 << OCIE0A));

	// enable timer 0 overflow interrupt to generate ~1 ms tick
	TIMSK0 |= (1 << TOIE0);

	// clear timer 0 interrupt flags
	TIFR0 |= ((1 << OCF0B) | (1 << OCF0A) | (1 << TOV0));

	// disable digital inputs for all ADC capable pins to reduce power consumption
	DIDR0 |= ((1 << ADC7D) | (1 << ADC6D) | (1 << ADC5D) | (1 << ADC4D) | (1 << ADC3D) | (1 << ADC2D) | (1 << ADC1D) | (1 << ADC0D));
	DIDR1 |= ((1 << AIN1D) | (1 << AIN0D));
	DIDR2 |= ((1 << ADC15D) | (1 << ADC14D) | (1 << ADC13D) | (1 << ADC12D) | (1 << ADC11D) | (1 << ADC10D) | (1 << ADC9D) | (1 << ADC8D));

	// shut off on-board peripherals to reduce power consumption
	PRR0 |= ((1 << PRTWI) | (1 << PRTIM2) | (1 << PRTIM1) | (1 << PRSPI) | (1 << PRUSART0) | (1 << PRADC));
	PRR1 |= ((1 << PRTIM5) | (1 << PRTIM4) | (1 << PRTIM3) | (1 << PRUSART3) | (1 << PRUSART2) | (1 << PRUSART1));

#endif // defined(__AVR_ATmega2560__)
#if defined(__AVR_ATmega328P__)
	// turn on timer0 module
	PRR &= ~(1 << PRTIM0);

	// set timer 0 to fast PWM mode, TOP = 0xFF
	TCCR0A |= ((1 << WGM01) | (1 << WGM00));
	TCCR0B &= ~(1 << WGM02);

	// set timer 0 prescale factor to 64
	TCCR0B &= ~(1 << CS02);
	TCCR0B |= ((1 << CS01) | (1 << CS00));

	// set OC0A to disabled
	TCCR0A &= ~((1 << COM0A1) | (1 << COM0A0));

	// set OC0B to disabled
	TCCR0A &= ~((1 << COM0B1) | (1 << COM0B0));

	// clear timer 0 output compare force bits for OC0A and OC0B
	TCCR0B &= ~((1 << FOC0A) | (1 << FOC0B));

	// disable timer 0 output compare interrupts
	TIMSK0 &= ~((1 << OCIE0B) | (1 << OCIE0A));

	// enable timer 0 overflow interrupt to generate ~1 ms tick
	TIMSK0 |= (1 << TOIE0);

	// clear timer 0 interrupt flags
	TIFR0 |= ((1 << OCF0B) | (1 << OCF0A) | (1 << TOV0));

	// disable digital inputs for all ADC capable pins to reduce power consumption
	DIDR0 |= ((1 << ADC5D) | (1 << ADC4D) | (1 << ADC3D) | (1 << ADC2D) | (1 << ADC1D) | (1 << ADC0D));
	DIDR1 |= ((1 << AIN1D) | (1 << AIN0D));

	// shut off on-board peripherals to reduce power consumption
	PRR |= ((1 << PRTWI) | (1 << PRTIM2) | (1 << PRTIM1) | (1 << PRSPI) | (1 << PRUSART0) | (1 << PRADC));

#endif // defined(__AVR_ATmega328P__)
	ACSR &= ~(1 << ACIE); // disable analog comparator interrupt
	ACSR |= (1 << ACD); // disable analog comparator module
	ADCSRB &= ~(1 << ACME); // disable analog comparator multiplexer

	timer0Command = t0cResetTimer;
#if defined(useTimer1Interrupt)
	timer1Command = t1cResetTimer;
#endif // defined(useTimer1Interrupt)

	SREG = oldSREG; // restore interrupt flag status

}

static void heart::initHardware(void)
{

	uint8_t oldSREG;

	oldSREG = SREG; // save interrupt flag status
	cli(); // disable interrupts

	// timer initialization section - multiple peripherals may use the same timer
#if defined(useTimer1)
#if defined(__AVR_ATmega32U4__)
	// turn on timer1 module
	PRR0 &= ~(1 << PRTIM1);

	// set timer 1 to 8-bit phase correct PWM mode, TOP = 0xFF
	TCCR1A &= ~(1 << WGM11);
	TCCR1A |= (1 << WGM10);
	TCCR1B &= ~((1 << WGM13) | (1 << WGM12));

	// set timer 1 prescale factor to 1
	TCCR1B &= ~((1 << CS12) | (1 << CS11));
	TCCR1B |= (1 << CS10);

	// disable timer 1 input capture noise canceler, select timer 1 falling edge for input capture
	TCCR1B &= ~((1 << ICNC1) | (1 << ICES1));

	// set OC1A to disabled
	TCCR1A &= ~((1 << COM1A1) | (1 << COM1A0));

	// set OC1B to disabled
	TCCR1A &= ~((1 << COM1B1) | (1 << COM1B0));

	// set OC1C to disabled
	TCCR1A &= ~((1 << COM1C1) | (1 << COM1C0));

	// clear timer 1 output compare force bits for OC1A, OC1B, and OC1C
	TCCR1C &= ~((1 << FOC1A) | (1 << FOC1B) | (1 << FOC1C));

#if defined(useTimer1Interrupt)
	// disable timer 1 interrupts
	TIMSK1 &= ~((1 << ICIE1) | (1 << OCIE1C) | (1 << OCIE1B) | (1 << OCIE1A));

	// enable timer1 overflow interrupt
	TIMSK1 |= (1 << TOIE1);
#else // defined(useTimer1Interrupt)
	// disable timer 1 interrupts
	TIMSK1 &= ~((1 << ICIE1) | (1 << OCIE1C) | (1 << OCIE1B) | (1 << OCIE1A) | (1 << TOIE1));
#endif // defined(useTimer1Interrupt)

	// clear timer 1 interrupt flags
	TIFR1 |= ((1 << ICF1) | (1 << OCF1C) | (1 << OCF1B) | (1 << OCF1A) | (1 << TOV1));

#endif // defined(__AVR_ATmega32U4__)
#if defined(__AVR_ATmega2560__)
	// turn on timer1 module
	PRR0 &= ~(1 << PRTIM1);

	// set timer 1 to 8-bit phase correct PWM mode, TOP = 0xFF
	TCCR1A &= ~(1 << WGM11);
	TCCR1A |= (1 << WGM10);
	TCCR1B &= ~((1 << WGM13) | (1 << WGM12));

	// set timer 1 prescale factor to 1
	TCCR1B &= ~((1 << CS12) | (1 << CS11));
	TCCR1B |= (1 << CS10);

	// disable timer 1 input capture noise canceler, select timer 1 falling edge for input capture
	TCCR1B &= ~((1 << ICNC1) | (1 << ICES1));

	// set OC1A to disabled
	TCCR1A &= ~((1 << COM1A1) | (1 << COM1A0));

	// set OC1B to disabled
	TCCR1A &= ~((1 << COM1B1) | (1 << COM1B0));

	// clear timer 1 output compare force bits for OC1A, OC1B, and OC1C
	TCCR1C &= ~((1 << FOC1A) | (1 << FOC1B) | (1 << FOC1C));

#if defined(useTimer1Interrupt)
	// disable timer 1 interrupts
	TIMSK1 &= ~((1 << ICIE1) | (1 << OCIE1C) | (1 << OCIE1B) | (1 << OCIE1A));

	// enable timer1 overflow interrupt
	TIMSK1 |= (1 << TOIE1);
#else // defined(useTimer1Interrupt)
	// disable timer 1 interrupts
	TIMSK1 &= ~((1 << ICIE1) | (1 << OCIE1C) | (1 << OCIE1B) | (1 << OCIE1A) | (1 << TOIE1));
#endif // defined(useTimer1Interrupt)

	// clear timer 1 interrupt flags
	TIFR1 |= ((1 << ICF1) | (1 << OCF1C) | (1 << OCF1B) | (1 << OCF1A) | (1 << TOV1));

#endif // defined(__AVR_ATmega2560__)
#if defined(__AVR_ATmega328P__)
	// turn on timer1 module
	PRR &= ~(1 << PRTIM1);

	// set timer 1 to 8-bit phase correct PWM mode, TOP = 0xFF
	TCCR1A &= ~(1 << WGM11);
	TCCR1A |= (1 << WGM10);
	TCCR1B &= ~((1 << WGM13) | (1 << WGM12));

	// set timer 1 prescale factor to 1
	TCCR1B &= ~((1 << CS12) | (1 << CS11));
	TCCR1B |= (1 << CS10);

	// disable timer 1 input capture noise canceler, select timer 1 falling edge for input capture
	TCCR1B &= ~((1 << ICNC1) | (1 << ICES1));

	// set OC1A to disabled
	TCCR1A &= ~((1 << COM1A1) | (1 << COM1A0));

	// set OC1B to disabled
	TCCR1A &= ~((1 << COM1B1) | (1 << COM1B0));

	// clear timer 1 output compare force bits for OC1A and OC1B
	TCCR1C &= ~((1 << FOC1A) | (1 << FOC1B));

#if defined(useTimer1Interrupt)
	// disable timer 1 interrupts
	TIMSK1 &= ~((1 << ICIE1) | (1 << OCIE1B) | (1 << OCIE1A));

	// enable timer1 overflow interrupt
	TIMSK1 |= (1 << TOIE1);
#else // defined(useTimer1Interrupt)
	// disable timer 1 interrupts
	TIMSK1 &= ~((1 << ICIE1) | (1 << OCIE1B) | (1 << OCIE1A) | (1 << TOIE1));
#endif // defined(useTimer1Interrupt)

	// clear timer 1 interrupt flags
	TIFR1 |= ((1 << ICF1) | (1 << OCF1B) | (1 << OCF1A) | (1 << TOV1));

#endif // defined(__AVR_ATmega328P__)
#endif // defined(useTimer1)
#if defined(useTimer2)
#if defined(__AVR_ATmega2560__)
	// turn on timer2 module
	PRR0 &= ~(1 << PRTIM2);

	// set timer 2 to 8-bit phase correct PWM mode, TOP = 0xFF
	TCCR2A &= ~(1 << WGM21);
	TCCR2A |= (1 << WGM20);
	TCCR2B &= ~(1 << WGM22);

	// set timer 2 prescale factor to 64
	TCCR2B &= ~((1 << CS22));
	TCCR2B |= ((1 << CS21) | (1 << CS20));

	// set OC2A to disabled
	TCCR2A &= ~((1 << COM2A1) | (1 << COM2A0));

	// set OC2B to disabled
	TCCR2A &= ~((1 << COM2B1) | (1 << COM2B0));

	// clear timer 2 output compare force bits for OC2A and OC2B
	TCCR2B &= ~((1 << FOC2A) | (1 << FOC2B));

	// disable timer 2 interrupts
	TIMSK2 &= ~((1 << OCIE2B) | (1 << OCIE2A) | (1 << TOIE2));

	// clear timer 2 interrupt flags
	TIFR2 |= ((1 << OCF2B) | (1 << OCF2A) | (1 << TOV2));

#endif // defined(__AVR_ATmega2560__)
#if defined(__AVR_ATmega328P__)
	// turn on timer2 module
	PRR &= ~(1 << PRTIM2);

	// set timer 2 to phase correct PWM mode, TOP = 0xFF
	TCCR2A &= ~(1 << WGM21);
	TCCR2A |= (1 << WGM20);
	TCCR2B &= ~(1 << WGM22);

	// set timer 2 prescale factor to 64
	TCCR2B &= ~((1 << CS22));
	TCCR2B |= ((1 << CS21) | (1 << CS20));

	// set OC2A to disabled
	TCCR2A &= ~((1 << COM2A1) | (1 << COM2A0));

	// set OC2B to disabled
	TCCR2A &= ~((1 << COM2B1) | (1 << COM2B0));

	// clear force bits for OC2A and OC2B
	TCCR2B &= ~((1 << FOC2A) | (1 << FOC2B));

	// disable timer 2 interrupts
	TIMSK2 &= ~((1 << OCIE2B) | (1 << OCIE2A) | (1 << TOIE2));

	// clear timer 2 interrupt flags
	TIFR2 |= ((1 << OCF2B) | (1 << OCF2A) | (1 << TOV2));

#endif // defined(__AVR_ATmega328P__)
#endif // defined(useTimer2)
#if defined(useTimer4)
#if defined(__AVR_ATmega32U4__)
	// turn on timer4 module
	PRR1 &= ~(1 << PRTIM4);

	// set timer 4 to phase and frequency correct mode
	TCCR4D &= ~(1 << WGM41);
	TCCR4D |= (1 << WGM40);

	// set timer 4 prescale factor to 64
	TCCR4B &= ~(1 << CS43);
	TCCR4B |= ((1 << CS42) | (1 << CS41) | (1 << CS40));

	// clear timer 4 fault protection
	TCCR4D &= ~((1 << FPIE4) | (1 << FPEN4) | (1 << FPNC4) | (1 << FPES4)  | (1 << FPAC4) | (1 << FPF4));

	// set OC4A to disabled
	TCCR4A &= ~((1 << COM4A1) | (1 << COM4A0) | (1 << PWM4A));

	// set OC4B to disabled
	TCCR4A &= ~((1 << COM4B1) | (1 << COM4B0) | (1 << PWM4B));

	// set OC4D to disabled
	TCCR4C &= ~((1 << COM4D1) | (1 << COM4D0) | (1 << PWM4D));

	// clear timer 4 PWM inversion mode
	TCCR4B &= ~(1 << PWM4X);

	// set timer 4 dead time prescaler to 1
	TCCR4B &= ~((1 << DTPS41) | (1 << DTPS40));

	// clear timer 4 output compare force bits for OC4A and OC4B
	TCCR4A &= ~((1 << FOC4A) | (1 << FOC4B));

	// clear timer 4 output compare force bits for OC4D
	TCCR4C &= ~(1 << FOC4D);

	// clear timer 4 update lock, disable timer 4 enhanced compare mode
	TCCR4E &= ~((1 << TLOCK4) | (1 << ENHC4));

	// disable timer 4 interrupts
	TIMSK4 &= ~((1 < OCIE4D) | (1 < OCIE4A) | (1 < OCIE4B) | (1 < TOIE4));

	// clear timer 4 interrupt flags
	TIFR4 |= ((1 << OCF4D) | (1 << OCF4A) | (1 << OCF4B) | (1 << TOV4));

	// set timer 4 dead time to 0
	DT4 = 0;

	// set timer 4 TOP value to 0x00FF, setting 8 bit mode
	TC4H = 0;
	OCR4C = 255;

#endif // defined(__AVR_ATmega32U4__)
#endif // defined(useTimer4)
	SREG = oldSREG; // restore interrupt flag status

#if defined(useBuffering)
	ringBuffer::init();
#endif // defined(useBuffering)
#if defined(useTWIsupport)
	TWI::init();
#if defined(useMCP23017portExpander)
	MCP23017portExpanderSupport::init(); // go init MCP23017 port expander
#endif // defined(useMCP23017portExpander)
#endif // defined(useTWIsupport)
#if defined(useSerial0Port)
	serial0::init();
#endif // defined(useSerial0Port)
#if defined(useSerial1Port)
	serial1::init();
#endif // defined(useSerial1Port)
#if defined(useSerial2Port)
	serial2::init();
#endif // defined(useSerial2Port)
#if defined(useSerial3Port)
	serial3::init();
#endif // defined(useSerial3Port)
#if defined(useHardwareSPI)
	spi::init();
#endif // defined(useHardwareSPI)
#if defined(__AVR_ATmega32U4__)
//	usbSupport::init();
#endif // defined(__AVR_ATmega32U4__)
#if defined(useBluetoothAdaFruitSPI)
	blefriend::init();
#endif // defined(useBluetoothAdaFruitSPI)
#if defined(useBluetooth)
	bluetooth::init();
#endif // defined(useBluetooth)
#if defined(useButtonInput)
	button::init();
#endif // defined(useButtonInput)
#if defined(useLCDoutput)
	LCD::init();
#endif // defined(useLCDoutput)
#if defined(useTFToutput)
	TFT::init();
#endif // defined(useTFToutput)
#if defined(useActivityLED)
	activityLED::init();
#endif // defined(useActivityLED)
#if defined(useOutputPins)
	outputPin::init();
#endif // defined(useOutputPins)

}

#ifdef useDeepSleep
static void heart::doGoDeepSleep(void)
{

#if defined(useOutputPins)
	outputPin::shutdown();
#endif // defined(useOutputPins)
#if defined(useActivityLED)
	activityLED::shutdown();
#endif // defined(useActivityLED)
	heart::changeBitFlags(timer0DelayFlags, 0xFF, 0); // cancel any timer0 delays in progress
#if defined(useTFToutput)
	TFT::shutdown(); // shut down the TFT display
#endif // defined(useTFToutput)
#if defined(useLCDoutput)
	LCD::shutdown(); // shut down the LCD display
#endif // defined(useLCDoutput)
#if defined(useButtonInput)
	button::shutdown();
#endif // defined(useButtonInput)
#if defined(useBluetooth)
	bluetooth::shutdown();
#endif // defined(useBluetooth)
#if defined(useBluetoothAdaFruitSPI)
	blefriend::shutdown();
#endif // defined(useBluetoothAdaFruitSPI)
#if defined(__AVR_ATmega32U4__)
//	usbSupport::shutdown();
#endif // defined(__AVR_ATmega32U4__)
#if defined(useHardwareSPI)
	spi::shutdown();
#endif // defined(useHardwareSPI)
#if defined(useSerial3Port)
	serial3::shutdown();
#endif // defined(useSerial3Port)
#if defined(useSerial2Port)
	serial2::shutdown();
#endif // defined(useSerial2Port)
#if defined(useSerial1Port)
	serial1::shutdown();
#endif // defined(useSerial1Port)
#if defined(useSerial0Port)
	serial0::shutdown();
#endif // defined(useSerial0Port)
#if defined(useTWIsupport)
	TWI::shutdown();
#endif // defined(useTWIsupport)

#if defined(useTimer4)
#if defined(__AVR_ATmega32U4__)
	PRR0 |= (1 << PRTIM4); // shut off timer4 module to reduce power consumption
#endif // defined(__AVR_ATmega32U4__)

#endif // defined(useTimer4)
#if defined(useTimer2)
#if defined(__AVR_ATmega2560__)
	PRR0 |= (1 << PRTIM2); // shut off timer2 module to reduce power consumption
#endif // defined(__AVR_ATmega2560__)
#if defined(__AVR_ATmega328P__)
	PRR |= (1 << PRTIM2); // shut off timer2 module to reduce power consumption
#endif // defined(__AVR_ATmega328P__)

#endif // defined(useTimer2)
#if defined(useTimer1Interrupt)
#if defined(__AVR_ATmega32U4__)
	// disable timer1 overflow interrupt
	TIMSK1 &= ~(1 << TOIE1);
#endif // defined(__AVR_ATmega32U4__)
#if defined(__AVR_ATmega2560__)
	// disable timer1 overflow interrupt
	TIMSK1 &= ~(1 << TOIE1);
#endif // defined(__AVR_ATmega2560__)
#if defined(__AVR_ATmega328P__)
	// disable timer1 overflow interrupt
	TIMSK1 &= ~(1 << TOIE1);
#endif // defined(__AVR_ATmega328P__)

#endif // defined(useTimer1Interrupt)
#if defined(useTimer1)
#if defined(__AVR_ATmega32U4__)
	PRR0 |= (1 << PRTIM1); // shut off timer1 module to reduce power consumption
#endif // defined(__AVR_ATmega32U4__)
#if defined(__AVR_ATmega2560__)
	PRR0 |= (1 << PRTIM1); // shut off timer1 module to reduce power consumption
#endif // defined(__AVR_ATmega2560__)
#if defined(__AVR_ATmega328P__)
	PRR |= (1 << PRTIM1); // shut off timer1 module to reduce power consumption
#endif // defined(__AVR_ATmega328P__)

#endif // defined(useTimer1)
	performSleepMode(SLEEP_MODE_PWR_DOWN); // go perform power-down sleep mode

	initHardware(); // restart all peripherals

}

#endif // useDeepSleep
static uint32_t heart::findCycle0Length(uint32_t lastCycle, uint32_t thisCycle) // this is only to be meant to be used with interrupt handlers
{

	if (thisCycle < lastCycle) thisCycle = 4294967295ul - lastCycle + thisCycle + 1;
	else thisCycle = thisCycle - lastCycle;

	return thisCycle;

}

static uint32_t heart::findCycle0Length(uint32_t lastCycle) // this is only to be meant to be used with the main program
{

	uint32_t thisCycle;

	thisCycle = cycles0();

	if (thisCycle < lastCycle) thisCycle = 4294967295ul - lastCycle + thisCycle + 1;
	else thisCycle = thisCycle - lastCycle;

	return thisCycle;

}

static uint32_t heart::cycles0(void)
{

	uint8_t oldSREG;
	uint32_t t;
	uint16_t a;

	oldSREG = SREG; // save state of interrupt flag
	cli(); // disable interrupts

	a = (uint16_t)(TCNT0); // do a microSeconds() - like read to determine loop length in cycles
	if (TIFR0 & (1 << TOV0)) a = (uint16_t)(TCNT0) + 256; // if overflow occurred, re-read with overflow flag taken into account

	t = timer0_overflow_count + (uint32_t)(a);

	SREG = oldSREG; // restore state of interrupt flag

	return t;

}

static void heart::wait0(uint16_t ms)
{

	uint8_t delay0Channel;

	delay0Channel = delay0(ms);
	doDelay0(delay0Channel);

}

static void heart::doDelay0(uint8_t delay0Channel)
{

	while (timer0DelayFlags & delay0Channel) idleProcess(); // wait for delay timeout

}

static uint8_t heart::delay0(uint16_t ms)
{

	uint8_t oldSREG;
	uint8_t delay0Channel;
	uint8_t i;

	while (timer0DelayFlags == 0xFF) idleProcess(); // wait for an available timer0 channel to become available

	delay0Channel = 0x01;
	i = 0;

	oldSREG = SREG; // save interrupt flag status
	cli(); // disable interrupts

	while (timer0DelayFlags & delay0Channel)
	{

		i++;
		delay0Channel <<= 1;

	}

	timer0DelayCount[(uint16_t)(i)] = ms; // request a set number of timer tick delays per millisecond

	if (ms) timer0DelayFlags |= (delay0Channel); // signal request to timer
	else timer0DelayFlags &= ~(delay0Channel);

	SREG = oldSREG; // restore interrupt flag status

	return delay0Channel;

}

static void heart::delayS(uint16_t ms)
{

	uint8_t oldSREG;

	oldSREG = SREG; // save interrupt flag status
	cli(); // disable interrupts

	timer0DelayFlags &= ~(timer0DisplayDelayFlags); // turn off all active display delays in progress
	timer0DisplayDelayFlags = 0;

	SREG = oldSREG; // restore interrupt flag status

	if (ms) heart::changeBitFlags(timer0DisplayDelayFlags, 0, delay0(ms));

}

// this function is needed since there is no way to perform an atomic bit change of an SRAM byte value
// most MPGuino variables that are shared between main program and interrupt handlers should not need to
//    be treated as atomic (!) because only one side or the other is supposed to change said variables
// however, status flag registers are obviously an exception, and status flag changes are common
//    enough to warrant an explicit function definition
static void heart::changeBitFlags(volatile uint8_t &flagRegister, uint8_t maskAND, uint8_t maskOR)
{

	uint8_t oldSREG;

	oldSREG = SREG; // save interrupt flag status
	cli(); // disable interrupts

	flagRegister = (flagRegister & ~(maskAND)) | (maskOR); // go perform atomic status flag change

	SREG = oldSREG; // restore interrupt flag status

}

static void heart::changeBitFlagBits(uint8_t bitFlagIdx, uint8_t maskAND, uint8_t maskOR)
{

	uint8_t oldSREG;

	oldSREG = SREG; // save interrupt flag status
	cli(); // disable interrupts

	bitFlags[(uint16_t)(bitFlagIdx)] = ((bitFlags[(uint16_t)(bitFlagIdx)] & ~(maskAND)) | (maskOR)); // go perform atomic status flag change

	SREG = oldSREG; // restore interrupt flag status

}

static void heart::performSleepMode(uint8_t sleepMode)
{

	set_sleep_mode(sleepMode); // set for specified sleep mode
	sleep_enable(); // enable sleep mode
	sleep_mode(); // go sleep for a bit
	sleep_disable(); // disable sleep mode

}

