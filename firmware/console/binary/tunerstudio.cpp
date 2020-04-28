/**
 * @file	tunerstudio.cpp
 * @brief	Binary protocol implementation
 *
 * This implementation would not happen without the documentation
 * provided by Jon Zeeff (jon@zeeff.com)
 *
 *
 * @brief Integration with EFI Analytics Tuner Studio software
 *
 * Tuner Studio has a really simple protocol, a minimal implementation
 * capable of displaying current engine state on the gauges would
 * require only two commands: queryCommand and ochGetCommand
 *
 * queryCommand:
 * 		Communication initialization command. TunerStudio sends a single byte H
 * 		ECU response:
 * 			One of the known ECU id strings.
 *
 * ochGetCommand:
 * 		Request for output channels state.TunerStudio sends a single byte O
 * 		ECU response:
 * 			A snapshot of output channels as described in [OutputChannels] section of the .ini file
 * 			The length of this block is 'ochBlockSize' property of the .ini file
 *
 * These two commands are enough to get working gauges. In order to start configuring the ECU using
 * tuner studio, three more commands should be implemented:
 *
 *
 * @date Oct 22, 2013
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
#include "os_access.h"

#include "allsensors.h"
#include "tunerstudio.h"

#include "main_trigger_callback.h"
#include "flash_main.h"

#include "tunerstudio_io.h"
#include "tunerstudio_configuration.h"
#include "malfunction_central.h"
#include "console_io.h"
#include "crc.h"
#include "bluetooth.h"
#include "tunerstudio_io.h"
#include "tooth_logger.h"
#include "electronic_throttle.h"

#include <string.h>
#include "engine_configuration.h"
#include "bench_test.h"
#include "svnversion.h"
#include "loggingcentral.h"
#include "status_loop.h"
#include "mmc_card.h"
#include "perf_trace.h"

#if EFI_SIMULATOR
#include "rusEfiFunctionalTest.h"
#endif /* EFI_SIMULATOR */

#if EFI_TUNER_STUDIO


#ifndef EFI_IDLE_CONTROL
 #if EFI_IDLE_PID_CIC
  extern PidCic idlePid;
 #else
  extern Pid idlePid;
 #endif /* EFI_IDLE_PID_CIC */
#endif /* EFI_IDLE_CONTROL */


EXTERN_ENGINE;

extern persistent_config_container_s persistentState;

/**
 * note the use-case where text console port is switched into
 * binary port
 */
LoggingWithStorage tsLogger("binary");

#if !defined(EFI_NO_CONFIG_WORKING_COPY)
/**
 * this is a local copy of the configuration. Any changes to this copy
 * have no effect until this copy is explicitly propagated to the main working copy
 */
persistent_config_s configWorkingCopy;

#endif /* EFI_NO_CONFIG_WORKING_COPY */

static efitimems_t previousWriteReportMs = 0;

static ts_channel_s tsChannel;

// this thread wants a bit extra stack
static THD_WORKING_AREA(tunerstudioThreadStack, 3 * UTILITY_THREAD_STACK_SIZE);

static void resetTs(void) {
	memset(&tsState, 0, sizeof(tsState));
}

static void printErrorCounters(void) {
	scheduleMsg(&tsLogger, "TunerStudio size=%d / total=%d / errors=%d / H=%d / O=%d / P=%d / B=%d",
			sizeof(tsOutputChannels), tsState.totalCounter, tsState.errorCounter, tsState.queryCommandCounter,
			tsState.outputChannelsCommandCounter, tsState.readPageCommandsCounter, tsState.burnCommandCounter);
	scheduleMsg(&tsLogger, "TunerStudio W=%d / C=%d / P=%d", tsState.writeValueCommandCounter,
			tsState.writeChunkCommandCounter, tsState.pageCommandCounter);
}

