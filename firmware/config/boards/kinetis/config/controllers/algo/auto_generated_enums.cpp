#include "global.h"
#include "rusefi_enums.h"
#include "rusefi_hw_enums.h"
// was generated automatically by rusEfi tool  from rusefi_hw_enums.h
// was generated automatically by rusEfi tool  from rusefi_enums.h
// by enum2string.jar tool
// on Sat Jul 27 16:38:51 EEST 2019
// see also gen_config_and_enums.bat



const char *getPidAutoTune_AutoTunerState(PidAutoTune_AutoTunerState value){
switch(value) {
case AUTOTUNER_OFF:
  return "AUTOTUNER_OFF";
case CONVERGED:
  return "CONVERGED";
case FAILED:
  return "FAILED";
case RELAY_STEP_DOWN:
  return "RELAY_STEP_DOWN";
case RELAY_STEP_UP:
  return "RELAY_STEP_UP";
case STEADY_STATE_AFTER_STEP_UP:
  return "STEADY_STATE_AFTER_STEP_UP";
case STEADY_STATE_AT_BASELINE:
  return "STEADY_STATE_AT_BASELINE";
  }
 return NULL;
}
const char *getPidAutoTune_Peak(PidAutoTune_Peak value){
switch(value) {
case MAXIMUM:
  return "MAXIMUM";
case MINIMUM:
  return "MINIMUM";
case NOT_A_PEAK:
  return "NOT_A_PEAK";
  }
 return NULL;
}
const char *getAdc_channel_e(adc_channel_e value){
switch(value) {
case EFI_ADC_0:
  return "EFI_ADC_0";
case EFI_ADC_1:
  return "EFI_ADC_1";
case EFI_ADC_10:
  return "EFI_ADC_10";
case EFI_ADC_11:
  return "EFI_ADC_11";
case EFI_ADC_12:
  return "EFI_ADC_12";
case EFI_ADC_13:
  return "EFI_ADC_13";
case EFI_ADC_14:
  return "EFI_ADC_14";
case EFI_ADC_15:
  return "EFI_ADC_15";
case EFI_ADC_2:
  return "EFI_ADC_2";
case EFI_ADC_3:
  return "EFI_ADC_3";
case EFI_ADC_4:
  return "EFI_ADC_4";
case EFI_ADC_5:
  return "EFI_ADC_5";
case EFI_ADC_6:
  return "EFI_ADC_6";
case EFI_ADC_7:
  return "EFI_ADC_7";
case EFI_ADC_8:
  return "EFI_ADC_8";
case EFI_ADC_9:
  return "EFI_ADC_9";
case EFI_ADC_ERROR:
  return "EFI_ADC_ERROR";
case EFI_ADC_NONE:
  return "EFI_ADC_NONE";
  }
 return NULL;
}
const char *getAdc_channel_mode_e(adc_channel_mode_e value){
switch(value) {
case ADC_FAST:
  return "ADC_FAST";
case ADC_OFF:
  return "ADC_OFF";
case ADC_SLOW:
  return "ADC_SLOW";
case Force_4_bytes_size_adc_channel_mode:
  return "Force_4_bytes_size_adc_channel_mode";
  }
 return NULL;
}
const char *getAir_pressure_sensor_type_e(air_pressure_sensor_type_e value){
switch(value) {
case Force_4_bytes_size_cranking_map_type:
  return "Force_4_bytes_size_cranking_map_type";
case MT_CUSTOM:
  return "MT_CUSTOM";
case MT_DENSO183:
  return "MT_DENSO183";
case MT_DODGE_NEON_2003:
  return "MT_DODGE_NEON_2003";
case MT_GM_3_BAR:
  return "MT_GM_3_BAR";
case MT_HONDA3BAR:
  return "MT_HONDA3BAR";
case MT_MPX4100:
  return "MT_MPX4100";
case MT_MPX4250:
  return "MT_MPX4250";
case MT_MPX4250A:
  return "MT_MPX4250A";
case MT_SUBY_DENSO:
  return "MT_SUBY_DENSO";
case MT_TOYOTA_89420_02010:
  return "MT_TOYOTA_89420_02010";
  }
 return NULL;
}
const char *getBrain_pin_e(brain_pin_e value){
switch(value) {
case GPIOA_0:
  return "GPIOA_0";
case GPIOA_1:
  return "GPIOA_1";
case GPIOA_10:
  return "GPIOA_10";
case GPIOA_11:
  return "GPIOA_11";
case GPIOA_12:
  return "GPIOA_12";
case GPIOA_13:
  return "GPIOA_13";
case GPIOA_14:
  return "GPIOA_14";
case GPIOA_15:
  return "GPIOA_15";
case GPIOA_16:
  return "GPIOA_16";
case GPIOA_17:
  return "GPIOA_17";
case GPIOA_2:
  return "GPIOA_2";
case GPIOA_3:
  return "GPIOA_3";
case GPIOA_4:
  return "GPIOA_4";
case GPIOA_5:
  return "GPIOA_5";
case GPIOA_6:
  return "GPIOA_6";
case GPIOA_7:
  return "GPIOA_7";
case GPIOA_8:
  return "GPIOA_8";
case GPIOA_9:
  return "GPIOA_9";
case GPIOB_0:
  return "GPIOB_0";
case GPIOB_1:
  return "GPIOB_1";
case GPIOB_10:
  return "GPIOB_10";
case GPIOB_11:
  return "GPIOB_11";
case GPIOB_12:
  return "GPIOB_12";
case GPIOB_13:
  return "GPIOB_13";
case GPIOB_14:
  return "GPIOB_14";
case GPIOB_15:
  return "GPIOB_15";
case GPIOB_16:
  return "GPIOB_16";
case GPIOB_17:
  return "GPIOB_17";
case GPIOB_2:
  return "GPIOB_2";
case GPIOB_3:
  return "GPIOB_3";
case GPIOB_4:
  return "GPIOB_4";
case GPIOB_5:
  return "GPIOB_5";
case GPIOB_6:
  return "GPIOB_6";
case GPIOB_7:
  return "GPIOB_7";
case GPIOB_8:
  return "GPIOB_8";
case GPIOB_9:
  return "GPIOB_9";
case GPIOC_0:
  return "GPIOC_0";
case GPIOC_1:
  return "GPIOC_1";
case GPIOC_10:
  return "GPIOC_10";
case GPIOC_11:
  return "GPIOC_11";
case GPIOC_12:
  return "GPIOC_12";
case GPIOC_13:
  return "GPIOC_13";
case GPIOC_14:
  return "GPIOC_14";
case GPIOC_15:
  return "GPIOC_15";
case GPIOC_16:
  return "GPIOC_16";
case GPIOC_17:
  return "GPIOC_17";
case GPIOC_2:
  return "GPIOC_2";
case GPIOC_3:
  return "GPIOC_3";
case GPIOC_4:
  return "GPIOC_4";
case GPIOC_5:
  return "GPIOC_5";
case GPIOC_6:
  return "GPIOC_6";
case GPIOC_7:
  return "GPIOC_7";
case GPIOC_8:
  return "GPIOC_8";
case GPIOC_9:
  return "GPIOC_9";
case GPIOD_0:
  return "GPIOD_0";
case GPIOD_1:
  return "GPIOD_1";
case GPIOD_10:
  return "GPIOD_10";
case GPIOD_11:
  return "GPIOD_11";
case GPIOD_12:
  return "GPIOD_12";
case GPIOD_13:
  return "GPIOD_13";
case GPIOD_14:
  return "GPIOD_14";
case GPIOD_15:
  return "GPIOD_15";
case GPIOD_16:
  return "GPIOD_16";
case GPIOD_17:
  return "GPIOD_17";
case GPIOD_2:
  return "GPIOD_2";
case GPIOD_3:
  return "GPIOD_3";
case GPIOD_4:
  return "GPIOD_4";
case GPIOD_5:
  return "GPIOD_5";
case GPIOD_6:
  return "GPIOD_6";
case GPIOD_7:
  return "GPIOD_7";
case GPIOD_8:
  return "GPIOD_8";
case GPIOD_9:
  return "GPIOD_9";
case GPIOE_0:
  return "GPIOE_0";
case GPIOE_1:
  return "GPIOE_1";
case GPIOE_10:
  return "GPIOE_10";
case GPIOE_11:
  return "GPIOE_11";
case GPIOE_12:
  return "GPIOE_12";
case GPIOE_13:
  return "GPIOE_13";
case GPIOE_14:
  return "GPIOE_14";
case GPIOE_15:
  return "GPIOE_15";
case GPIOE_16:
  return "GPIOE_16";
case GPIOE_17:
  return "GPIOE_17";
case GPIOE_2:
  return "GPIOE_2";
case GPIOE_3:
  return "GPIOE_3";
case GPIOE_4:
  return "GPIOE_4";
case GPIOE_5:
  return "GPIOE_5";
case GPIOE_6:
  return "GPIOE_6";
case GPIOE_7:
  return "GPIOE_7";
case GPIOE_8:
  return "GPIOE_8";
case GPIOE_9:
  return "GPIOE_9";
case GPIO_INVALID:
  return "GPIO_INVALID";
case GPIO_UNASSIGNED:
  return "GPIO_UNASSIGNED";
case TLE6240_PIN_1:
  return "TLE6240_PIN_1";
case TLE6240_PIN_10:
  return "TLE6240_PIN_10";
case TLE6240_PIN_11:
  return "TLE6240_PIN_11";
case TLE6240_PIN_12:
  return "TLE6240_PIN_12";
case TLE6240_PIN_13:
  return "TLE6240_PIN_13";
case TLE6240_PIN_14:
  return "TLE6240_PIN_14";
case TLE6240_PIN_15:
  return "TLE6240_PIN_15";
case TLE6240_PIN_16:
  return "TLE6240_PIN_16";
case TLE6240_PIN_2:
  return "TLE6240_PIN_2";
case TLE6240_PIN_3:
  return "TLE6240_PIN_3";
case TLE6240_PIN_4:
  return "TLE6240_PIN_4";
case TLE6240_PIN_5:
  return "TLE6240_PIN_5";
case TLE6240_PIN_6:
  return "TLE6240_PIN_6";
case TLE6240_PIN_7:
  return "TLE6240_PIN_7";
case TLE6240_PIN_8:
  return "TLE6240_PIN_8";
case TLE6240_PIN_9:
  return "TLE6240_PIN_9";
  }
 return NULL;
}

