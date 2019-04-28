
#include "LETIMER.h"
#include "PWM.h"
#include "IR.h"

uint8_t counts=0;

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
	  LETIMER_CompareSet(LETIMER0,0,CompValue);
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
		if(door_open==false)
			TIMER_Enable(TIMER0,false);
	}

	else
	{
		LETIMER_IntClear(LETIMER0,LETIMER_IF_COMP1);
		if(counts<8 && adc_convert==false)
		{
			counts+=1;
			event_mask |= timer_trigg;
			gecko_external_signal(event_mask);
		}
		else
		{
			//PWMInit();
			adc_convert=false;
			counts=0;
			ServoPosition(lock);
			LETIMER_IntDisable(LETIMER0,LETIMER_IFC_COMP1);
		}
	}
}

//#pragma GCC push_options
//#pragma GCC optimize("O0")
//void WaitFunc(uint32_t wait_in_us)
//{
//	uint32_t counts=LETIMER_CounterGet(LETIMER0);
//	uint32_t counter = GetTicks(wait_in_us);
//	uint32_t i=0;
//	if(counts>counter)
//		while((counts-LETIMER_CounterGet(LETIMER0))<counter);
//	else
//		{
//			i=CompValue-counter-counts;
//			while(i>0)i-=1;
//		}
//}
//#pragma GCC pop_options

void timerEnable1HzSchedulerEvent(uint32_t Scheduler)
{
	DispUpdate = Scheduler;
}

/************************************************************************
 * @func :	Calculates the delay using LETIMER0 ticks as a reference.
 * @param:	Delay in micro-seconds.
 ***********************************************************************/

void timerWaitUs(uint32_t us_wait)
{
		uint32_t ticks=LETIMER_CounterGet(LETIMER0);
		uint32_t counter = GetTicks(us_wait);				//Calculates the delay using LFA clock(#defined in LETIMER.h)
		if(us_wait==0)counter=1;							//Ensures that if us_wait is 0, the delay runs for atleast 1 clock cycle
		NVIC_DisableIRQ(LETIMER0_IRQn);						//Disabling interrupts before setting value in the COMP1 register
		if(ticks>counter)
		{
			LETIMER_CompareSet(LETIMER0,1,counter);				//COMP1 set to calculated counter value

		}
		else
		{
			LETIMER_CompareSet(LETIMER0,1,(CompValue-counter-ticks));
		}
		NVIC_EnableIRQ(LETIMER0_IRQn);									//Enabling the LETIMER interrupts
		LETIMER_IntSet(LETIMER0,LETIMER_IFC_COMP0|LETIMER_IFC_COMP1);	//Setting the interrupt for COMP1
		LETIMER_IntEnable(LETIMER0,LETIMER_IFC_COMP0|LETIMER_IFC_COMP1);//Enabling interrupt for COMP1.
}


