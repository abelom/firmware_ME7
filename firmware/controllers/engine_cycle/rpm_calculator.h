/**
 * @file    rpm_calculator.h
 * @brief   Shaft position sensor(s) decoder header
 *
 * @date Jan 1, 2013
 * @author Andrey Belomutskiy, (c) 2012-2020
 */

#pragma once

#include "globalaccess.h"
#include "scheduler.h"

// we use this value in case of noise on trigger input lines
#define NOISY_RPM -1
#define UNREALISTIC_RPM 30000

#ifndef RPM_LOW_THRESHOLD
// no idea what is the best value, 25 is as good as any other guess
#define RPM_LOW_THRESHOLD 25
#endif

typedef enum {
	/**
	 * The engine is not spinning, RPM=0
	 */
	STOPPED,
	/**
	 * The engine is spinning up (reliable RPM is not detected yet).
	 * In this state, rpmValue is >= 0 (can be zero).
	 */
	SPINNING_UP,
	/**
	 * The engine is cranking (0 < RPM < cranking.rpm)
	 */
	CRANKING,
	/**
	 * The engine is running (RPM >= cranking.rpm)
	 */
	RUNNING,
} spinning_state_e;

class RpmCalculator {
public:
#if !EFI_PROD_CODE
	int mockRpm;
#endif /* EFI_PROD_CODE */
	RpmCalculator();
	/**
	 * Returns true if the engine is not spinning (RPM==0)
	 */
	bool isStopped(DECLARE_ENGINE_PARAMETER_SIGNATURE) const;
	/**
	 * Returns true if the engine is spinning up
	 */
	bool isSpinningUp(DECLARE_ENGINE_PARAMETER_SIGNATURE) const;
	/**
	 * Returns true if the engine is cranking OR spinning up
	 */
	bool isCranking(DECLARE_ENGINE_PARAMETER_SIGNATURE) const;
	/**
	 * Returns true if the engine is running and not cranking
	 */
	bool isRunning(DECLARE_ENGINE_PARAMETER_SIGNATURE) const;

	bool checkIfSpinning(efitick_t nowNt DECLARE_ENGINE_PARAMETER_SUFFIX) const;

	/**
	 * This accessor is used in unit-tests.
	 */
	spinning_state_e getState() const;

	/**
	 * Should be called on every trigger event when the engine is just starting to spin up.
	 */
	void setSpinningUp(efitick_t nowNt DECLARE_ENGINE_PARAMETER_SUFFIX);
	/**
	 * Called if the synchronization is lost due to a trigger timeout.
	 */
	void setStopSpinning(DECLARE_ENGINE_PARAMETER_SIGNATURE);

	/**
	 * Just a getter for rpmValue
	 * Also handles mockRpm if not EFI_PROD_CODE
	 */
	int getRpm(DECLARE_ENGINE_PARAMETER_SIGNATURE) const;
	/**
	 * This method is invoked once per engine cycle right after we calculate new RPM value
	 */
	void onNewEngineCycle();
	uint32_t getRevolutionCounterM(void) const;
	void setRpmValue(float value DECLARE_ENGINE_PARAMETER_SUFFIX);
	/**
	 * The same as setRpmValue() but without state change.
	 * We need this to be public because of calling rpmState->assignRpmValue() from rpmShaftPositionCallback()
	 */
	void assignRpmValue(float value DECLARE_ENGINE_PARAMETER_SUFFIX);
	uint32_t getRevolutionCounterSinceStart(void) const;
	/**
	 * RPM rate of change between current RPM and RPM measured during previous engine cycle
	 * see also SC_RPM_ACCEL
	 */
	float getRpmAcceleration() const;
	/**
	 * This is public because sometimes we cannot afford to call isRunning() and the value is good enough
	 * Zero if engine is not running
	 */
	volatile int rpmValue = 0;
	/**
	 * this is RPM on previous engine cycle.
	 */
	int previousRpmValue = 0;
	/**
	 * This is a performance optimization: let's pre-calculate this each time RPM changes
	 * NaN while engine is not spinning
	 */
	volatile floatus_t oneDegreeUs = NAN;
	volatile efitick_t lastRpmEventTimeNt = 0;
private:
	/**
	 * Should be called once we've realized engine is not spinning any more.
	 */
	void setStopped(DECLARE_ENGINE_PARAMETER_SIGNATURE);

	/**
	 * This counter is incremented with each revolution of one of the shafts. Could be
	 * crankshaft could be camshaft.
	 */
	volatile uint32_t revolutionCounterSinceBoot = 0;
	/**
	 * Same as the above, but since the engine started spinning
	 */
	volatile uint32_t revolutionCounterSinceStart = 0;

	spinning_state_e state = STOPPED;

	/**
	 * True if the engine is spinning (regardless of its state), i.e. if shaft position changes.
	 * Needed by spinning-up logic.
	 */
	bool isSpinning = false;
};

// Just a getter for rpmValue which also handles mockRpm if not EFI_PROD_CODE
#define GET_RPM() ( ENGINE(rpmCalculator.getRpm(PASS_ENGINE_PARAMETER_SIGNATURE)) )

// simple variable access, theoretically could be faster than getter method but that's a long stretch
#define GET_RPM_VALUE ( ENGINE(rpmCalculator.rpmValue) )

#define isValidRpm(rpm) ((rpm) > 0 && (rpm) < UNREALISTIC_RPM)

void rpmShaftPositionCallback(trigger_event_e ckpSignalType, uint32_t index DECLARE_ENGINE_PARAMETER_SUFFIX);
/**
 * @brief   Initialize RPM calculator
 */
void initRpmCalculator(Logging *sharedLogger DECLARE_ENGINE_PARAMETER_SUFFIX);

float getCrankshaftAngleNt(efitick_t timeNt DECLARE_ENGINE_PARAMETER_SUFFIX);

#define getRevolutionCounter() ENGINE(rpmCalculator.getRevolutionCounterM())

#if EFI_ENGINE_SNIFFER
#define addEngineSnifferEvent(name, msg) if (ENGINE(isEngineChartEnabled)) { waveChart.addEvent3((name), (msg)); }
 #else
#define addEngineSnifferEvent(n, msg) {}
#endif /* EFI_ENGINE_SNIFFER */

efitick_t scheduleByAngle(scheduling_s *timer, efitick_t edgeTimestamp, angle_t angle, action_s action DECLARE_ENGINE_PARAMETER_SUFFIX);

