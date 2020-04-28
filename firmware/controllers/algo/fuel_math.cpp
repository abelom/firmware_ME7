/**
 * @file	fuel_math.cpp
 * @brief	Fuel amount calculation logic
 *
 *
 * @date May 27, 2013
 * @author Andrey Belomutskiy, (c) 2012-2020
 *
 * This file is part of rusEfi - see http://rusefi.com
 *
 * rusEfi is free software; you can redistribute it and/or modify it under the terms of
 * the GNU General Public License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * rusEfi is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without
 * even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with this program.
 * If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "global.h"
#include "fuel_math.h"
#include "interpolation.h"
#include "engine_configuration.h"
#include "allsensors.h"
#include "engine_math.h"
#include "rpm_calculator.h"
#include "speed_density.h"
#include "perf_trace.h"
#include "sensor.h"

EXTERN_ENGINE;

fuel_Map3D_t fuelMap("fuel");
static fuel_Map3D_t fuelPhaseMap("fl ph");
extern fuel_Map3D_t veMap;
extern afr_Map3D_t afrMap;
extern baroCorr_Map3D_t baroCorrMap;

#if EFI_ENGINE_CONTROL

DISPLAY_STATE(Engine)

DISPLAY(DISPLAY_FIELD(sparkDwell))
DISPLAY(DISPLAY_FIELD(dwellAngle))
DISPLAY(DISPLAY_FIELD(cltTimingCorrection))
DISPLAY_TEXT(eol);

DISPLAY(DISPLAY_IF(isCrankingState)) floatms_t getCrankingFuel3(float coolantTemperature,
		uint32_t revolutionCounterSinceStart DECLARE_ENGINE_PARAMETER_SUFFIX) {
	// these magic constants are in Celsius
	float baseCrankingFuel;
	if (engineConfiguration->useRunningMathForCranking) {
		baseCrankingFuel = engine->engineState.running.baseFuel;
	} else {
		baseCrankingFuel = engineConfiguration->cranking.baseFuel;
	}
	/**
	 * Cranking fuel changes over time
	 */
	DISPLAY_TEXT(Duration_coef);
	engine->engineState.DISPLAY_PREFIX(cranking).DISPLAY_FIELD(durationCoefficient) = interpolate2d("crank", revolutionCounterSinceStart, config->crankingCycleBins,
			config->crankingCycleCoef);
	DISPLAY_TEXT(eol);

	/**
	 * Cranking fuel is different depending on engine coolant temperature
	 */
	DISPLAY_TEXT(Coolant_coef);
	engine->engineState.DISPLAY_PREFIX(cranking).DISPLAY_FIELD(coolantTemperatureCoefficient) = cisnan(coolantTemperature) ? 1 : interpolate2d("crank", coolantTemperature, config->crankingFuelBins,
			config->crankingFuelCoef);
	DISPLAY_SENSOR(CLT);
	DISPLAY_TEXT(eol);

	auto tps = Sensor::get(SensorType::DriverThrottleIntent);

	DISPLAY_TEXT(TPS_coef);
	engine->engineState.DISPLAY_PREFIX(cranking).DISPLAY_FIELD(tpsCoefficient) = tps.Valid ? 1 : interpolate2d("crankTps", tps.Value, engineConfiguration->crankingTpsBins,
			engineConfiguration->crankingTpsCoef);
	DISPLAY_SENSOR(TPS);
	DISPLAY_TEXT(eol);

	floatms_t crankingFuel = baseCrankingFuel
			* engine->engineState.cranking.durationCoefficient
			* engine->engineState.cranking.coolantTemperatureCoefficient
			* engine->engineState.cranking.tpsCoefficient;

	DISPLAY_TEXT(Cranking_fuel);
	engine->engineState.DISPLAY_PREFIX(cranking).DISPLAY_FIELD(fuel) = crankingFuel;

	if (crankingFuel <= 0) {
		warning(CUSTOM_ERR_ZERO_CRANKING_FUEL, "Cranking fuel value %f", crankingFuel);
	}
	return crankingFuel;
}

/* DISPLAY_ELSE */

