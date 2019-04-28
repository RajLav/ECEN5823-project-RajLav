
#ifndef SRC_DEFINES_H_
#define SRC_DEFINES_H_


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

#define ResetID  				1
#define provisionID				2
#define NoFriendID				3

#define DOOROPEN	(uint8_t)(1)
#define OVERRIDE	(int16_t)(200)
#define LED_ON		(uint8_t)(1)
#define SMOKE		(int16_t)(400)
#define ADDR_SEND	(uint8_t)(0)
#define STORE_KEY	(uint16_t)(0x4000)

uint16_t elemID = 0xffff;
uint16_t AppKey = 2;
uint16_t NetKey = 0;
uint8_t transID = 0;
uint8_t connHand = 0xFF;
char DisplayString[20];
uint16 primary_address = 0;
uint16_t resp;

extern void handle_gecko_event(uint32_t evt_id, struct gecko_cmd_packet *evt);
extern void mesh_native_bgapi_init(void);
extern bool mesh_bgapi_listener(struct gecko_cmd_packet *evt);

#endif /* SRC_DEFINES_H_ */
