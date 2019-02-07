
/*Name :- Raj Lavingia
 * Credits :- Dan Walkes
 *Date : 2/6/19
 */

/* Board headers */
#include <stdbool.h>
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




bool SchedulerEvent;

#define COMP0 	    (0x1UL) // values taken from efr32bg13p_letimer.h
#define COMP1 	    (0x2UL) // values taken from efr32bg13p_letimer.h
#define TOTAL_P     (3.00) //total period of 2.25 seconds
#define On_Period   (0.175) // on time of 0.175 seconds
#define interrupt    ( COMP0 | COMP1) //bitwise or comp0 and comp1 for interrupt


void modes(void);
void intialization(void);
void Sleep(void);
void timerWaitUs(uint32_t);
void I2C_TempInit(void);
void I2C_MeasureAndRead(uint16_t,uint8_t,I2C_TransferSeq_TypeDef,I2C_TransferReturn_TypeDef);

int x,y;
uint8_t ReadBuff[2];
uint8_t WriteBuff[1];
uint16_t Temp;

I2C_TransferSeq_TypeDef seq;
I2C_TransferReturn_TypeDef status;

