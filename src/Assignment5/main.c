/* Name :- Raj Lavingia
Credits : Dan Walkes
Date :- 2/20/19
Reference :- Help Taken from Simplicity studio Demo code for heath thermometer
*/

#include "LETIMER.h"

#include "init_mcu.h"
#include "init_board.h"
#include "init_app.h"
#include "ble-configuration.h"
#include "board_features.h"


#include "bg_types.h"
#include "native_gecko.h"
#include "gatt_db.h"


#include "em_emu.h"
#include "em_cmu.h"


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

#include "log.h"
#include "i2cspm.h"

#ifndef MAX_CONNECTIONS
#define MAX_CONNECTIONS 4
#endif


uint8_t bluetooth_stack_heap[DEFAULT_BLUETOOTH_HEAP(MAX_CONNECTIONS)];

int main(void)
{
	//Variables initialised before running the gecko init function
	Report_Check = 0;
	Event_Status_Retun_Back = 0;
	Para_Passed = 1;
	Connection_Established=0;

  initMcu();

  initBoard();

  initApp();

//For log initialisation
  logInit();

  //Init_Globals();

  intialization();

  // Initialise serial interface
    RETARGET_SerialInit();

  //Gecko main initialisation
  gecko_init(&config);

  	while(1)
  	{
  		 // Always flush the UART buffer before letting the device go to sleep
  		   RETARGET_SerialFlush();
  		evt=gecko_wait_event();
  		gecko_custom_update(evt);
  	}

}
