/*
 * PWM.c
 *
 *  Created on: Apr 24, 2019
 *      Author: mentor
 */

#include <src/PWM.h>

/* Use 1 kHz for PWM frequency */
#define PWM_FREQ 1000

void PWMInit(void)
{
  CMU_ClockEnable(cmuClock_GPIO, true);
  CMU_ClockEnable(cmuClock_TIMER0, true);
  /* set CC0 location 15 pin (PC10) as output, initially high */
  GPIO_PinModeSet (gpioPortC, 10, gpioModePushPull, 1);

  /* select CC channel parameters */
  TIMER_InitCC_TypeDef timerCCInit =
  {
    .eventCtrl  = timerEventEveryEdge,
    .edge       = timerEdgeBoth,
    .prsSel     = timerPRSSELCh0,
    .cufoa      = timerOutputActionNone,
    .cofoa      = timerOutputActionNone,
    .cmoa       = timerOutputActionToggle,
    .mode       = timerCCModePWM,
    .filter     = false,
    .prsInput   = false,
    .coist      = true,
    .outInvert  = false,
  };

  /* configure CC channel 0 */
  TIMER_InitCC (TIMER0, 0, &timerCCInit);

  /* route CC0 to location 15 (PC10) and enable pin */
  TIMER0->ROUTELOC0 |= TIMER_ROUTELOC0_CC0LOC_LOC15;
  TIMER0->ROUTEPEN |= TIMER_ROUTEPEN_CC0PEN;

  /* set PWM period */
  topValue = (CMU_ClockFreqGet (cmuClock_HFPER) / (50*2))-1; // TIMER_ClkSel_TypeDef

  TIMER_TopSet (TIMER0, topValue);

  /* set timer parameters */
  TIMER_Init_TypeDef timerInit =
  {
    .enable     = false,
    .debugRun   = true,
    .prescale   = timerPrescale2,
    .clkSel     = timerClkSelHFPerClk,
    .fallAction = timerInputActionNone,
    .riseAction = timerInputActionNone,
    .mode       = timerModeUpDown,
    .dmaClrAct  = false,
    .quadModeX4 = false,
    .oneShot    = false,
    .sync       = false,
  };

  /* configure and start timer */
  TIMER_Init (TIMER0, &timerInit);
  door_open=false;
  ServoPosition(lock);//(true,false);//LockDoor();
}

void ServoPosition(bool state)
{
	if(state == unlock)
		TIMER_CompareSet (TIMER0, 0, (topValue*0.01));//3840
	else
	{
		LOG_INFO(">>>>>>>>>>>>locked");
		door_open=false;
		servo_state+=1;
		TIMER_CompareSet (TIMER0, 0, (topValue*0.05)); //19200
		displayPrintf(DISPLAY_ROW_ACTION,"Door locked");
		struct gecko_msg_flash_ps_save_rsp_t *response = gecko_cmd_flash_ps_save(0x4000,1,&servo_state);
		if(response->result)
		{
			LOG_INFO("Error:%d",response->result);
		}
	}
	TIMER_Enable(TIMER0,true);
	LOG_INFO("<<<<<<<<<<<<<<<<End");
}
