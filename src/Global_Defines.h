
#ifndef SRC_GLOBAL_DEFINES_H_
#define SRC_GLOBAL_DEFINES_H_


#include "LETIMER.h"

#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include "init_mcu.h"
#include "init_board.h"
#include "init_app.h"
#include "ble-configuration.h"
#include "board_features.h"
#include "retargetserial.h"
#include "bg_types.h"
#include "native_gecko.h"
#include "gatt_db.h"
#include <gecko_configuration.h>
#include "mesh_generic_model_capi_types.h"
#include "mesh_lib.h"
#include <mesh_sizes.h>
#include "init_mcu.h"
#include "init_board.h"
#include "init_app.h"
#include "ble-configuration.h"
#include "board_features.h"
#include "bg_types.h"
#include "native_gecko.h"
#include "gatt_db.h"
#include <gecko_configuration.h>
#include <mesh_sizes.h>
#include "em_emu.h"
#include "em_cmu.h"
#include "em_core.h"
#include <em_gpio.h>
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

#define RESET_TIME			(2*32768)

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

uint16_t resp;

char print[3];

#endif