void printTsStats(void) {
#if EFI_PROD_CODE
	if (false) {
		// todo: is this code needed somewhere else?
		scheduleMsg(&tsLogger, "TS RX on %s", hwPortname(engineConfiguration->binarySerialRxPin));

		scheduleMsg(&tsLogger, "TS TX on %s @%d", hwPortname(engineConfiguration->binarySerialTxPin),
				CONFIG(tunerStudioSerialSpeed));
	}
#endif /* EFI_PROD_CODE */

	printErrorCounters();

//	scheduleMsg(logger, "analogChartFrequency %d",
//			(int) (&engineConfiguration->analogChartFrequency) - (int) engineConfiguration);
//
//	int fuelMapOffset = (int) (&engineConfiguration->fuelTable) - (int) engineConfiguration;
//	scheduleMsg(logger, "fuelTable %d", fuelMapOffset);
//
//	int offset = (int) (&CONFIG(hip9011Gain)) - (int) engineConfiguration;
//	scheduleMsg(&tsLogger, "hip9011Gain %d", offset);
//
//	offset = (int) (&engineConfiguration->crankingCycleBins) - (int) engineConfiguration;
//	scheduleMsg(&tsLogger, "crankingCycleBins %d", offset);
//
//	offset = (int) (&engineConfiguration->engineCycle) - (int) engineConfiguration;
//	scheduleMsg(&tsLogger, "engineCycle %d", offset);
}

static void setTsSpeed(int value) {
	CONFIG(tunerStudioSerialSpeed) = value;
	printTsStats();
}

#if EFI_BLUETOOTH_SETUP
// Bluetooth HC-05 module initialization start (it waits for disconnect and then communicates to the module)
static void bluetoothHC05(const char *baudRate, const char *name, const char *pinCode) {
	bluetoothStart(&tsChannel, BLUETOOTH_HC_05, baudRate, name, pinCode);
}
// Bluetooth HC-06 module initialization start (it waits for disconnect and then communicates to the module)
static void bluetoothHC06(const char *baudRate, const char *name, const char *pinCode) {
	bluetoothStart(&tsChannel, BLUETOOTH_HC_06, baudRate, name, pinCode);
}
// Bluetooth SPP-C module initialization start (it waits for disconnect and then communicates to the module)
static void bluetoothSPP(const char *baudRate, const char *name, const char *pinCode) {
	bluetoothStart(&tsChannel, BLUETOOTH_SPP, baudRate, name, pinCode);
}
#endif  /* EFI_BLUETOOTH_SETUP */

void tunerStudioDebug(const char *msg) {
#if EFI_TUNER_STUDIO_VERBOSE
	scheduleMsg(&tsLogger, "%s", msg);
#endif /* EFI_TUNER_STUDIO_VERBOSE */
}

char *getWorkingPageAddr() {
#ifndef EFI_NO_CONFIG_WORKING_COPY
	return (char*) &configWorkingCopy.engineConfiguration;
#else
	return (char*) engineConfiguration;
#endif /* EFI_NO_CONFIG_WORKING_COPY */
}

static constexpr size_t getTunerStudioPageSize() {
	return TOTAL_CONFIG_SIZE;
}

static void sendOkResponse(ts_channel_s *tsChannel, ts_response_format_e mode) {
	sr5SendResponse(tsChannel, mode, NULL, 0);
}

static void sendErrorCode(ts_channel_s *tsChannel) {
	sr5WriteCrcPacket(tsChannel, TS_RESPONSE_CRC_FAILURE, NULL, 0);
}

void handlePageSelectCommand(ts_channel_s *tsChannel, ts_response_format_e mode, uint16_t pageId) {
	tsState.pageCommandCounter++;

	scheduleMsg(&tsLogger, "PAGE %d", pageId);

	if (pageId == 0) {
		sendOkResponse(tsChannel, mode);
	} else {
		sendErrorCode(tsChannel);
	}
}

/**
 * Copy specified amount of bytes from specified offset from communication layer working copy into real configuration
 *
 * Some changes like changing VE table or timing table are applied right away, meaning
 * that the values are copied from communication copy into actual engine control copy right away.
 * We call these parameters 'soft parameters'
 *
 * This is needed to support TS online auto-tune.
 *
 * On the contrary, 'hard parameters' are waiting for the Burn button to be clicked and configuration version
 * would be increased and much more complicated logic would be executed.
 */
static void onlineApplyWorkingCopyBytes(uint32_t offset, int count) {
	if (offset >= sizeof(engine_configuration_s)) {
		int maxSize = sizeof(persistent_config_s) - offset;
		if (count > maxSize) {
			warning(CUSTOM_TS_OVERFLOW, "TS overflow %d %d", offset, count);
			return;
		}
		scheduleMsg(&tsLogger, "applying soft change from %d length %d", offset, count);
#if !defined(EFI_NO_CONFIG_WORKING_COPY)
		memcpy(((char*) &persistentState.persistentConfiguration) + offset, ((char*) &configWorkingCopy) + offset,
				count);
#endif /* EFI_NO_CONFIG_WORKING_COPY */
	}
}