floatms_t getRunningFuel(floatms_t baseFuel DECLARE_ENGINE_PARAMETER_SUFFIX) {
	ScopePerf perf(PE::GetRunningFuel);

	DISPLAY_TEXT(Base_fuel);
	ENGINE(engineState.DISPLAY_PREFIX(running).DISPLAY_FIELD(baseFuel)) = baseFuel;
	DISPLAY_TEXT(eol);


	DISPLAY_TEXT(Intake_coef);
	float iatCorrection = ENGINE(engineState.DISPLAY_PREFIX(running).DISPLAY_FIELD(intakeTemperatureCoefficient));
	DISPLAY_SENSOR(IAT);
	DISPLAY_TEXT(eol);

	DISPLAY_TEXT(Coolant_coef);
	float cltCorrection = ENGINE(engineState.DISPLAY_PREFIX(running).DISPLAY_FIELD(coolantTemperatureCoefficient));
	DISPLAY_SENSOR(CLT);
	DISPLAY_TEXT(eol);

	DISPLAY_TEXT(Post_cranking_coef);
	float postCrankingFuelCorrection = ENGINE(engineState.DISPLAY_PREFIX(running).DISPLAY_FIELD(postCrankingFuelCorrection));
	DISPLAY_TEXT(eol);

	efiAssert(CUSTOM_ERR_ASSERT, !cisnan(iatCorrection), "NaN iatCorrection", 0);
	efiAssert(CUSTOM_ERR_ASSERT, !cisnan(cltCorrection), "NaN cltCorrection", 0);
	efiAssert(CUSTOM_ERR_ASSERT, !cisnan(postCrankingFuelCorrection), "NaN postCrankingFuelCorrection", 0);

	floatms_t runningFuel = baseFuel * iatCorrection * cltCorrection * postCrankingFuelCorrection + ENGINE(engineState.running.pidCorrection);
	efiAssert(CUSTOM_ERR_ASSERT, !cisnan(runningFuel), "NaN runningFuel", 0);
	DISPLAY_TEXT(eol);

	DISPLAY_TEXT(Running_fuel);
	ENGINE(engineState.DISPLAY_PREFIX(running).DISPLAY_FIELD(fuel)) = runningFuel;
	DISPLAY_TEXT(eol);

	DISPLAY_TEXT(Injector_lag);
	DISPLAY(DISPLAY_PREFIX(running).DISPLAY_FIELD(injectorLag));
	DISPLAY_SENSOR(VBATT);
	return runningFuel;
}

/* DISPLAY_ENDIF */

/**
 * Function block now works to create a standardised load from the cylinder filling as well as tune fuel via VE table. 
 * @return total duration of fuel injection per engine cycle, in milliseconds
 */
float getRealMafFuel(float airSpeed, int rpm DECLARE_ENGINE_PARAMETER_SUFFIX) {
	// If the engine is stopped, MAF is meaningless
	if (rpm == 0) {
		return 0;
	}

	// kg/hr -> g/s
	float gramPerSecond = airSpeed * 1000 / 3600;

	// 1/min -> 1/s
	float revsPerSecond = rpm / 60.0f;
	float airPerRevolution = gramPerSecond / revsPerSecond;

	// Now we have to divide among cylinders - on a 4 stroke, half of the cylinders happen every rev
	// This math is floating point to work properly on engines with odd cyl count
	float halfCylCount = CONFIG(specs.cylindersCount) / 2.0f;

	float cylinderAirmass = airPerRevolution / halfCylCount;
	
	//Calculation of 100% VE air mass in g/rev - 1 cylinder filling at 1.2929g/L
	float StandardAirCharge = CONFIG(specs.displacement) / CONFIG(specs.cylindersCount) * 1.2929; 
	//Create % load for fuel table using relative naturally aspiratedcylinder filling
	float airChargeLoad = 100 * cylinderAirmass/StandardAirCharge;
	
	//Correct air mass by VE table 
	float corrCylAirmass = cylinderAirmass * veMap.getValue(rpm, airChargeLoad) / 100;
	float fuelMassGram = corrCylAirmass / afrMap.getValue(rpm, airSpeed);
	float pulseWidthSeconds = fuelMassGram / cc_minute_to_gramm_second(engineConfiguration->injector.flow);

	// Convert to ms
	return 1000 * pulseWidthSeconds;
}

