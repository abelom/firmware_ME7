/**
 * @file    datalogging.h
 * @brief   Buffered console output stream header
 *
 * @date Feb 25, 2013
 * @author Andrey Belomutskiy, (c) 2012-2020
 */

#pragma once

#include <stdarg.h>
#include <stdint.h>

#define DELIMETER	","

// todo: migrate to external buffer so that different instances have different
// size of buffers?
class Logging {
public:
	Logging();
	Logging(const char *name, char *buffer, int bufferSize);
	void initLoggingExt(const char *name, char *buffer, int bufferSize);
	void vappendPrintf(const char *fmt, va_list arg);
	void append(const char *text);
	void appendFast(const char *text);
	void appendPrintf(const char *fmt, ...);
	const char *name = nullptr;
	char SMALL_BUFFER[40];
	/**
	 * Zero-terminated buffer of pending debug message
	 *
	 * Unless a larger external buffer is specified, this is just a pointer to DEFAULT_BUFFER
	 */
	char *buffer = nullptr;
	/**
	 * This pointer is always pointing at the position within the buffer into which next
	 * write operation would append additional data
	 */
	char *linePointer = nullptr;
	int bufferSize = 0;
	volatile bool isInitialized = false;
};

class LoggingWithStorage : public Logging {
public:
	explicit LoggingWithStorage(const char *name);
	char DEFAULT_BUFFER[200];
};

void initIntermediateLoggingBuffer(void);

int isInitialized(Logging *logging);

void initLoggingExt(Logging *logging, const char *name, char *buffer, int bufferSize);

void debugInt(Logging *logging, const char *caption, int value);

void debugFloat(Logging *logging, const char *text, float value, int precision);
void appendFloat(Logging *logging, float value, int precision);

void resetLogging(Logging *logging);

void appendMsgPrefix(Logging *logging);
void appendMsgPostfix(Logging *logging);


#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

#define lockOutputBuffer lockAnyContext
#define unlockOutputBuffer unlockAnyContext

uint32_t remainingSize(Logging *logging);

#define loggingSize(logging) ((int) (logging)->linePointer - (int) ((logging)->buffer))


void printMsg(Logging *logging, const char *fmt, ...);
void appendPrintf(Logging *logging, const char *fmt, ...);
void append(Logging *logging, const char *text);
void appendFast(Logging *logging, const char *text);

/**
 * This macro breaks the normal zero=termination constraint, please take care of this outside of this macro
 */
#define appendChar(logging, symbol) {(logging)->linePointer[0] = (symbol);(logging)->linePointer++;}

/**
 * this method copies the line into the intermediate buffer for later output by
 * the main thread
 */
void scheduleLogging(Logging *logging);

#ifdef __cplusplus
}
#endif /* __cplusplus */

void printWithLength(char *line);