static const void * getStructAddr(int structId) {
	switch (structId) {
	case LDS_CLT_STATE_INDEX:
		return static_cast<thermistor_state_s*>(&engine->engineState.cltCurve);
	case LDS_IAT_STATE_INDEX:
		return static_cast<thermistor_state_s*>(&engine->engineState.iatCurve);
	case LDS_ENGINE_STATE_INDEX:
		return static_cast<engine_state2_s*>(&engine->engineState);
	case LDS_FUEL_TRIM_STATE_INDEX:
		return static_cast<wall_fuel_state*>(&engine->wallFuel[0]);
	case LDS_TRIGGER_CENTRAL_STATE_INDEX:
		return static_cast<trigger_central_s*>(&engine->triggerCentral);
	case LDS_TRIGGER_STATE_STATE_INDEX:
		return static_cast<trigger_state_s*>(&engine->triggerCentral.triggerState);
#if EFI_ELECTRONIC_THROTTLE_BODY
	case LDS_ETB_PID_STATE_INDEX:
		return static_cast<EtbController*>(engine->etbControllers[0])->getPidState();
#endif /* EFI_ELECTRONIC_THROTTLE_BODY */

#ifndef EFI_IDLE_CONTROL
	case LDS_IDLE_PID_STATE_INDEX:
		return static_cast<pid_state_s*>(&idlePid);
#endif /* EFI_IDLE_CONTROL */

	default:
		return NULL;
	}
}

/**
 * Read internal structure for Live Doc
 * This is somewhat similar to read page and somewhat similar to read outputs
 * We can later consider combining this functionality
 */
static void handleGetStructContent(ts_channel_s *tsChannel, int structId, int size) {
	tsState.readPageCommandsCounter++;

	const void *addr = getStructAddr(structId);
	if (addr == nullptr) {
		// todo: add warning code - unexpected structId
		return;
	}
	sr5SendResponse(tsChannel, TS_CRC, (const uint8_t *)addr, size);
}

// Validate whether the specified offset and count would cause an overrun in the tune.
// Returns true if an overrun would occur.
static bool validateOffsetCount(size_t offset, size_t count, ts_channel_s *tsChannel) {
	if (offset + count > getTunerStudioPageSize()) {
		scheduleMsg(&tsLogger, "TS: Project mismatch? Too much data requested %d/%d", offset, count);
		tunerStudioError("ERROR: out of range");
		sendErrorCode(tsChannel);
		return true;
	}

	return false;
}

/**
 * read log file content for rusEfi console
 */
static void handleReadFileContent(ts_channel_s *tsChannel, short fileId, uint16_t offset, uint16_t length) {
//#if EFI_FILE_LOGGING
//	readLogFileContent(tsChannel->crcReadBuffer, fileId, offset, length);
//#else
	UNUSED(tsChannel); UNUSED(fileId); UNUSED(offset); UNUSED(length);
//#endif /* EFI_FILE_LOGGING */
}

/**
 * This command is needed to make the whole transfer a bit faster
 * @note See also handleWriteValueCommand
 */
void handleWriteChunkCommand(ts_channel_s *tsChannel, ts_response_format_e mode, uint16_t offset, uint16_t count,
		void *content) {
	tsState.writeChunkCommandCounter++;

	scheduleMsg(&tsLogger, "WRITE CHUNK mode=%d o=%d s=%d", mode, offset, count);

	if (validateOffsetCount(offset, count, tsChannel)) {
		return;
	}

	uint8_t * addr = (uint8_t *) (getWorkingPageAddr() + offset);
	memcpy(addr, content, count);
	onlineApplyWorkingCopyBytes(offset, count);

	sendOkResponse(tsChannel, mode);
}

void handleCrc32Check(ts_channel_s *tsChannel, ts_response_format_e mode, uint16_t pageId, uint16_t offset,
		uint16_t count) {
	UNUSED(pageId);

	tsState.crc32CheckCommandCounter++;

	count = getTunerStudioPageSize();

	scheduleMsg(&tsLogger, "CRC32 request: offset %d size %d", offset, count);

	uint32_t crc = SWAP_UINT32(crc32((void * ) getWorkingPageAddr(), count));

	scheduleMsg(&tsLogger, "CRC32 response: %x", crc);

	sr5SendResponse(tsChannel, mode, (const uint8_t *) &crc, 4);
}