/**
 * per-cylinder fuel amount
 * todo: rename this method since it's now base+TPSaccel
 */
floatms_t getBaseFuel(int rpm DECLARE_ENGINE_PARAMETER_SUFFIX) {
	ScopePerf perf(PE::GetBaseFuel);

	floatms_t tpsAccelEnrich = ENGINE(tpsAccelEnrichment.getTpsEnrichment(PASS_ENGINE_PARAMETER_SIGNATURE));
	efiAssert(CUSTOM_ERR_ASSERT, !cisnan(tpsAccelEnrich), "NaN tpsAccelEnrich", 0);
	ENGINE(engineState.tpsAccelEnrich) = tpsAccelEnrich;

	floatms_t baseFuel;
	if (CONFIG(fuelAlgorithm) == LM_SPEED_DENSITY) {
		baseFuel = getSpeedDensityFuel(getMap(PASS_ENGINE_PARAMETER_SIGNATURE) PASS_ENGINE_PARAMETER_SUFFIX);
		efiAssert(CUSTOM_ERR_ASSERT, !cisnan(baseFuel), "NaN sd baseFuel", 0);
	} else if (engineConfiguration->fuelAlgorithm == LM_REAL_MAF) {
		float maf = getRealMaf(PASS_ENGINE_PARAMETER_SIGNATURE) + engine->engineLoadAccelEnrichment.getEngineLoadEnrichment(PASS_ENGINE_PARAMETER_SIGNATURE);
		baseFuel = getRealMafFuel(maf, rpm PASS_ENGINE_PARAMETER_SUFFIX);
		efiAssert(CUSTOM_ERR_ASSERT, !cisnan(baseFuel), "NaN rm baseFuel", 0);
	} else {
		baseFuel = engine->engineState.baseTableFuel;
		efiAssert(CUSTOM_ERR_ASSERT, !cisnan(baseFuel), "NaN bt baseFuel", 0);
	}
	engine->engineState.baseFuel = baseFuel;

	return tpsAccelEnrich + baseFuel;
}

angle_t getInjectionOffset(float rpm DECLARE_ENGINE_PARAMETER_SUFFIX) {
	if (cisnan(rpm)) {
		return 0; // error already reported
	}
	float engineLoad = getEngineLoadT(PASS_ENGINE_PARAMETER_SIGNATURE);
	if (cisnan(engineLoad)) {
		return 0; // error already reported
	}
	angle_t value = fuelPhaseMap.getValue(rpm, engineLoad);
	if (cisnan(value)) {
		// we could be here while resetting configuration for example
		warning(CUSTOM_ERR_6569, "phase map not ready");
		return 0;
	}
	angle_t result =  value + CONFIG(extraInjectionOffset);
	fixAngle(result, "inj offset#2", CUSTOM_ERR_6553);
	return result;
}

/**
 * Number of injections using each injector per engine cycle
 * @see getNumberOfSparks
 */
int getNumberOfInjections(injection_mode_e mode DECLARE_ENGINE_PARAMETER_SUFFIX) {
	switch (mode) {
	case IM_SIMULTANEOUS:
	case IM_SINGLE_POINT:
		return engineConfiguration->specs.cylindersCount;
	case IM_BATCH:
		return 2;
	case IM_SEQUENTIAL:
		return 1;
	default:
		firmwareError(CUSTOM_ERR_INVALID_INJECTION_MODE, "Unexpected injection_mode_e %d", mode);
		return 1;
	}
}

/**
 * This is more like MOSFET duty cycle since durations include injector lag
 * @see getCoilDutyCycle
 */
