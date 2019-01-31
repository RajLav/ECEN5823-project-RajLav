/*Name :- Raj Lavingia
  Credits : Dan Walkes
  Date :- 1/30/19 
  reference : https://www.silabs.com/community/wireless/bluetooth/knowledge-base.entry.html/2017/08/17/using_low_energytim-9RYB
  https://www.silabs.com/community/mcu/32-bit/forum.topic.html/bgm111_letimer_outpu-WMEF
  */

/* Board Headers */

#include "init_mcu.h"
#include "init_board.h"
#include "init_app.h"
#include "ble-configuration.h"
#include "board_features.h"

/* Bluetooth stack header*/
#include "bg_types.h"
#include "native_gecko.h"
#include "gatt_db.h"

/*Libraries containing default gecko configuration value*/
#include "em_emu.h"
#include "em_cmu.h"

/*Device Initialization error*/
#include "hal-config.h"

#if defined(HAL_CONFIG)
#include "bsphalconfig.h"
#else
#include "bspconfig.h"
#endif

#include "em_device.h"
#include "em_chip.h"
#include "gpio.h"

#include "sleep.h"
#include "em_letimer.h"

#include "em_assert.h"
#include "em_core.h"
#include "em_rmu.h"

//LETIMER header file RM
//#include "LETIMER.h"
//Max connections
#ifndef MAX_CONNECTIONS
#define MAX_CONNECTIONS 4
#endif


uint8_t bluetooth_stack_heap[DEFAULT_BLUETOOTH_HEAP(MAX_CONNECTIONS)];

// Gecko configuration parameters (see gecko_configuration.h)
static const gecko_configuration_t config = {
  .config_flags = 0,
  .sleep.flags = SLEEP_FLAGS_DEEP_SLEEP_ENABLE,
  .bluetooth.max_connections = MAX_CONNECTIONS,
  .bluetooth.heap = bluetooth_stack_heap,
  .bluetooth.heap_size = sizeof(bluetooth_stack_heap),
  .bluetooth.sleep_clock_accuracy = 100, // ppm
  .gattdb = &bg_gattdb_data,
  .ota.flags = 0,
  .ota.device_name_len = 3,
  .ota.device_name_ptr = "OTA",
#if (HAL_PA_ENABLE) && defined(FEATURE_PA_HIGH_POWER)
  .pa.config_enable = 1, // Enable high power PA
  .pa.input = GECKO_RADIO_PA_INPUT_VBAT, // Configure PA input to VBAT
#endif // (HAL_PA_ENABLE) && defined(FEATURE_PA_HIGH_POWER)
};

const LETIMER_Init_TypeDef letimer=
{
  .enable = true,        /**< Start counting when initialization completes. */
  .debugRun = false ,       /**< Counter shall keep running during debug halt. */
  .comp0Top=true,   /**< Load COMP0 register into CNT when counter underflows. */
  .bufTop=false,      /**< Load COMP1 into COMP0 when REP0 reaches 0. */
  .out0Pol= false ,       /**< Idle value for output 0. */
  .out1Pol = false ,       /**< Idle value for output 1. */
  .ufoa0=letimerUFOAPulse,          /**< Underflow output 0 action. */
  .ufoa1=letimerUFOANone,          /**< Underflow output 1 action. */
  .repMode= letimerRepeatFree       /**< Repeat mode. */
 // uint32_t                   topValue;       /**< Top value. Counter wraps when top value matches counter value is reached. */
};
uint32_t divide=0;
uint32_t selection=0;
uint32_t frequency_1=0;
uint8_t select_sleep=sleepEM2;

void modes(void);
void intialization(void);
void Sleep(void);

#define COMP0 	    (0x1UL) // values taken from efr32bg13p_letimer.h
#define COMP1 	    (0x2UL) // values taken from efr32bg13p_letimer.h
#define TOTAL_P     (2.250) //total period of 2.25 seconds
#define On_Period   (0.175) // on time of 0.175 seconds
#define interrupt    ( COMP0 | COMP1) //bitwise or comp0 and comp1 for interrupt


