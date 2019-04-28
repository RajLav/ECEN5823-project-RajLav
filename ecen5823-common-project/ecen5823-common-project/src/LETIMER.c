
#include "LETIMER.h"

const LETIMER_Init_TypeDef letimer=	//Structure
 {
 		.enable=true,
 		.debugRun=false,
 		.comp0Top=true,
 		.bufTop=false,
 		.out0Pol=false,
 		.out1Pol=false,
		.ufoa0= false,
		.ufoa1= false,
 		.repMode= letimerRepeatFree
  };

void TimerInitialize(void)
{
	  CMU_OscillatorEnable(cmuOsc_LFXO,true,true);
	  CMU_ClockSelectSet(cmuClock_LFA,cmuSelect_LFXO);
	  CMU_ClockEnable(cmuClock_HFLE, true);
	  CMU_ClockEnable(cmuClock_LETIMER0, true);
	  CMU_ClockDivSet(cmuClock_LETIMER0,4);
	  CMU_ClockEnable(cmuClock_GPIO, true);
	  LETIMER_Enable(LETIMER0,false);
	  LETIMER_Init(LETIMER0,&letimer);
	  LETIMER_CompareSet(LETIMER0,0,Comp0Val);
	  LETIMER_IntSet(LETIMER0,LETIMER_IFC_COMP0);
	  NVIC_EnableIRQ(LETIMER0_IRQn);
	  LETIMER_IntEnable(LETIMER0,LETIMER_IFC_COMP0);
	  LETIMER_Enable(LETIMER0,true);

 }

void LETIMER0_IRQHandler(void)
{
	if(LETIMER_IntGet(LETIMER0)&LETIMER_IFC_COMP0)
	{
		LETIMER_IntClear(LETIMER0,LETIMER_IF_COMP0);
		mask |= update_display;
		++roll;
		if(roll%10 == 0)
			gecko_external_signal(mask);
#if (DEVICE_IS_BLE_SERVER==SERVER)
		if(roll%3 == 0)
			mask |= LETIMER_Triggered;

#endif
//			gecko_external_signal(mask);
	}

	else
	{
		LETIMER_IntClear(LETIMER0,LETIMER_IF_COMP1);
		mask |= timer_trigg;
		gecko_external_signal(mask);
	}
}

void BlockWait(uint32_t us_wait)	//80ms delay needed
{
		uint32_t ticks=LETIMER_CounterGet(LETIMER0);
		uint32_t counter = CounterGet(us_wait);				//Calculates the delay using LFA clock(#defined in LETIMER.h)
		uint32_t i=0;
		if(ticks>counter)
			while((ticks-LETIMER_CounterGet(LETIMER0))<counter);
		else
			{
				i=Comp0Val-counter-ticks;
				while(i>0)i-=1;
			}
}

void timerEnable1HzSchedulerEvent(uint32_t Scheduler)
{
	update_display = Scheduler;
}