percent_t getInjectorDutyCycle(int rpm DECLARE_ENGINE_PARAMETER_SUFFIX) {
	floatms_t totalInjectiorAmountPerCycle = ENGINE(injectionDuration) * getNumberOfInjections(engineConfiguration->injectionMode PASS_ENGINE_PARAMETER_SUFFIX);
	floatms_t engineCycleDuration = getEngineCycleDuration(rpm PASS_ENGINE_PARAMETER_SUFFIX);
	return 100 * totalInjectiorAmountPerCycle / engineCycleDuration;
}

/**
 * @returns	Length of each individual fuel injection, in milliseconds
 *     in case of single point injection mode the amount of fuel into all cylinders, otherwise the amount for one cylinder
 */
floatms_t getInjectionDuration(int rpm DECLARE_ENGINE_PARAMETER_SUFFIX) {
	ScopePerf perf(PE::GetInjectionDuration);

#if EFI_SHAFT_POSITION_INPUT
	bool isCranking = ENGINE(rpmCalculator).isCranking(PASS_ENGINE_PARAMETER_SIGNATURE);
	injection_mode_e mode = isCranking ?
			engineConfiguration->crankingInjectionMode :
			engineConfiguration->injectionMode;
	int numberOfInjections = getNumberOfInjections(mode PASS_ENGINE_PARAMETER_SUFFIX);
	if (numberOfInjections == 0) {
		warning(CUSTOM_CONFIG_NOT_READY, "config not ready");
		return 0; // we can end up here during configuration reset
	}
	floatms_t fuelPerCycle;
	if (isCranking) {
		fuelPerCycle = getCrankingFuel(PASS_ENGINE_PARAMETER_SIGNATURE);
		efiAssert(CUSTOM_ERR_ASSERT, !cisnan(fuelPerCycle), "NaN cranking fuelPerCycle", 0);
	} else {
		floatms_t baseFuel = getBaseFuel(rpm PASS_ENGINE_PARAMETER_SUFFIX);
		fuelPerCycle = getRunningFuel(baseFuel PASS_ENGINE_PARAMETER_SUFFIX);
		efiAssert(CUSTOM_ERR_ASSERT, !cisnan(fuelPerCycle), "NaN fuelPerCycle", 0);
#if EFI_PRINTF_FUEL_DETAILS
	printf("baseFuel=%.2f fuelPerCycle=%.2f \t\n",
			baseFuel, fuelPerCycle);
#endif /*EFI_PRINTF_FUEL_DETAILS */
	}
	if (mode == IM_SINGLE_POINT) {
		// here we convert per-cylinder fuel amount into total engine amount since the single injector serves all cylinders
		fuelPerCycle *= engineConfiguration->specs.cylindersCount;
	}
	// Fuel cut-off isn't just 0 or 1, it can be tapered
	fuelPerCycle *= ENGINE(engineState.fuelCutoffCorrection);
	// If no fuel, don't add injector lag
	if (fuelPerCycle == 0.0f)
		return 0;

	floatms_t theoreticalInjectionLength = fuelPerCycle / numberOfInjections;
	floatms_t injectorLag = ENGINE(engineState.running.injectorLag);
	if (cisnan(injectorLag)) {
		warning(CUSTOM_ERR_INJECTOR_LAG, "injectorLag not ready");
		return 0; // we can end up here during configuration reset
	}
	return theoreticalInjectionLength * engineConfiguration->globalFuelCorrection + injectorLag;
#else
	return 0;
#endif
}

/**
 * @brief	Injector lag correction
 * @param	vBatt	Battery voltage.
 * @return	Time in ms for injection opening time based on current battery voltage
 */
floatms_t getInjectorLag(float vBatt DECLARE_ENGINE_PARAMETER_SUFFIX) {
	if (cisnan(vBatt)) {
		warning(OBD_System_Voltage_Malfunction, "vBatt=%.2f", vBatt);
		return 0;
	}
	
	return interpolate2d("lag", vBatt, engineConfiguration->injector.battLagCorrBins, engineConfiguration->injector.battLagCorr);
}

/**
 * @brief	Initialize fuel map data structure
 * @note this method has nothing to do with fuel map VALUES - it's job
 * is to prepare the fuel map data structure for 3d interpolation
 */