int x,y;

 void modes(void)
 {
 	if(select_sleep == sleepEM0)
 	{
 		 divide =8;
 		 selection = cmuSelect_LFXO;
 		 frequency_1 = cmuOsc_LFXO;
 	}

 	else if(select_sleep == sleepEM1)
 		{
 			divide =8;
 			selection = cmuSelect_LFXO;
 			frequency_1 = cmuOsc_LFXO;
 		}
 	else if(select_sleep == sleepEM2)
 		{
 			divide =8;
 			selection = cmuSelect_LFXO;
 			frequency_1 = cmuOsc_LFXO;
 		}
 	else if(select_sleep == sleepEM3)
 		{
			//divide =8;
			//selection = cmuSelect_LFXO;
			//frequency_1 = cmuOsc_LFXO;//divide =1;
 		    divide =1;
 		    selection = cmuSelect_ULFRCO;
 			frequency_1 = cmuOsc_ULFRCO;
 		}
 }

 void Sleep(void)
 {

 	if(select_sleep == sleepEM0)
 	{
 		while(1);

 	}

 	else if(select_sleep == sleepEM1)
 		{
 			__WFI();

 		}

 	else if(select_sleep == sleepEM2)
 		{
 		    EMU_EnterEM2(true);

 		}

 	else if(select_sleep == sleepEM3)
 			{
 			    EMU_EnterEM3(false);

 			}

 }

 void intialization(void)
{

	 modes();
	 //LED0 made off
	  gpioLed0SetOff();
	  //LED1 made off
	  gpioLed1SetOff();
	  //LETIMER0

	  CMU_OscillatorEnable(frequency_1,true,true);
	  CMU_ClockSelectSet(cmuClock_LFA,selection);
	  CMU_ClockEnable(cmuClock_CORELE, true);
	  CMU_ClockEnable(cmuClock_LETIMER0, true);
	  CMU_ClockDivSet(cmuClock_LETIMER0,divide);
	  CMU_ClockEnable(cmuClock_GPIO, true);

	  LETIMER_Enable(LETIMER0,false);
 	  LETIMER_RepeatSet(LETIMER0,0,1);
 	  LETIMER_RepeatSet(LETIMER0,1,1);
	  LETIMER_CompareSet(LETIMER0,0,(((CMU_ClockFreqGet(cmuClock_LFA)/(divide))*(TOTAL_P))));
 	  LETIMER_CompareSet(LETIMER0,1,(((CMU_ClockFreqGet(cmuClock_LFA)/(divide))*(On_Period))));
 	  LETIMER_IntSet(LETIMER0,interrupt);
 	  NVIC_EnableIRQ(LETIMER0_IRQn);
	  LETIMER_IntEnable(LETIMER0,interrupt);
 	  LETIMER_Enable(LETIMER0,true);
 	  LETIMER_Init(LETIMER0,&letimer);
 }


void LETIMER0_IRQHandler(void)
{

	LETIMER_IntDisable(LETIMER0,(_LETIMER_IF_COMP0_MASK|_LETIMER_IF_COMP1_MASK)); //Disable Interrupts
	if(_LETIMER_IF_COMP0_MASK & LETIMER_IntGet(LETIMER0))
	{
		gpioLed1SetOff();
		LETIMER_CompareSet(LETIMER0,0,(((CMU_ClockFreqGet(cmuClock_LFA)/(divide))*(TOTAL_P))));
	}
	else
	{
		gpioLed1SetOn();
		SLEEP_SleepBlockBegin(select_sleep);
		LETIMER_CompareSet(LETIMER0,1,(((CMU_ClockFreqGet(cmuClock_LFA)/(divide))*(On_Period))));
	}
	LETIMER_IntClear(LETIMER0,(_LETIMER_IF_COMP0_MASK|_LETIMER_IF_COMP1_MASK));
	LETIMER_IntEnable(LETIMER0,(_LETIMER_IF_COMP0_MASK|_LETIMER_IF_COMP1_MASK));
}

int main(void)
{

  // Initialize device
  initMcu();
  // Initialize board
  initBoard();
  // Initialize application
  initApp();
  //SLEEP_sleep();
  gpioInit();

  intialization();



  SLEEP_InitEx(NULL);

  // Initialize stack
  gecko_init(&config);

  /* Infinite loop */

  while (1) {

	  Sleep();//modes();

  }
}
