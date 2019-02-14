/* Name :- Raj Lavingia
Credits : Dan Walkes
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

uint16_t addr;
uint8_t command;
uint8_t i2c_read_data[2];
uint8_t i2c_write_data[1];
uint16_t data;
float temp;
uint8_t roll;
uint8_t SleepMode;
WriteRead Write_Read;
Event event;

int main(void)
{
	event = Idle;
	SleepMode = sleepEM0;
	roll = 0;
	select_sleep = sleepEM3;
	Write_Read = Read;
	addr = 0x80;
	command = 0xE3;

  initMcu();

  initBoard();

  initApp();

  logInit();

  gpioInit();

  intialization();

  gecko_init(&config);

  SLEEP_InitEx(NULL);

  	while(1)
  	{
  		Event_Handler();
  		//modes();
  	}


}