void initFuelMap(DECLARE_ENGINE_PARAMETER_SIGNATURE) {
	fuelMap.init(config->fuelTable, config->fuelLoadBins, config->fuelRpmBins);
#if (IGN_LOAD_COUNT == FUEL_LOAD_COUNT) && (IGN_RPM_COUNT == FUEL_RPM_COUNT)
	fuelPhaseMap.init(config->injectionPhase, config->injPhaseLoadBins, config->injPhaseRpmBins);
#endif /* (IGN_LOAD_COUNT == FUEL_LOAD_COUNT) && (IGN_RPM_COUNT == FUEL_RPM_COUNT) */
}

/**
 * @brief Engine warm-up fuel correction.
 */
float getCltFuelCorrection(DECLARE_ENGINE_PARAMETER_SIGNATURE) {
	const auto [valid, clt] = Sensor::get(SensorType::Clt);
	
	if (!valid)
		return 1; // this error should be already reported somewhere else, let's just handle it

	return interpolate2d("cltf", clt, config->cltFuelCorrBins, config->cltFuelCorr);
}

angle_t getCltTimingCorrection(DECLARE_ENGINE_PARAMETER_SIGNATURE) {
	const auto [valid, clt] = Sensor::get(SensorType::Clt);

	if (!valid)
		return 0; // this error should be already reported somewhere else, let's just handle it

	return interpolate2d("timc", clt, engineConfiguration->cltTimingBins, engineConfiguration->cltTimingExtra);
}

float getIatFuelCorrection(DECLARE_ENGINE_PARAMETER_SIGNATURE) {
	const auto [valid, iat] = Sensor::get(SensorType::Iat);

	if (!valid)
		return 1; // this error should be already reported somewhere else, let's just handle it

	return interpolate2d("iatc", iat, config->iatFuelCorrBins, config->iatFuelCorr);
}

float getAfterStartEnrichment(DECLARE_ENGINE_PARAMETER_SIGNATURE) {
		const auto [valid, clt] = Sensor::get(SensorType::Clt);
			if (!valid)
			return 0;

			float afterStartEnrich;
			float afterstartHoldTime;
			float afterstartDecayTime;
			float correction = 1.0f;
			float runTime = engine->engineState.running.timeSinceCrankingInSecs;


			afterStartEnrich = interpolate2d("aseEnrich", clt, config->afterstartCoolantBins, config->afterstartEnrich);
			afterstartHoldTime = interpolate2d("aseHold", clt, config->afterstartCoolantBins, config->afterstartHoldTime);
			afterstartDecayTime = interpolate2d("aseDecay", clt, config->afterstartCoolantBins, config->afterstartDecayTime);
			engine->engineState.running.timeSinceCrankingInSecs = NT2US(engine->engineState.timeSinceCranking) / 1000000.0f;



			if (ENGINE(rpmCalculator).isRunning(PASS_ENGINE_PARAMETER_SIGNATURE)) {
				if (runTime < afterstartHoldTime) {
				correction = afterStartEnrich;
				 if (afterStartEnrich < 1)
					 correction = 1.0f;
			    }
				if (runTime > afterstartHoldTime)  {
				correction = interpolateClamped(afterstartHoldTime, afterStartEnrich, (afterstartDecayTime + afterstartHoldTime), 1.0f , runTime);
				if (correction < 1)
					correction = 1.0f;
				}
				if (runTime > (afterstartHoldTime + afterstartDecayTime)) {
				correction = 1.0f;
				}
				} else {
				correction = 1.0f;
			}

return correction;
}
/**
 * @brief	Called from EngineState::periodicFastCallback to update the state.
 * @note The returned value is float, not boolean - to implement taper (smoothed correction).
 * @return	Fuel duration correction for fuel cut-off control (ex. if coasting). No correction if 1.0
 */
