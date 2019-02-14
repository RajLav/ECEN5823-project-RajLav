/* Name :- Raj Lavingia
Credits : Dan Walkes
*/

/* Board headers */
#include "init_mcu.h"
#include "init_board.h"
#include "init_app.h"
#include "ble-configuration.h"
#include "board_features.h"

/* Bluetooth stack headers */
#include "bg_types.h"
#include "native_gecko.h"
#include "gatt_db.h"

/* Libraries containing default Gecko configuration values */
#include "em_emu.h"
#include "em_cmu.h"

/* Device initialization header */
#include "hal-config.h"

#if defined(HAL_CONFIG)
#include "bsphalconfig.h"
#else
#include "bspconfig.h"
#endif

#include "em_device.h"
#include "em_chip.h"
#include "src/gpio.h"

#include "sleep.h"
#include "em_letimer.h"
#include "em_assert.h"
#include "em_core.h"
#include "em_rmu.h"

#include "i2cspm.h"
#include "em_i2c.h"
#define INCLUDE_LOG_DEBUG 1
#include "log.h"

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

#define TOTAL_P (3.00)
#define COMP0 	    (0x1UL) // values taken from efr32bg13p_letimer.h
#define COMP1 	    (0x2UL) // values taken from efr32bg13p_letimer.h
#define interrupt  ( COMP0 | COMP1) //bitwise or comp0 and comp1 for interrupt
#define SLEEP_MODE  sleepEM3

typedef enum {I2CComplete,Error,TurnPowerOff,On,InitWrite,Idle,In_progress,Incomplete,SleepDeep,Wait,Sleep1}Event;
typedef enum{Read,Write}WriteRead;

I2C_TransferReturn_TypeDef I2C_Status;

#define CounterGet(us_wait)   ((uint32_t)(((us_wait*0.000001*CMU_ClockFreqGet(cmuClock_LFA))/divide)))
uint32_t TimerTicks,ReqTicks,divide,selection,frequency_1,select_sleep;