const char *getCan_nbc_e(can_nbc_e value){
switch(value) {
case CAN_BUS_NBC_NONE:
  return "CAN_BUS_NBC_NONE";
case CAN_BUS_MAZDA_RX8:
  return "CAN_BUS_MAZDA_RX8";
case CAN_BUS_NBC_BMW:
  return "CAN_BUS_NBC_BMW";
case CAN_BUS_NBC_FIAT:
  return "CAN_BUS_NBC_FIAT";
case CAN_BUS_NBC_VAG:
  return "CAN_BUS_NBC_VAG";
case Internal_ForceMyEnumIntSize_can_nbc:
  return "Internal_ForceMyEnumIntSize_can_nbc";
  }
 return NULL;
}
const char *getChamber_style_e(chamber_style_e value){
switch(value) {
case CS_CLOSED:
  return "CS_CLOSED";
case CS_OPEN:
  return "CS_OPEN";
case CS_SWIRL_TUMBLE:
  return "CS_SWIRL_TUMBLE";
case Internal_ForceMyEnumIntSize_chamber_stype:
  return "Internal_ForceMyEnumIntSize_chamber_stype";
  }
 return NULL;
}
const char *getCranking_ignition_mode_e(cranking_ignition_mode_e value){
switch(value) {
case CIM_DEFAULT:
  return "CIM_DEFAULT";
case CIM_FIXED_ANGLE:
  return "CIM_FIXED_ANGLE";
case Force_4_bytes_size_cranking_ignition_mode:
  return "Force_4_bytes_size_cranking_ignition_mode";
  }
 return NULL;
}
const char *getDebug_mode_e(debug_mode_e value){
switch(value) {
case DBG_2:
  return "DBG_2";
case DBG_BOOST:
  return "DBG_BOOST";
case DBG_START_STOP:
  return "DBG_START_STOP";
case DBG_LAUNCH:
  return "DBG_LAUNCH";
case DBG_39:
  return "DBG_39";
case DBG_40:
  return "DBG_40";
case DBG_ALTERNATOR_PID:
  return "DBG_ALTERNATOR_PID";
case DBG_ANALOG_INPUTS:
  return "DBG_ANALOG_INPUTS";
case DBG_ANALOG_INPUTS2:
  return "DBG_ANALOG_INPUTS2";
case DBG_AUX_PID_1:
  return "DBG_AUX_PID_1";
case DBG_34:
  return "DBG_34";
case DBG_AUX_VALVES:
  return "DBG_AUX_VALVES";
case DBG_BENCH_TEST:
  return "DBG_BENCH_TEST";
case DBG_CAN:
  return "DBG_CAN";
case DBG_CJ125:
  return "DBG_CJ125";
case DBG_CRANKING_DETAILS:
  return "DBG_CRANKING_DETAILS";
case DBG_DWELL_METRIC:
  return "DBG_DWELL_METRIC";
case DBG_ELECTRONIC_THROTTLE_EXTRA:
  return "DBG_ELECTRONIC_THROTTLE_EXTRA";
case DBG_ELECTRONIC_THROTTLE_PID:
  return "DBG_ELECTRONIC_THROTTLE_PID";
case DBG_EL_ACCEL:
  return "DBG_EL_ACCEL";
case DBG_ETB_LOGIC:
  return "DBG_ETB_LOGIC";
case DBG_EXECUTOR:
  return "DBG_EXECUTOR";
case DBG_FSIO_ADC:
  return "DBG_FSIO_ADC";
case DBG_FSIO_EXPRESSION:
  return "DBG_FSIO_EXPRESSION";
case DBG_FUEL_PID_CORRECTION:
  return "DBG_FUEL_PID_CORRECTION";
case DBG_IDLE_CONTROL:
  return "DBG_IDLE_CONTROL";
case DBG_IGNITION_TIMING:
  return "DBG_IGNITION_TIMING";
case DBG_INSTANT_RPM:
  return "DBG_INSTANT_RPM";
case DBG_ION:
  return "DBG_ION";
case DBG_KNOCK:
  return "DBG_KNOCK";
case DBG_MAP:
  return "DBG_MAP";
case DBG_METRICS:
  return "DBG_METRICS";
case DBG_SD_CARD:
  return "DBG_SD_CARD";
case DBG_SR5_PROTOCOL:
  return "DBG_SR5_PROTOCOL";
case DBG_STATUS:
  return "DBG_STATUS";
case DBG_TLE8888:
  return "DBG_TLE8888";
case DBG_TPS_ACCEL:
  return "DBG_TPS_ACCEL";
case DBG_TRIGGER_COUNTERS:
  return "DBG_TRIGGER_COUNTERS";
case DBG_TRIGGER_SYNC:
  return "DBG_TRIGGER_SYNC";
case DBG_VEHICLE_SPEED_SENSOR:
  return "DBG_VEHICLE_SPEED_SENSOR";
case DBG_VVT:
  return "DBG_VVT";
case Force_4_bytes_size_debug_mode_e:
  return "Force_4_bytes_size_debug_mode_e";
  }
 return NULL;
}
const char *getDisplay_mode_e(display_mode_e value){
switch(value) {
case DM_HD44780:
  return "DM_HD44780";
case DM_HD44780_OVER_PCF8574:
  return "DM_HD44780_OVER_PCF8574";
case DM_NONE:
  return "DM_NONE";
case Force_4_bytes_size_display_mode:
  return "Force_4_bytes_size_display_mode";
  }
 return NULL;
}
const char *getEgo_sensor_e(ego_sensor_e value){
switch(value) {
case ES_14Point7_Free:
  return "ES_14Point7_Free";
case ES_AEM:
  return "ES_AEM";
case ES_BPSX_D1:
  return "ES_BPSX_D1";
case ES_Custom:
  return "ES_Custom";
case ES_Innovate_MTX_L:
  return "ES_Innovate_MTX_L";
case ES_NarrowBand:
  return "ES_NarrowBand";
case ES_PLX:
  return "ES_PLX";
case Force_4_bytes_size_ego_sensor:
  return "Force_4_bytes_size_ego_sensor";
  }
 return NULL;
}
const char *getEngine_load_mode_e(engine_load_mode_e value){
switch(value) {
case Force_4_bytes_size_engine_load_mode:
  return "Force_4_bytes_size_engine_load_mode";
case LM_ALPHA_N:
  return "LM_ALPHA_N";
case LM_MAP:
  return "LM_MAP";
case LM_PLAIN_MAF:
  return "LM_PLAIN_MAF";
case LM_REAL_MAF:
  return "LM_REAL_MAF";
case LM_SPEED_DENSITY:
  return "LM_SPEED_DENSITY";
  }
 return NULL;
}
const char *getEngine_type_e(engine_type_e value){
switch(value) {
case ISSUE_898:
  return "ISSUE_898";
case AUDI_AAN:
  return "AUDI_AAN";
case BMW_E34:
  return "BMW_E34";
case CAMARO_4:
  return "CAMARO_4";
case CHEVY_C20_1973:
  return "CHEVY_C20_1973";
case CITROEN_TU3JP:
  return "CITROEN_TU3JP";
case DAIHATSU:
  return "DAIHATSU";
case DEFAULT_FRANKENSO:
  return "DEFAULT_FRANKENSO";
case DODGE_NEON_1995:
  return "DODGE_NEON_1995";
case DODGE_NEON_2003_CAM:
  return "DODGE_NEON_2003_CAM";
case DODGE_NEON_2003_CRANK:
  return "DODGE_NEON_2003_CRANK";
case DODGE_RAM:
  return "DODGE_RAM";
case DODGE_STRATUS:
  return "DODGE_STRATUS";
case ETB_BENCH_ENGINE:
  return "ETB_BENCH_ENGINE";
case FORD_ASPIRE_1996:
  return "FORD_ASPIRE_1996";
case FORD_ESCORT_GT:
  return "FORD_ESCORT_GT";
case FORD_FIESTA:
  return "FORD_FIESTA";
case FORD_INLINE_6_1995:
  return "FORD_INLINE_6_1995";
case FRANKENSO_QA_ENGINE:
  return "FRANKENSO_QA_ENGINE";
case Force_4_bytes_size_engine_type:
  return "Force_4_bytes_size_engine_type";
case BMW_M73_F:
  return "BMW_M73_F";
case MRE_BOARD_TEST:
  return "MRE_BOARD_TEST";
case GY6_139QMB:
  return "GY6_139QMB";
case HONDA_600:
  return "HONDA_600";
case HONDA_ACCORD_1_24_SHIFTED:
  return "HONDA_ACCORD_1_24_SHIFTED";
case HONDA_ACCORD_CD:
  return "HONDA_ACCORD_CD";
case HONDA_ACCORD_CD_DIP:
  return "HONDA_ACCORD_CD_DIP";
case HONDA_ACCORD_CD_TWO_WIRES:
  return "HONDA_ACCORD_CD_TWO_WIRES";
case LADA_KALINA:
  return "LADA_KALINA";
case MRE_MIATA_NB2_MTB:
  return "MRE_MIATA_NB2_MTB";
case MAZDA_626:
  return "MAZDA_626";
case MAZDA_MIATA_2003:
  return "MAZDA_MIATA_2003";
case MAZDA_MIATA_2003_BOARD_TEST:
  return "MAZDA_MIATA_2003_BOARD_TEST";
case MAZDA_MIATA_2003_NA_RAIL:
  return "MAZDA_MIATA_2003_NA_RAIL";
case MAZDA_MIATA_NA8:
  return "MAZDA_MIATA_NA8";
case MAZDA_MIATA_NB1:
  return "MAZDA_MIATA_NB1";
case MIATA_1990:
  return "MIATA_1990";
case MIATA_1994_DEVIATOR:
  return "MIATA_1994_DEVIATOR";
case BMW_M73_M:
  return "BMW_M73_M";
case BMW_M73_PROTEUS:
  return "BMW_M73_P";
case BMW_M73_MRE:
  return "BMW_M73_MRE";
case BMW_M73_MRE_SLAVE:
  return "BMW_M73_MRE_SLAVE";
case MIATA_1996:
  return "MIATA_1996";
case MIATA_NA6_MAP:
  return "MIATA_NA6_MAP";
case MIATA_NA6_VAF:
  return "MIATA_NA6_VAF";
case MICRO_RUS_EFI:
  return "MICRO_RUS_EFI";
case MINIMAL_PINS:
  return "MINIMAL_PINS";
case MRE_MIATA_NB2:
  return "MRE_MIATA_NB2";
case MITSU_4G93:
  return "MITSU_4G93";
case MRE_MIATA_NA6:
  return "MRE_MIATA_NA6";
case NISSAN_PRIMERA:
  return "NISSAN_PRIMERA";
case PROMETHEUS_DEFAULTS:
  return "PROMETHEUS_DEFAULTS";
case PROTEUS:
  return "PROTEUS";
case ROVER_V8:
  return "ROVER_V8";
case SACHS:
  return "SACHS";
case SUBARUEJ20G_DEFAULTS:
  return "SUBARUEJ20G_DEFAULTS";
case SUBARU_2003_WRX:
  return "SUBARU_2003_WRX";
case SUZUKI_VITARA:
  return "SUZUKI_VITARA";
case TEST_CIVIC_4_0_BOTH:
  return "TEST_CIVIC_4_0_BOTH";
case TEST_CIVIC_4_0_RISE:
  return "TEST_CIVIC_4_0_RISE";
case TEST_ENGINE:
  return "TEST_ENGINE";
case TEST_ENGINE_VVT:
  return "TEST_ENGINE_VVT";
case TEST_ISSUE_366_BOTH:
  return "TEST_ISSUE_366_BOTH";
case TEST_ISSUE_366_RISE:
  return "TEST_ISSUE_366_RISE";
case TLE8888_BENCH_ENGINE:
  return "TLE8888_BENCH_ENGINE";
case TOYOTA_2JZ_GTE_VVTi:
  return "TOYOTA_2JZ_GTE_VVTi";
case TOYOTA_JZS147:
  return "TOYOTA_JZS147";
case VAG_18_TURBO:
  return "VAG_18_TURBO";
case TEST_33816:
  return "TEST_33816";
case VW_B6:
  return "VW_B6";
case VW_ABA:
  return "VW_ABA";
case ZIL_130:
  return "ZIL_130";
  }
 return NULL;
}
const char *getGear_e(gear_e value){
switch(value) {
case GEAR_1:
  return "GEAR_1";
case GEAR_2:
  return "GEAR_2";
case GEAR_3:
  return "GEAR_3";
case GEAR_4:
  return "GEAR_4";
case NEUTRAL:
  return "NEUTRAL";
  }
 return NULL;
}
const char *getHip_state_e(hip_state_e value){
switch(value) {
case IS_INTEGRATING:
  return "IS_INTEGRATING";
case IS_SENDING_SPI_COMMAND:
  return "IS_SENDING_SPI_COMMAND";
case NOT_READY:
  return "NOT_READY";
case READY_TO_INTEGRATE:
  return "READY_TO_INTEGRATE";
case WAITING_FOR_ADC_TO_SKIP:
  return "WAITING_FOR_ADC_TO_SKIP";
case WAITING_FOR_RESULT_ADC:
  return "WAITING_FOR_RESULT_ADC";
  }
 return NULL;
}
const char *getIdle_mode_e(idle_mode_e value){
switch(value) {
case Force_4_bytes_size_idle_mode:
  return "Force_4_bytes_size_idle_mode";
case IM_AUTO:
  return "IM_AUTO";
case IM_MANUAL:
  return "IM_MANUAL";
  }
 return NULL;
}
const char *getIdle_state_e(idle_state_e value){
switch(value) {
case ADJUSTING:
  return "ADJUSTING";
case BLIP:
  return "BLIP";
case Force_4bytes_size_idle_state_e:
  return "Force_4bytes_size_idle_state_e";
case INIT:
  return "INIT";
case PID_UPPER:
  return "PID_UPPER";
case PID_VALUE:
  return "PID_VALUE";
case PWM_PRETTY_CLOSE:
  return "PWM_PRETTY_CLOSE";
case RPM_DEAD_ZONE:
  return "RPM_DEAD_ZONE";
case TPS_THRESHOLD:
  return "TPS_THRESHOLD";
  }
 return NULL;
}
const char *getIgnition_mode_e(ignition_mode_e value){
switch(value) {
case Force_4_bytes_size_ignition_mode:
  return "Force_4_bytes_size_ignition_mode";
case IM_INDIVIDUAL_COILS:
  return "IM_INDIVIDUAL_COILS";
case IM_ONE_COIL:
  return "IM_ONE_COIL";
case IM_TWO_COILS:
  return "IM_TWO_COILS";
case IM_WASTED_SPARK:
  return "IM_WASTED_SPARK";
  }
 return NULL;
}
const char *getInjection_mode_e(injection_mode_e value){
switch(value) {
case Force_4_bytes_size_injection_mode:
  return "Force_4_bytes_size_injection_mode";
case IM_BATCH:
  return "IM_BATCH";
case IM_SEQUENTIAL:
  return "IM_SEQUENTIAL";
case IM_SIMULTANEOUS:
  return "IM_SIMULTANEOUS";
case IM_SINGLE_POINT:
  return "IM_SINGLE_POINT";
  }
 return NULL;
}
const char *getLog_format_e(log_format_e value){
switch(value) {
case Force_4_bytes_size_log_format:
  return "Force_4_bytes_size_log_format";
case LF_NATIVE:
  return "LF_NATIVE";
case LM_MLV:
  return "LM_MLV";
  }
 return NULL;
}
const char *getMaf_sensor_type_e(maf_sensor_type_e value){
switch(value) {
case Bosch0280218004:
  return "Bosch0280218004";
case Bosch0280218037:
  return "Bosch0280218037";
case CUSTOM:
  return "CUSTOM";
case DensoTODO:
  return "DensoTODO";
case Internal_ForceMyEnumIntSize_maf_sensor:
  return "Internal_ForceMyEnumIntSize_maf_sensor";
  }
 return NULL;
}
const char *getMass_storage_e(mass_storage_e value){
switch(value) {
case Force_4_bytes_size_mass_storage:
  return "Force_4_bytes_size_mass_storage";
case MS_ALWAYS:
  return "MS_ALWAYS";
case MS_AUTO:
  return "MS_AUTO";
case MS_NEVER:
  return "MS_NEVER";
  }
 return NULL;
}
const char *getOperation_mode_e(operation_mode_e value){
switch(value) {
case FOUR_STROKE_CAM_SENSOR:
  return "FOUR_STROKE_CAM_SENSOR";
case FOUR_STROKE_CRANK_SENSOR:
  return "FOUR_STROKE_CRANK_SENSOR";
case FOUR_STROKE_SYMMETRICAL_CRANK_SENSOR:
  return "FOUR_STROKE_SYMMETRICAL_CRANK_SENSOR";
case Force_4_bytes_size_operation_mode_e:
  return "Force_4_bytes_size_operation_mode_e";
case OM_NONE:
  return "OM_NONE";
case TWO_STROKE:
  return "TWO_STROKE";
  }
 return NULL;
}
const char *getPin_input_mode_e(pin_input_mode_e value){
switch(value) {
case PI_DEFAULT:
  return "PI_DEFAULT";
case PI_PULLDOWN:
  return "PI_PULLDOWN";
case PI_PULLUP:
  return "PI_PULLUP";
  }
 return NULL;
}
const char *getPin_mode_e(pin_mode_e value){
switch(value) {
case PO_DEFAULT:
  return "PO_DEFAULT";
case PO_OPENDRAIN:
  return "PO_OPENDRAIN";
case PO_PULLDOWN:
  return "PO_PULLDOWN";
case PO_PULLUP:
  return "PO_PULLUP";
  }
 return NULL;
}
const char *getPin_output_mode_e(pin_output_mode_e value){
switch(value) {
case OM_DEFAULT:
  return "OM_DEFAULT";
case OM_INVERTED:
  return "OM_INVERTED";
case OM_OPENDRAIN:
  return "OM_OPENDRAIN";
case OM_OPENDRAIN_INVERTED:
  return "OM_OPENDRAIN_INVERTED";
  }
 return NULL;
}
const char *getSensor_chart_e(sensor_chart_e value){
switch(value) {
case Internal_ForceMyEnumIntSize_sensor_chart:
  return "Internal_ForceMyEnumIntSize_sensor_chart";
case SC_DETAILED_RPM:
  return "SC_DETAILED_RPM";
case SC_MAP:
  return "SC_MAP";
case SC_AUX_FAST1:
	return "SC_AUX_FAST1";
case SC_OFF:
  return "SC_OFF";
case SC_RPM_ACCEL:
  return "SC_RPM_ACCEL";
case SC_TRIGGER:
  return "SC_TRIGGER";
  }
 return NULL;
}
const char *getSpi_device_e(spi_device_e value){
switch(value) {
case SPI_DEVICE_1:
  return "SPI_DEVICE_1";
case SPI_DEVICE_2:
  return "SPI_DEVICE_2";
case SPI_DEVICE_3:
  return "SPI_DEVICE_3";
case SPI_DEVICE_4:
  return "SPI_DEVICE_4";
case SPI_NONE:
  return "SPI_NONE";
  }
 return NULL;
}
const char *getSpi_speed_e(spi_speed_e value){
switch(value) {
case _150KHz:
  return "_150KHz";
case _1_25MHz:
  return "_1_25MHz";
case _2_5MHz:
  return "_2_5MHz";
case _5MHz:
  return "_5MHz";
  }
 return NULL;
}
const char *getTChargeMode_e(tChargeMode_e value){
switch(value) {
case Force_4bytes_size_tChargeMode_e:
  return "Force_4bytes_size_tChargeMode_e";
case TCHARGE_MODE_AIR_INTERP:
  return "TCHARGE_MODE_AIR_INTERP";
case TCHARGE_MODE_RPM_TPS:
  return "TCHARGE_MODE_RPM_TPS";
  }
 return NULL;
}
const char *getTiming_mode_e(timing_mode_e value){
switch(value) {
case Internal_ForceMyEnumIntSize_timing_mode:
  return "Internal_ForceMyEnumIntSize_timing_mode";
case TM_DYNAMIC:
  return "TM_DYNAMIC";
case TM_FIXED:
  return "TM_FIXED";
  }
 return NULL;
}
const char *getTrigger_event_e(trigger_event_e value){
switch(value) {
case SHAFT_3RD_FALLING:
  return "SHAFT_3RD_FALLING";
case SHAFT_3RD_RISING:
  return "SHAFT_3RD_RISING";
case SHAFT_PRIMARY_FALLING:
  return "SHAFT_PRIMARY_FALLING";
case SHAFT_PRIMARY_RISING:
  return "SHAFT_PRIMARY_RISING";
case SHAFT_SECONDARY_FALLING:
  return "SHAFT_SECONDARY_FALLING";
case SHAFT_SECONDARY_RISING:
  return "SHAFT_SECONDARY_RISING";
  }
 return NULL;
}
const char *getTrigger_type_e(trigger_type_e value){
switch(value) {
case Force_4_bytes_size_trigger_type:
  return "Force_4_bytes_size_trigger_type";
case TT_2JZ_1_12:
  return "TT_2JZ_1_12";
case TT_2JZ_3_34:
  return "TT_2JZ_3_34";
case TT_36_2_2_2:
  return "TT_36_2_2_2";
case TT_3_1_CAM:
  return "TT_3_1_CAM";
case TT_60_2_VW:
  return "TT_60_2_VW";
case TT_DODGE_NEON_1995:
  return "TT_DODGE_NEON_1995";
case TT_DODGE_NEON_1995_ONLY_CRANK:
  return "TT_DODGE_NEON_1995_ONLY_CRANK";
case TT_DODGE_NEON_2003_CAM:
  return "TT_DODGE_NEON_2003_CAM";
case TT_DODGE_NEON_2003_CRANK:
  return "TT_DODGE_NEON_2003_CRANK";
case TT_DODGE_RAM:
  return "TT_DODGE_RAM";
case TT_DODGE_STRATUS:
  return "TT_DODGE_STRATUS";
case TT_FIAT_IAW_P8:
  return "TT_FIAT_IAW_P8";
case TT_FORD_ASPIRE:
  return "TT_FORD_ASPIRE";
case TT_GM_7X:
  return "TT_GM_7X";
case TT_GM_LS_24:
  return "TT_GM_LS_24";
case TT_HONDA_1_24:
  return "TT_HONDA_1_24";
case TT_HONDA_1_4_24:
  return "TT_HONDA_1_4_24";
case TT_HONDA_4_24:
  return "TT_HONDA_4_24";
case TT_HONDA_4_24_1:
  return "TT_HONDA_4_24_1";
case TT_HONDA_ACCORD_1_24_SHIFTED:
  return "TT_HONDA_ACCORD_1_24_SHIFTED";
case TT_HONDA_CBR_600:
  return "TT_HONDA_CBR_600";
case TT_HONDA_CBR_600_CUSTOM:
  return "TT_HONDA_CBR_600_CUSTOM";
case TT_JEEP_18_2_2_2:
  return "TT_JEEP_18_2_2_2";
case TT_JEEP_4_CYL:
  return "TT_JEEP_4_CYL";
case TT_MAZDA_DOHC_1_4:
  return "TT_MAZDA_DOHC_1_4";
case TT_MAZDA_MIATA_NA:
  return "TT_MAZDA_MIATA_NA";
case TT_MAZDA_MIATA_NB1:
  return "TT_MAZDA_MIATA_NB1";
case TT_MAZDA_MIATA_VVT_TEST:
  return "TT_MAZDA_MIATA_VVT_TEST";
case TT_MAZDA_SOHC_4:
  return "TT_MAZDA_SOHC_4";
case TT_MIATA_NB2_VVT_CAM:
  return "TT_MIATA_NB2_VVT_CAM";
case TT_MAZDA_Z5:
  return "TT_MAZDA_Z5";
case TT_MIATA_VVT:
  return "TT_MIATA_VVT";
case TT_MINI_COOPER_R50:
  return "TT_MINI_COOPER_R50";
case TT_MITSUBISHI:
  return "TT_MITSUBISHI";
case TT_NISSAN_SR20VE:
  return "TT_NISSAN_SR20VE";
case TT_NISSAN_SR20VE_360:
  return "TT_NISSAN_SR20VE_360";
case TT_ONE:
  return "TT_ONE";
case TT_ONE_PLUS_ONE:
  return "TT_ONE_PLUS_ONE";
case TT_ONE_PLUS_TOOTHED_WHEEL_60_2:
  return "TT_ONE_PLUS_TOOTHED_WHEEL_60_2";
case TT_ROVER_K:
  return "TT_ROVER_K";
case TT_SUBARU_7_6:
  return "TT_SUBARU_7_6";
case TT_TOOTHED_WHEEL:
  return "TT_TOOTHED_WHEEL";
case TT_TOOTHED_WHEEL_36_1:
  return "TT_TOOTHED_WHEEL_36_1";
case TT_TOOTHED_WHEEL_60_2:
  return "TT_TOOTHED_WHEEL_60_2";
case TT_UNUSED:
  return "TT_UNUSED";
  }
 return NULL;
}
const char *getTrigger_value_e(trigger_value_e value){
switch(value) {
case TV_FALL:
  return "TV_FALL";
case TV_RISE:
  return "TV_RISE";
  }
 return NULL;
}
const char *getTrigger_wheel_e(trigger_wheel_e value){
switch(value) {
case T_CHANNEL_3:
  return "T_CHANNEL_3";
case T_NONE:
  return "T_NONE";
case T_PRIMARY:
  return "T_PRIMARY";
case T_SECONDARY:
  return "T_SECONDARY";
  }
 return NULL;
}
const char *getUart_device_e(uart_device_e value){
switch(value) {
case UART_DEVICE_1:
  return "UART_DEVICE_1";
case UART_DEVICE_2:
  return "UART_DEVICE_2";
case UART_DEVICE_3:
  return "UART_DEVICE_3";
case UART_DEVICE_4:
  return "UART_DEVICE_4";
case UART_NONE:
  return "UART_NONE";
  }
 return NULL;
}
const char *getVvt_mode_e(vvt_mode_e value){
switch(value) {
case Force_4_bytes_size_vvt_mode:
  return "Force_4_bytes_size_vvt_mode";
case MIATA_NB2:
  return "MIATA_NB2";
case VVT_2GZ:
  return "VVT_2GZ";
case VVT_FIRST_HALF:
  return "VVT_FIRST_HALF";
case VVT_SECOND_HALF:
  return "VVT_SECOND_HALF";
  }
 return NULL;
}
