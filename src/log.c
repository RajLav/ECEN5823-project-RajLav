/*
 * log.c
 *
 *  Created on: Dec 18, 2018
 *      Author: Dan Walkes
 */

#include "retargetserial.h"
#include "log.h"
#include <stdbool.h>

#if INCLUDE_LOGGING

uint32_t loggerGetTimestamp(void)
{
	return 0;
}
void logInit(void)
{
	RETARGET_SerialInit();
	RETARGET_SerialCrLf(true);
	LOG_INFO("Initialized Logging");
}


void logFlush(void)
{
	RETARGET_SerialFlush();
}
#endif