/**
 * 'Write' command receives a single value at a given offset
 * @note Writing values one by one is pretty slow
 */
void handleWriteValueCommand(ts_channel_s *tsChannel, ts_response_format_e mode, uint16_t page, uint16_t offset,
		uint8_t value) {
	UNUSED(tsChannel);
	UNUSED(mode);
	UNUSED(page);

	tsState.writeValueCommandCounter++;

	tunerStudioDebug("got W (Write)"); // we can get a lot of these

#if EFI_TUNER_STUDIO_VERBOSE
//	scheduleMsg(logger, "Page number %d\r\n", pageId); // we can get a lot of these
#endif

	if (validateOffsetCount(offset, 1, tsChannel)) {
		return;
	}

	efitimems_t nowMs = currentTimeMillis();
	if (nowMs - previousWriteReportMs > 5) {
		previousWriteReportMs = nowMs;
		scheduleMsg(&tsLogger, "offset %d: value=%d", offset, value);
	}

	getWorkingPageAddr()[offset] = value;

	onlineApplyWorkingCopyBytes(offset, 1);

//	scheduleMsg(logger, "va=%d", configWorkingCopy.boardConfiguration.idleValvePin);
}

void handlePageReadCommand(ts_channel_s *tsChannel, ts_response_format_e mode, uint16_t pageId, uint16_t offset,
		uint16_t count) {
	tsState.readPageCommandsCounter++;

#if EFI_TUNER_STUDIO_VERBOSE
	scheduleMsg(&tsLogger, "READ mode=%d offset=%d size=%d", mode, offset, count);
#endif

	if (pageId != 0) {
		// something is not right here
		tunerStudioError("ERROR: invalid page number");
		return;
	}

	if (validateOffsetCount(offset, count, tsChannel)) {
		return;
	}

	const uint8_t *addr = (const uint8_t *) (getWorkingPageAddr() + offset);
	sr5SendResponse(tsChannel, mode, addr, count);
#if EFI_TUNER_STUDIO_VERBOSE
//	scheduleMsg(&tsLogger, "Sending %d done", count);
#endif
}

void requestBurn(void) {
#if EFI_INTERNAL_FLASH
	setNeedToWriteConfiguration();
#endif
	incrementGlobalConfigurationVersion(PASS_ENGINE_PARAMETER_SIGNATURE);
}

static void sendResponseCode(ts_response_format_e mode, ts_channel_s *tsChannel, const uint8_t responseCode) {
	if (mode == TS_CRC) {
		sr5WriteCrcPacket(tsChannel, responseCode, NULL, 0);
	}
}

/**
 * 'Burn' command is a command to commit the changes
 */
void handleBurnCommand(ts_channel_s *tsChannel, ts_response_format_e mode, uint16_t page) {
	UNUSED(page);
	
	efitimems_t nowMs = currentTimeMillis();
	tsState.burnCommandCounter++;

	scheduleMsg(&tsLogger, "got B (Burn) %s", mode == TS_PLAIN ? "plain" : "CRC");

#if !defined(EFI_NO_CONFIG_WORKING_COPY)
	memcpy(&persistentState.persistentConfiguration, &configWorkingCopy, sizeof(persistent_config_s));
#endif /* EFI_NO_CONFIG_WORKING_COPY */

	requestBurn();
	sendResponseCode(mode, tsChannel, TS_RESPONSE_BURN_OK);
	scheduleMsg(&tsLogger, "BURN in %dms", currentTimeMillis() - nowMs);
}

static bool isKnownCommand(char command) {
	return command == TS_HELLO_COMMAND || command == TS_READ_COMMAND || command == TS_OUTPUT_COMMAND
			|| command == TS_PAGE_COMMAND || command == TS_BURN_COMMAND || command == TS_SINGLE_WRITE_COMMAND
			|| command == TS_CHUNK_WRITE_COMMAND || command == TS_EXECUTE
			|| command == TS_IO_TEST_COMMAND
			|| command == TS_GET_STRUCT
			|| command == TS_GET_FILE_RANGE
			|| command == TS_SET_LOGGER_MODE
			|| command == TS_GET_LOGGER_BUFFER
			|| command == TS_GET_TEXT
			|| command == TS_CRC_CHECK_COMMAND
			|| command == TS_GET_FIRMWARE_VERSION
			|| command == TS_PERF_TRACE_BEGIN
			|| command == TS_PERF_TRACE_GET_BUFFER
			|| command == TS_GET_CONFIG_ERROR;
}

