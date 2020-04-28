/**
 * @file	can_verbose.cpp
 *
 * TODO: change 'verbose' into 'broadcast'?
 *
 * @author Matthew Kennedy, (c) 2020
 */

#include "globalaccess.h"
#if EFI_CAN_SUPPORT

#include "engine.h"

#include "scaled_channel.h"
#include "can_msg_tx.h"
#include "sensor.h"
#include "can.h"
#include "allsensors.h"
#include "fuel_math.h"
#include "spark_logic.h"
#include "vehicle_speed.h"

EXTERN_ENGINE;

struct Status {
    uint16_t warningCounter;
    uint16_t lastErrorCode;

    uint8_t revLimit : 1;
    uint8_t mainRelay : 1;
    uint8_t fuelPump : 1;
    uint8_t checkEngine : 1;
    uint8_t o2Heater : 1;

    uint8_t pad6 : 1;
    uint8_t pad7 : 1;
    uint8_t pad8 : 1;

    uint8_t pad[3];
};

static void populateFrame(Status& msg) {
    msg.warningCounter = engine->engineState.warnings.warningCounter;
    msg.lastErrorCode = engine->engineState.warnings.lastErrorCode;

    msg.revLimit = GET_RPM() > CONFIG(rpmHardLimit);
    msg.mainRelay = enginePins.mainRelay.getLogicValue();
    msg.fuelPump = enginePins.fuelPumpRelay.getLogicValue();
    msg.checkEngine = enginePins.checkEnginePin.getLogicValue();
    msg.o2Heater = enginePins.o2heater.getLogicValue();
}

struct Speeds {
    uint16_t rpm;
    scaled_angle timing;
    scaled_channel<uint8_t, 2> injDuty;
    scaled_channel<uint8_t, 2> coilDuty;
    scaled_channel<uint8_t> vssKph;
    uint8_t pad[1];
};

static void populateFrame(Speeds& msg) {
    auto rpm = GET_RPM();
    msg.rpm = rpm;

    auto timing = engine->engineState.timingAdvance;
    msg.timing = timing > 360 ? timing - 720 : timing;

    msg.injDuty = getInjectorDutyCycle(rpm);
    msg.coilDuty = getCoilDutyCycle(rpm);

    msg.vssKph = getVehicleSpeed();
}

struct PedalAndTps {
    scaled_percent pedal;
    scaled_percent tps1;
    scaled_percent tps2;
    uint8_t pad[2];
};

static void populateFrame(PedalAndTps& msg)
{
    msg.pedal = Sensor::get(SensorType::AcceleratorPedal).value_or(-1);
    msg.tps1 = Sensor::get(SensorType::Tps1).value_or(-1);
    msg.tps2 = Sensor::get(SensorType::Tps2).value_or(-1);
}

struct Sensors1 {
    scaled_pressure map;
    scaled_channel<uint8_t> clt;
    scaled_channel<uint8_t> iat;
    scaled_channel<uint8_t> aux1;
    scaled_channel<uint8_t> aux2;
    scaled_channel<uint8_t> mcuTemp;
    scaled_channel<uint8_t, 2> fuelLevel;
};

static void populateFrame(Sensors1& msg) {
    msg.map = getMap();

    msg.clt = Sensor::get(SensorType::Clt).value_or(0) + PACK_ADD_TEMPERATURE;
    msg.iat = Sensor::get(SensorType::Iat).value_or(0) + PACK_ADD_TEMPERATURE;

    // todo: does aux temp even work?
    msg.aux1 = 0 + PACK_ADD_TEMPERATURE;
    msg.aux2 = 0 + PACK_ADD_TEMPERATURE;

    msg.mcuTemp = getMCUInternalTemperature();
    msg.fuelLevel = engine->sensors.fuelTankLevel;
}

struct Sensors2 {
    scaled_afr afr;
    scaled_pressure oilPressure;
    scaled_angle vvtPos;
    scaled_voltage vbatt;
};

static void populateFrame(Sensors2& msg) {
    msg.afr = getAfr();
    msg.oilPressure = Sensor::get(SensorType::OilPressure).value_or(-1);
    msg.vvtPos = engine->triggerCentral.getVVTPosition();
    msg.vbatt = getVBatt();
}

struct Fueling {
    scaled_channel<uint16_t, 1000> cylAirmass;
    scaled_channel<uint16_t, 100> estAirflow;
    scaled_ms fuel_pulse;
    scaled_percent stft;
};

static void populateFrame(Fueling& msg) {
    msg.cylAirmass = engine->engineState.sd.airMassInOneCylinder;
    msg.estAirflow = engine->engineState.airFlow;
    msg.fuel_pulse = engine->actualLastInjection;

    // todo
    msg.stft = 0;
}

void sendCanVerbose() {
    auto base = CONFIG(verboseCanBaseAddress);

    transmitStruct<Status>      (base + 0);
    transmitStruct<Speeds>      (base + 1);
    transmitStruct<PedalAndTps> (base + CAN_PEDAL_TPS_OFFSET);
    transmitStruct<Sensors1>    (base + CAN_SENSOR_1_OFFSET);
    transmitStruct<Sensors2>    (base + 4);
    transmitStruct<Fueling>     (base + 5);
}

#endif // EFI_CAN_SUPPORT
