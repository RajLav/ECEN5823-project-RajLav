/*
 * smoke.c
 *
 *  Created on: Apr 21, 2019
 *      Author: yashm
 */

#include "smoke.h"

void SmokeInit(void)
{
	WarnFlag=false;
}

void SmokeRead(void)
{
	GPIO_PinOutSet(gpioPortD,12);
	LOG_INFO("ON");
	static uint32_t i=0;
	WarnFlag=false;
	for(i=0;i<1;i+=1)
	{
		ADCSample();
		WaitForFlag();
		if(millivolts>2000)
		{
			WarnFlag=true;
			GPIO_PinOutClear(gpioPortD,12);//OFF
			return;
		}
		LOG_INFO("Millivolts:%d",millivolts);

	}
	GPIO_PinOutClear(gpioPortD,12);
}