// this function runs indefinitely
void runBinaryProtocolLoop(ts_channel_s *tsChannel) {
	int wasReady = false;

	while (true) {
		int isReady = sr5IsReady(tsChannel);
		if (!isReady) {
			chThdSleepMilliseconds(10);
			wasReady = false;
			continue;
		}

		if (!wasReady) {
			wasReady = true;
//			scheduleSimpleMsg(&logger, "ts channel is now ready ", hTimeNow());
		}

		tsState.totalCounter++;

		uint8_t firstByte;
		int received = sr5ReadData(tsChannel, &firstByte, 1);
#if EFI_SIMULATOR
			logMsg("received %d\r\n", received);
#endif


		if (received != 1) {
//			tunerStudioError("ERROR: no command");
#if EFI_BLUETOOTH_SETUP
			// assume there's connection loss and notify the bluetooth init code
			bluetoothSoftwareDisconnectNotify();
#endif  /* EFI_BLUETOOTH_SETUP */
			continue;
		}
		onDataArrived();

//		scheduleMsg(logger, "Got first=%x=[%c]", firstByte, firstByte);
		if (handlePlainCommand(tsChannel, firstByte))
			continue;

		uint8_t secondByte;
		received = sr5ReadData(tsChannel, &secondByte, 1);
		if (received != 1) {
			tunerStudioError("TS: ERROR: no second byte");
			continue;
		}
//		scheduleMsg(logger, "Got secondByte=%x=[%c]", secondByte, secondByte);

		uint16_t incomingPacketSize = firstByte << 8 | secondByte;

		if (incomingPacketSize == 0 || incomingPacketSize > (sizeof(tsChannel->crcReadBuffer) - CRC_WRAPPING_SIZE)) {
			scheduleMsg(&tsLogger, "TunerStudio: invalid size: %d", incomingPacketSize);
			tunerStudioError("ERROR: CRC header size");
			sendErrorCode(tsChannel);
			continue;
		}

		received = sr5ReadData(tsChannel, (uint8_t* )tsChannel->crcReadBuffer, 1);
		if (received != 1) {
			tunerStudioError("ERROR: did not receive command");
			continue;
		}

		char command = tsChannel->crcReadBuffer[0];
		if (!isKnownCommand(command)) {
			scheduleMsg(&tsLogger, "unexpected command %x", command);
			sendErrorCode(tsChannel);
			continue;
		}

#if EFI_SIMULATOR
			logMsg("command %c\r\n", command);
#endif


//		scheduleMsg(logger, "TunerStudio: reading %d+4 bytes(s)", incomingPacketSize);

		received = sr5ReadData(tsChannel, (uint8_t * ) (tsChannel->crcReadBuffer + 1),
				incomingPacketSize + CRC_VALUE_SIZE - 1);
		int expectedSize = incomingPacketSize + CRC_VALUE_SIZE - 1;
		if (received != expectedSize) {
			scheduleMsg(&tsLogger, "Got only %d bytes while expecting %d for command %c", received,
					expectedSize, command);
			tunerStudioError("ERROR: not enough bytes in stream");
			sendResponseCode(TS_CRC, tsChannel, TS_RESPONSE_UNDERRUN);
			continue;
		}

		uint32_t expectedCrc = *(uint32_t*) (tsChannel->crcReadBuffer + incomingPacketSize);

		expectedCrc = SWAP_UINT32(expectedCrc);

		uint32_t actualCrc = crc32(tsChannel->crcReadBuffer, incomingPacketSize);
		if (actualCrc != expectedCrc) {
			scheduleMsg(&tsLogger, "TunerStudio: CRC %x %x %x %x", tsChannel->crcReadBuffer[incomingPacketSize + 0],
					tsChannel->crcReadBuffer[incomingPacketSize + 1], tsChannel->crcReadBuffer[incomingPacketSize + 2],
					tsChannel->crcReadBuffer[incomingPacketSize + 3]);

			scheduleMsg(&tsLogger, "TunerStudio: command %c actual CRC %x/expected %x", tsChannel->crcReadBuffer[0],
					actualCrc, expectedCrc);
			tunerStudioError("ERROR: CRC issue");
			continue;
		}

//		scheduleMsg(logger, "TunerStudio: P00-07 %x %x %x %x %x %x %x %x", crcIoBuffer[0], crcIoBuffer[1],
//				crcIoBuffer[2], crcIoBuffer[3], crcIoBuffer[4], crcIoBuffer[5], crcIoBuffer[6], crcIoBuffer[7]);

		int success = tunerStudioHandleCrcCommand(tsChannel, tsChannel->crcReadBuffer, incomingPacketSize);
		if (!success)
			print("got unexpected TunerStudio command %x:%c\r\n", command, command);

	}
}

