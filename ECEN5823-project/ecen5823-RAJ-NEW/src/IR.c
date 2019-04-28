/*
 * IR.c
 *
 *  Created on: Apr 21, 2019
 *      Author: mentor
 */


#include <src/IR.h>
#include "log.h"
#include "adc.h"
#include "PWM.h"

void IRStart(void)
{
	ADCSample();
	WaitForFlag();
	if(millivolts<200)
	{
		adc_convert=true;
		LETIMER_IntDisable(LETIMER0,LETIMER_IFC_COMP1);
		ServoPosition(lock);
	}

	timerWaitUs(80000);
}

void IRRead(void)
{
	CORE_DECLARE_IRQ_STATE;
	NVIC_DisableIRQ(LETIMER0_IRQn);
	ADCSample();
	WaitForFlag();
	if(millivolts<200)
	{
		adc_convert=true;
		LETIMER_IntDisable(LETIMER0,LETIMER_IFC_COMP1);
		CORE_ENTER_CRITICAL();
		ServoPosition(lock);
		CORE_EXIT_CRITICAL();
	}
	NVIC_EnableIRQ(LETIMER0_IRQn);
}
