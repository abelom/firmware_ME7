/**
 * @file	tunerstudio_io.h
 *
 * @date Mar 8, 2015
 * @author Andrey Belomutskiy, (c) 2012-2020
 */

#pragma once
#include "global.h"

#if EFI_PROD_CODE
#include "usbconsole.h"
#include "pin_repository.h"
#endif

#define PROTOCOL  "001"

#define TS_RESPONSE_OK 0x00
#define TS_RESPONSE_BURN_OK 0x04
#define TS_RESPONSE_COMMAND_OK 0x07

#define TS_RESPONSE_UNDERRUN 0x80
#define TS_RESPONSE_CRC_FAILURE 0x82

typedef enum {
	TS_PLAIN = 0,
	TS_CRC = 1
} ts_response_format_e;

typedef struct {
	BaseChannel * channel;
	uint8_t writeBuffer[7];	// size(2 bytes) + response(1 byte) + crc32 (4 bytes)
	/**
	 * See 'blockingFactor' in rusefi.ini
	 */
	char crcReadBuffer[BLOCKING_FACTOR + 30];
} ts_channel_s;

// See uart_dma_s
#define TS_FIFO_BUFFER_SIZE (BLOCKING_FACTOR + 30)
// This must be a power of 2!
#define TS_DMA_BUFFER_SIZE 32

// struct needed for async DMA transfer mode (see TS_UART_DMA_MODE)
typedef struct {
	// circular DMA buffer
	uint8_t dmaBuffer[TS_DMA_BUFFER_SIZE];
	// current read position for the DMA buffer
	volatile int readPos;
	// secondary FIFO buffer for async. transfer
	uint8_t buffer[TS_FIFO_BUFFER_SIZE];
	// input FIFO Rx queue
	input_queue_t fifoRxQueue;
} uart_dma_s;

// These commands are used exclusively by the rusEfi console
#define TS_TEST_COMMAND 't' // 0x74
#define TS_GET_TEXT 'G' // 0x47
#define TS_GET_FILE_RANGE '2' // 0x32
#define TS_EXECUTE 'E' // 0x45
#define TS_GET_STRUCT '9' // 0x39

// These commands are used by TunerStudio and the rusEfi console
#define TS_HELLO_COMMAND 'Q' // 0x53 queryCommand
#define TS_OUTPUT_COMMAND 'O' // 0x4F ochGetCommand
#define TS_READ_COMMAND 'R' // 0x52
#define TS_PAGE_COMMAND 'P' // 0x50
#define TS_COMMAND_F 'F' // 0x46
#define TS_GET_FIRMWARE_VERSION 'S' // versionInfo
#define TS_GET_CONFIG_ERROR 'e' // returns getFirmwareError()

// High speed logger commands
#define TS_SET_LOGGER_MODE   'l'
#define TS_GET_LOGGER_BUFFER 'L'

// Performance tracing
#define TS_PERF_TRACE_BEGIN 'r'
#define TS_PERF_TRACE_GET_BUFFER 'b'

#define TS_SINGLE_WRITE_COMMAND 'W' // 0x57 pageValueWrite
#define TS_CHUNK_WRITE_COMMAND 'C' // 0x43 pageChunkWrite
#define TS_BURN_COMMAND 'B' // 0x42 burnCommand
#define TS_IO_TEST_COMMAND 'w' // 0x77

#define TS_CRC_CHECK_COMMAND 'k' // 0x6B

#define CRC_VALUE_SIZE 4
// todo: double-check this
#define CRC_WRAPPING_SIZE (CRC_VALUE_SIZE + 3)

#if HAL_USE_SERIAL_USB
#define CONSOLE_USB_DEVICE SDU1
#endif /* HAL_USE_SERIAL_USB */

void startTsPort(ts_channel_s *tsChannel);
bool stopTsPort(ts_channel_s *tsChannel);

// that's 1 second
#define BINARY_IO_TIMEOUT TIME_MS2I(1000)

// that's 1 second
#define SR5_READ_TIMEOUT TIME_MS2I(1000)

void sr5WriteData(ts_channel_s *tsChannel, const uint8_t * buffer, int size);
void sr5WriteCrcPacket(ts_channel_s *tsChannel, const uint8_t responseCode, const void *buf, const uint16_t size);
void sr5SendResponse(ts_channel_s *tsChannel, ts_response_format_e mode, const uint8_t * buffer, int size);
int sr5ReadData(ts_channel_s *tsChannel, uint8_t * buffer, int size);
int sr5ReadDataTimeout(ts_channel_s *tsChannel, uint8_t * buffer, int size, int timeout);
bool sr5IsReady(ts_channel_s *tsChannel);