static THD_FUNCTION(tsThreadEntryPoint, arg) {
	(void) arg;
	chRegSetThreadName("tunerstudio thread");

	startTsPort(&tsChannel);

	runBinaryProtocolLoop(&tsChannel);
}

/**
 * Copy real configuration into the communications layer working copy
 */
void syncTunerStudioCopy(void) {
#if !defined(EFI_NO_CONFIG_WORKING_COPY)
	memcpy(&configWorkingCopy, &persistentState.persistentConfiguration, sizeof(persistent_config_s));
#endif /* EFI_NO_CONFIG_WORKING_COPY */
}

tunerstudio_counters_s tsState;
TunerStudioOutputChannels tsOutputChannels;

void tunerStudioError(const char *msg) {
	tunerStudioDebug(msg);
	printErrorCounters();
	tsState.errorCounter++;
}

/**
 * Query with CRC takes place while re-establishing connection
 * Query without CRC takes place on TunerStudio startup
 */
void handleQueryCommand(ts_channel_s *tsChannel, ts_response_format_e mode) {
	tsState.queryCommandCounter++;
#if EFI_TUNER_STUDIO_VERBOSE
	scheduleMsg(&tsLogger, "got S/H (queryCommand) mode=%d", mode);
	printTsStats();
#endif
	sr5SendResponse(tsChannel, mode, (const uint8_t *) TS_SIGNATURE, strlen(TS_SIGNATURE) + 1);
}

/**
 * @brief 'Output' command sends out a snapshot of current values
 */
void handleOutputChannelsCommand(ts_channel_s *tsChannel, ts_response_format_e mode, uint16_t offset, uint16_t count) {
	if (offset + count > sizeof(TunerStudioOutputChannels)) {
		scheduleMsg(&tsLogger, "TS: Version Mismatch? Too much data requested %d+%d", offset, count);
		sendErrorCode(tsChannel);
		return;
	}

	tsState.outputChannelsCommandCounter++;
	prepareTunerStudioOutputs();
	// this method is invoked too often to print any debug information
	sr5SendResponse(tsChannel, mode, ((const uint8_t *) &tsOutputChannels) + offset, count);
}

void handleTestCommand(ts_channel_s *tsChannel) {
	tsState.testCommandCounter++;
	static char testOutputBuffer[24];
	/**
	 * this is NOT a standard TunerStudio command, this is my own
	 * extension of the protocol to simplify troubleshooting
	 */
	tunerStudioDebug("got T (Test)");
	sr5WriteData(tsChannel, (const uint8_t *) VCS_VERSION, sizeof(VCS_VERSION));

	chsnprintf(testOutputBuffer, sizeof(testOutputBuffer), " %d %d", engine->engineState.warnings.lastErrorCode, tsState.testCommandCounter);
	sr5WriteData(tsChannel, (const uint8_t *) testOutputBuffer, strlen(testOutputBuffer));

	chsnprintf(testOutputBuffer, sizeof(testOutputBuffer), " uptime=%ds", getTimeNowSeconds());
	sr5WriteData(tsChannel, (const uint8_t *) testOutputBuffer, strlen(testOutputBuffer));

	chsnprintf(testOutputBuffer, sizeof(testOutputBuffer), " %s\r\n", PROTOCOL_TEST_RESPONSE_TAG);
	sr5WriteData(tsChannel, (const uint8_t *) testOutputBuffer, strlen(testOutputBuffer));
}

extern CommandHandler console_line_callback;

