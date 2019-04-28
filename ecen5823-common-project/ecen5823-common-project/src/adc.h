/*
 * adc.h
 *
 *  Created on: Apr 21, 2019
 *      Author: yashm
 */

#ifndef SRC_ADC_H_
#define SRC_ADC_H_

#include "LETIMER.h"
#include "em_adc.h"
#include "em_chip.h"
#include "em_cmu.h"
#include "em_device.h"
#include "em_emu.h"
#include "em_core.h"

#include "em_prs.h"

#include "em_timer.h"


#include "em_device.h"
#include "em_chip.h"
#include "em_cmu.h"

#define adcFreq   16000000

volatile uint32_t sample;
volatile uint32_t millivolts;
volatile uint8_t adcFinished;

void initADC(void);
void ADCSample(void);
void WaitForFlag(void);

#endif /* SRC_ADC_H_ */
