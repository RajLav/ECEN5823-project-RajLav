

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


#define INCLUDE_LOG_DEBUG 1
#include "log.h"
#include "display.h"
#include "ble_device_type.h"

#define SCHEDULER_SUPPORTS_DISPLAY_UPDATE_EVENT 1
#define TIMER_SUPPORTS_1HZ_TIMER_EVENT	1

#ifndef MAX_CONNECTIONS
#define MAX_CONNECTIONS               4
#endif


#ifndef MAX_CONNECTIONS
#define MAX_CONNECTIONS 4
#endif

#define TimeSet (1.00)

#define COMP0   ((uint32_t)(0X01))
#define COMP1   ((uint32_t)(0X02))
#define TimerInt  ((uint32_t)(0X03))
#define button_event	(32)
#define ButtonMask	(0x40)
 void TimerInitialize(void);

 void LoggerTimeStamp(void);
 void timerEnable1HzSchedulerEvent(uint32_t Scheduler_DisplayUpdate);

 void EnableGPIOInterrupts(void);
 void initDefines(void);

 struct gecko_cmd_packet* evt;
 struct gecko_msg_system_get_bt_address_rsp_t * AddrBLE;

 uint8_t roll;
 uint8_t DetectEvent;
 uint32_t  timer_trigg;
 uint32_t mask;
 uint8_t ButtonToggle;
 uint8_t bootToDfu;
 uint8_t ButtonInitiationFlag;
 uint8_t DisplayButtonState;
 uint32_t update_display;


#define Comp0Val 	((uint32_t)(CMU_ClockFreqGet(cmuClock_LFA)/(4*TimeSet)))
#define Logging()				(float)(roll+(0.001*((LETIMER_CounterGet(LETIMER0)*4*1000)/CMU_ClockFreqGet(cmuClock_LFA))))
