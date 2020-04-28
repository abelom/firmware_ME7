/**
 * @file init_sensorss.cpp
 */

#include "init.h"
#include "cli_registry.h"
#include "sensor.h"

static void initSensorCli(Logging* logger);

void initNewSensors(Logging* logger DECLARE_ENGINE_PARAMETER_SUFFIX) {
#if EFI_CAN_SUPPORT
	initCanSensors();
#endif

	initTps(PASS_ENGINE_PARAMETER_SIGNATURE);
	initOilPressure(PASS_ENGINE_PARAMETER_SIGNATURE);
	initNewThermistors(PASS_ENGINE_PARAMETER_SIGNATURE);

	// Init CLI functionality for sensors (mocking)
	initSensorCli(logger);
}

void reconfigureSensors(DECLARE_ENGINE_PARAMETER_SIGNATURE) {
	reconfigureTps(PASS_ENGINE_PARAMETER_SIGNATURE);
	reconfigureOilPressure(PASS_ENGINE_PARAMETER_SIGNATURE);
	reconfigureThermistors(PASS_ENGINE_PARAMETER_SIGNATURE);
}

static Logging* s_logger;

// Mocking/testing helpers
static void initSensorCli(Logging* logger) {
	s_logger = logger;

	addConsoleActionIF("set_sensor_mock", Sensor::setMockValue);
	addConsoleAction("reset_sensor_mocks", Sensor::resetAllMocks);
	addConsoleAction("show_sensors", []() { Sensor::showAllSensorInfo(s_logger); });
	addConsoleActionI("show_sensor", 
		[](int idx) {
			if (auto s = Sensor::getSensorOfType(static_cast<SensorType>(idx))) {
				s->showAllSensorInfo(s_logger);
			}
		});
}
