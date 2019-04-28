/*
 * smoke.h
 *
 *  Created on: Apr 21, 2019
 *      Author: yashm
 */

#ifndef SRC_SMOKE_H_
#define SRC_SMOKE_H_

#include "adc.h"

volatile bool WarnFlag;
volatile bool OverrideFlag;
volatile uint8_t OverrideCounts;

void SmokeInit(void);
void SmokeRead(void);

#endif /* SRC_SMOKE_H_ */
