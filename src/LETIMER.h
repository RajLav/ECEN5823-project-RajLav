/* Name :- Raj Lavingia
Credits : Dan Walkes
Date :- 3/20/19
*/

/* Board headers */
#include "infrastructure.h"
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
#include "gpio.h"

#include "sleep.h"
#include "em_letimer.h"
#include "em_assert.h"
#include "em_core.h"
#include "em_rmu.h"

#include "i2cspm.h"
#include "em_i2c.h"

#include "log.h"
#include "display.h"

#ifndef MAX_CONNECTIONS
#define MAX_CONNECTIONS 4
#endif

#include "ble_device_type.h"

#define SCHEDULER_SUPPORTS_DISPLAY_UPDATE_EVENT 1
#define TIMER_SUPPORTS_1HZ_TIMER_EVENT  1
#define INT_MIN_DURATION             80
#define INT_MAX_DURATION             80
#define SLAVE_LATENCY            0
#define TIMEOUT                  100

#define INTERVAL                 16
#define WINDOW                   16
#define PASSIVE                  0

#define TEMP_INVALID                  (uint32_t)0xFFFFFFFFu
#define RSSI_INVALID                  (int8_t)127
#define CONNECTION_HANDLE_INVALID     (uint8_t)0xFFu
#define SERVICE_HANDLE_INVALID        (uint32_t)0xFFFFFFFFu
#define CHARACTERISTIC_HANDLE_INVALID (uint16_t)0xFFFFu
#define STATUS_INVALID           (uint8_t)0xFFu

#define bit_mask_external_event      (uint32_t)(16)

bd_addr ServerAddress;

uint8_t bluetooth_stack_heap[DEFAULT_BLUETOOTH_HEAP(MAX_CONNECTIONS)];

typedef enum {
  scanning,
  opening,
  discoverServices,
  discoverCharacteristics,
  enableIndication,
  running
} CONN_STATE;

typedef struct {
  uint8_t  connectionHandle;
  int8_t   rssi;
  uint16_t serverAddress;
  uint32_t thermometerServiceHandle;
  uint16_t thermometerCharacteristicHandle;
  uint32_t temperature;

  uint32_t	buttonServiceHandle;
  uint16_t buttonCharacteristicHandle;
  uint32_t button;


} CONN_PROPERTIES;

// Flag for indicating DFU Reset must be performed
uint8_t BOOT;
// Array for holding properties of multiple (parallel) connections
CONN_PROPERTIES connProperties;
// State of the connection under establishment
CONN_STATE present_state;
// Health Thermometer service UUID defined by Bluetooth SIG
 uint8_t SERVICE[2]; //
// Temperature Measurement characteristic UUID defined by Bluetooth SIG
 uint8_t CHARACTERS[2]; //= { 0x1c, 0x2a };
///////////////////////////////////////////
 uint8_t buttonSERVICE[16];

 uint8_t buttonCHARACTERS[16];
//////////////////////////////////////////

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

#define TOTAL_P (1.00) //Total period of 3sec , every 3 sec asks for temperature
#define COMP0       (0x1UL) // values taken from efr32bg13p_letimer.h
#define COMP1       (0x2UL) // values taken from efr32bg13p_letimer.h
#define interrupt  ( COMP0 | COMP1) //bitwise or comp0 and comp1 for interrupt


typedef enum {I2CComplete,Error,TurnPowerOff,On,InitWrite,Idle,In_progress,Incomplete,SleepDeep,Wait,Sleep1}Event;
typedef enum{Read,Write}WriteRead;

#define CounterGet(us_wait)((uint32_t)(((us_wait*0.000001*CMU_ClockFreqGet(cmuClock_LFA))/divide)))

uint32_t TimerTicks,ReqTicks,divide,selection,frequency_1,select_sleep,address,address_2;

//Defined for read and write
I2C_TransferSeq_TypeDef    seq;

//Check if the return is successful or not
I2C_TransferReturn_TypeDef return_check;

//Structure defined for gecko event
struct gecko_cmd_packet* evt;
struct gecko_msg_system_get_bt_address_rsp_t * Addressptr;

WriteRead Write_Read;
uint8_t i2c_read_data[2]; //read data from sensor
uint8_t i2c_write_data[1]; //wrote data to sensor
uint16_t data;
float temp;
Event event;

//Varibales declared for HT
uint8_t Slave_Latency_1,Event_Status_Retun_Back,Connection_Minimum,Connection_Maximum,Connection_over;
uint8_t Open_Close_Connection;
uint8_t Report_Check,flags,Para_Passed,Connection_Established,Minimum_Power_Set,Maximum_power_Set;
uint32_t rssi_value_owndefined;
uint32_t temperature;
uint8_t temp11[50];
uint8_t roll;
uint32_t mask;
uint32_t mask_I2C;
uint8_t flag;
uint32_t update_display;
bool HEADER;
uint8_t* charValue;
uint8_t temperory_variable;
uint16_t addrValue;
uint8_t statusRet;
uint32_t Connection_Not_Established_Server;
int GPIO_Return_Key;
 uint8_t Connection_Established_Server_Success;
  bool Passkey_Status_Check;
  bool Passkey_Bonding_Status_Fail;
  uint8_t Connection_Established_Server_Success;
  uint32_t passphrase_key;
  uint8_t *pointer;
	uint8_t flag_array_define[2];
	uint8_t Flag_Status_Check;
	uint8_t *ptr, *ptr3;
	uint16_t check_value;
	void Button_UUID();

uint8_t ButtonState;
uint8_t Button_Status_Start_Time;