static void handleGetVersion(ts_channel_s *tsChannel, ts_response_format_e mode) {
	static char versionBuffer[32];
	chsnprintf(versionBuffer, sizeof(versionBuffer), "Black Magic@1.01", getRusEfiVersion(), VCS_VERSION);
	sr5SendResponse(tsChannel, mode, (const uint8_t *) versionBuffer, strlen(versionBuffer) + 1);
}

static void handleGetText(ts_channel_s *tsChannel) {
	tsState.textCommandCounter++;

	printOverallStatus(getTimeNowSeconds());

	int outputSize;
	char *output = swapOutputBuffers(&outputSize);
#if EFI_SIMULATOR
			logMsg("get test sending [%d]\r\n", outputSize);
#endif

	sr5WriteCrcPacket(tsChannel, TS_RESPONSE_COMMAND_OK, output, outputSize);
#if EFI_SIMULATOR
			logMsg("sent [%d]\r\n", outputSize);
#endif
}

static void handleExecuteCommand(ts_channel_s *tsChannel, char *data, int incomingPacketSize) {
	sr5WriteCrcPacket(tsChannel, TS_RESPONSE_COMMAND_OK, NULL, 0);
	data[incomingPacketSize] = 0;
	char *trimmed = efiTrim(data);
#if EFI_SIMULATOR
			logMsg("execute [%s]\r\n", trimmed);
#endif
	(console_line_callback)(trimmed);
}

/**
 * @return true if legacy command was processed, false otherwise
 */
bool handlePlainCommand(ts_channel_s *tsChannel, uint8_t command) {
	// Bail fast if guaranteed not to be a plain command
	if (command == 0)
	{
		return false;
	}
	else if (command == TS_HELLO_COMMAND) {
		scheduleMsg(&tsLogger, "Got naked Query command");
		handleQueryCommand(tsChannel, TS_PLAIN);
		return true;
	} else if (command == TS_TEST_COMMAND || command == 'T') {
		handleTestCommand(tsChannel);
		return true;
	} else if (command == TS_COMMAND_F) {
		/**
		 * http://www.msextra.com/forums/viewtopic.php?f=122&t=48327
		 * Response from TS support: This is an optional command		 *
		 * "The F command is used to find what ini. file needs to be loaded in TunerStudio to match the controller.
		 * If you are able to just make your firmware ignore the command that would work.
		 * Currently on some firmware versions the F command is not used and is just ignored by the firmware as a unknown command."
		 */

		tunerStudioDebug("not ignoring F");
		sr5WriteData(tsChannel, (const uint8_t *) PROTOCOL, strlen(PROTOCOL));
		return true;
	} else {
		// This wasn't a valid command
		return false;
	}
}