float getFuelCutOffCorrection(efitick_t nowNt, int rpm DECLARE_ENGINE_PARAMETER_SUFFIX) {
	// no corrections by default
	float fuelCorr = 1.0f;

	// coasting fuel cut-off correction
	if (CONFIG(coastingFuelCutEnabled)) {
		auto [tpsValid, tpsPos] = Sensor::get(SensorType::Tps1);
		if (!tpsValid) {
			return 1.0f;
		}

		const auto [cltValid, clt] = Sensor::get(SensorType::Clt);
		if (!cltValid) {
			return 1.0f;
		}

		float map = getMap(PASS_ENGINE_PARAMETER_SIGNATURE);
	
		// gather events
		bool mapDeactivate = (map >= CONFIG(coastingFuelCutMap));
		bool tpsDeactivate = (tpsPos >= CONFIG(coastingFuelCutTps));
		// If no CLT sensor (or broken), don't allow DFCO
		bool cltDeactivate = clt < (float)CONFIG(coastingFuelCutClt);
		bool rpmDeactivate = (rpm < CONFIG(coastingFuelCutRpmLow));
		bool rpmActivate = (rpm > CONFIG(coastingFuelCutRpmHigh));
		
		// state machine (coastingFuelCutStartTime is also used as a flag)
		if (!mapDeactivate && !tpsDeactivate && !cltDeactivate && rpmActivate) {
			ENGINE(engineState.coastingFuelCutStartTime) = nowNt;
		} else if (mapDeactivate || tpsDeactivate || rpmDeactivate || cltDeactivate) {
			ENGINE(engineState.coastingFuelCutStartTime) = 0;
		}
		// enable fuelcut?
		if (ENGINE(engineState.coastingFuelCutStartTime) != 0) {
			// todo: add taper - interpolate using (nowNt - coastingFuelCutStartTime)?
			fuelCorr = 0.0f;
		}
	}
	
	// todo: add other fuel cut-off checks here (possibly cutFuelOnHardLimit?)
	return fuelCorr;
}

/**
 * @return Fuel injection duration injection as specified in the fuel map, in milliseconds
 */
floatms_t getBaseTableFuel(int rpm, float engineLoad) {
#if EFI_ENGINE_CONTROL && EFI_SHAFT_POSITION_INPUT
	if (cisnan(engineLoad)) {
		warning(CUSTOM_NAN_ENGINE_LOAD_2, "NaN engine load");
		return 0;
	}
	floatms_t result = fuelMap.getValue(rpm, engineLoad);
	if (cisnan(result)) {
		// result could be NaN in case of invalid table, like during initialization
		result = 0;
		warning(CUSTOM_ERR_FUEL_TABLE_NOT_READY, "baseFuel table not ready");
	}
	return result;
#else
	return 0;
#endif
}

float getBaroCorrection(DECLARE_ENGINE_PARAMETER_SIGNATURE) {
	if (hasBaroSensor(PASS_ENGINE_PARAMETER_SIGNATURE)) {
		float correction = baroCorrMap.getValue(GET_RPM(), getBaroPressure(PASS_ENGINE_PARAMETER_SIGNATURE));
		if (cisnan(correction) || correction < 0.01) {
			warning(OBD_Barometric_Press_Circ_Range_Perf, "Invalid baro correction %f", correction);
			return 1;
		}
		return correction;
	} else {
		return 1;
	}
}

#if EFI_ENGINE_CONTROL
/**
 * @return Duration of fuel injection while craning
 */
floatms_t getCrankingFuel(DECLARE_ENGINE_PARAMETER_SIGNATURE) {
	return getCrankingFuel3(Sensor::get(SensorType::Clt).value_or(20),
			engine->rpmCalculator.getRevolutionCounterSinceStart() PASS_ENGINE_PARAMETER_SUFFIX);
}
#endif

float getFuelRate(floatms_t totalInjDuration, efitick_t timePeriod DECLARE_ENGINE_PARAMETER_SUFFIX) {
	if (timePeriod <= 0.0f)
		return 0.0f;
	float timePeriodMs = (float)NT2US(timePeriod) / 1000.0f;
	float fuelRate = totalInjDuration / timePeriodMs;
	const float cc_min_to_L_h = 60.0f / 1000.0f;
	return fuelRate * CONFIG(injector.flow) * cc_min_to_L_h;
}

#endif
