
#ifndef SRC_GLOBAL_DEFINES_H_
#define SRC_GLOBAL_DEFINES_H_


#include "LETIMER.h"

/* C Standard Library headers */
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>

/* Board headers */
#include "init_mcu.h"
#include "init_board.h"
#include "init_app.h"
#include "ble-configuration.h"
#include "board_features.h"
#include "retargetserial.h"

/* Bluetooth stack headers */
#include "bg_types.h"
#include "native_gecko.h"
#include "gatt_db.h"
#include <gecko_configuration.h>
#include "mesh_generic_model_capi_types.h"
#include "mesh_lib.h"
#include <mesh_sizes.h>

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
#include <gecko_configuration.h>
#include <mesh_sizes.h>

/* Libraries containing default Gecko configuration values */
#include "em_emu.h"
#include "em_cmu.h"
#include "em_core.h"
#include <em_gpio.h>

/* Device initialization header */
#include "hal-config.h"

#if defined(HAL_CONFIG)
#include "bsphalconfig.h"
#else
#include "bspconfig.h"
#endif
#include "src/ble_mesh_device_type.h"

#include <stdbool.h>
#include "native_gecko.h"
#include "log.h"

#define Application_Key_Type					1

#define ProvisionTimeout	10

#define Event_Button0_Pressed		1
#define Event_Button1_Pressed		2

#define restartID    			1
#define factoryID  				2
#define debounceID				3
#define provisionID				4
#define FriendNotFoundID		5

#define RESET_TIME			(2*32768)

#define DOOROPEN	(int16_t)(100)
#define OVERRIDE	(int16_t)(200)
#define LED_ON		(int16_t)(300)
#define SMOKE		(int16_t)(400)

#define SET_VAL		(uint16_t)(0X7D0)

uint8_t override_count;

extern void handle_gecko_event(uint32_t evt_id, struct gecko_cmd_packet *evt);
extern void mesh_native_bgapi_init(void);
extern bool mesh_bgapi_listener(struct gecko_cmd_packet *evt);

uint16_t ElementID = 0xffff;
uint16_t appkeyindex = 0;
uint8_t transaction_id = 0;
uint8_t conn_handle = 0xFF;

char StringToDisplay[20];

uint16 primary_address = 0;
uint8 NumOfConn = 0;

uint8_t oob_array[16]={1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16};
uint16_t resp;

char print[3];

#endif /* SRC_GLOBAL_DEFINES_H_ */