int tunerStudioHandleCrcCommand(ts_channel_s *tsChannel, char *data, int incomingPacketSize) {
	ScopePerf perf(PE::TunerStudioHandleCrcCommand);

	char command = data[0];
	data++;

	const uint16_t* data16 = reinterpret_cast<uint16_t*>(data);

	switch(command)
	{
	case TS_OUTPUT_COMMAND:
		handleOutputChannelsCommand(tsChannel, TS_CRC, data16[0], data16[1]);
		break;
	case TS_HELLO_COMMAND:
		tunerStudioDebug("got Query command");
		handleQueryCommand(tsChannel, TS_CRC);
		break;
	case TS_GET_FIRMWARE_VERSION:
		handleGetVersion(tsChannel, TS_CRC);
		break;
	case TS_GET_TEXT:
		handleGetText(tsChannel);
		break;
	case TS_EXECUTE:
		handleExecuteCommand(tsChannel, data, incomingPacketSize - 1);
		break;
	case TS_PAGE_COMMAND:
		handlePageSelectCommand(tsChannel, TS_CRC, data16[0]);
		break;
	case TS_GET_STRUCT:
		handleGetStructContent(tsChannel, data16[0], data16[1]);
		break;
	case TS_GET_FILE_RANGE:
		handleReadFileContent(tsChannel, data16[0], data16[1], data16[2]);
		break;
	case TS_CHUNK_WRITE_COMMAND:
		handleWriteChunkCommand(tsChannel, TS_CRC, data16[1], data16[2], data + sizeof(TunerStudioWriteChunkRequest));
		break;
	case TS_SINGLE_WRITE_COMMAND:
		{
			uint8_t value = data[4];
			handleWriteValueCommand(tsChannel, TS_CRC, data16[0], data16[1], value);
		}
		break;
	case TS_CRC_CHECK_COMMAND:
		handleCrc32Check(tsChannel, TS_CRC, data16[0], data16[1], data16[2]);
		break;
	case TS_BURN_COMMAND:
		handleBurnCommand(tsChannel, TS_CRC, data16[0]);
		break;
	case TS_READ_COMMAND:
		handlePageReadCommand(tsChannel, TS_CRC, data16[0], data16[1], data16[2]);
		break;
	case TS_TEST_COMMAND:
		[[fallthrough]];
	case 'T':
		handleTestCommand(tsChannel);
		break;
	case TS_IO_TEST_COMMAND:
		{
			uint16_t subsystem = SWAP_UINT16(data16[0]);
			uint16_t index = SWAP_UINT16(data16[1]);

			if (engineConfiguration->debugMode == DBG_BENCH_TEST) {
				tsOutputChannels.debugIntField1++;
				tsOutputChannels.debugIntField2 = subsystem;
				tsOutputChannels.debugIntField3 = index;
			}

#if EFI_PROD_CODE && EFI_ENGINE_CONTROL
		executeTSCommand(subsystem, index);
#endif /* EFI_PROD_CODE */
			sendOkResponse(tsChannel, TS_CRC);
		}
		break;
#if EFI_TOOTH_LOGGER
	case TS_SET_LOGGER_MODE:
		switch(data[0]) {
		case 0x01:
			EnableToothLogger();
			break;
		case 0x02:
			DisableToothLogger();
			break;
		default:
			// dunno what that was, send NAK
			return false;
		}

		sendOkResponse(tsChannel, TS_CRC);

		break;
	case TS_GET_LOGGER_BUFFER:
		{
			auto toothBuffer = GetToothLoggerBuffer();
			sr5SendResponse(tsChannel, TS_CRC, toothBuffer.Buffer, toothBuffer.Length);
		}

		break;
#endif /* EFI_TOOTH_LOGGER */
#if ENABLE_PERF_TRACE
	case TS_PERF_TRACE_BEGIN:
		perfTraceEnable();
		sendOkResponse(tsChannel, TS_CRC);
		break;
	case TS_PERF_TRACE_GET_BUFFER:
		{
			auto trace = perfTraceGetBuffer();
			sr5SendResponse(tsChannel, TS_CRC, trace.Buffer, trace.Size);
		}

		break;
#endif /* ENABLE_PERF_TRACE */
	case TS_GET_CONFIG_ERROR:
		sr5SendResponse(tsChannel, TS_CRC, reinterpret_cast<const uint8_t*>(getFirmwareError()), strlen(getFirmwareError()));
		break;
	default:
		tunerStudioError("ERROR: ignoring unexpected command");
		return false;
	}

	return true;
}

void startTunerStudioConnectivity(void) {
	// Assert tune & output channel struct sizes
	static_assert(sizeof(persistent_config_s) == TOTAL_CONFIG_SIZE, "TS datapage size mismatch");
	static_assert(sizeof(TunerStudioOutputChannels) == TS_OUTPUT_SIZE, "TS output channels size mismatch");

	memset(&tsState, 0, sizeof(tsState));
	syncTunerStudioCopy();

	addConsoleAction("tsinfo", printTsStats);
	addConsoleAction("reset_ts", resetTs);
	addConsoleActionI("set_ts_speed", setTsSpeed);
	
#if EFI_BLUETOOTH_SETUP
	// Usage:   "bluetooth_hc06 <baud> <name> <pincode>"
	// Example: "bluetooth_hc06 38400 rusefi 1234"
	addConsoleActionSSS("bluetooth_hc05", bluetoothHC05);
	addConsoleActionSSS("bluetooth_hc06", bluetoothHC06);
	addConsoleActionSSS("bluetooth_spp", bluetoothSPP);
	addConsoleAction("bluetooth_cancel", bluetoothCancel);
#endif /* EFI_BLUETOOTH_SETUP */

	chThdCreateStatic(tunerstudioThreadStack, sizeof(tunerstudioThreadStack), NORMALPRIO, (tfunc_t)tsThreadEntryPoint, NULL);
}

#endif
