/***************************************************************************//**
 * @file
 * @brief Silicon Labs BT Mesh Empty Example Project
 * This example demonstrates the bare minimum needed for a Blue Gecko BT Mesh C application.
 * The application starts unprovisioned Beaconing after boot
 *******************************************************************************
 * # License
 * <b>Copyright 2018 Silicon Laboratories Inc. www.silabs.com</b>
 *******************************************************************************
 *
 * The licensor of this software is Silicon Laboratories Inc. Your use of this
 * software is governed by the terms of Silicon Labs Master Software License
 * Agreement (MSLA) available at
 * www.silabs.com/about-us/legal/master-software-license-agreement. This
 * software is distributed to you in Source Code format and is governed by the
 * sections of the MSLA applicable to Source Code.
 *
 ******************************************************************************/

/******************************************************************************
 * COMMON : adc.c,adc.h and PWM_Init() are common files between Yash Gupte and
 * 			Raj Lavingia.
 ******************************************************************************/

 /******************************************************************************
 * Sensor:
 * 		   @Name: IR sensor
 *@communication: ADC
 *		   @pins: P7 - ADC | VCC - 3.3 | GND
 *	_________________________________________________________________________
 *	Actuator:
 *	 	   @Name: Servo
 *@communication: PWM
 *		   @pins: P12 - PWM | GND
 *******************************************************************************/

#include <src/Defines.h>
#include <src/IR.h>
#include <src/PWM.h>
#include "src/gpio.h"
#include "native_gecko.h"
#include "src/display.h"
#include "src/LETIMER.h"
#include "src/buzzer.h"
#include "src/display.h"

uint8_t tmp_state = 0;


const char* ButtonState[] = {"Button Pressed","Button Released"};
char* ServoState[]={"Unlock","Lock"};

// bluetooth stack heap
#define MAX_CONNECTIONS 2

uint8_t bluetooth_stack_heap[DEFAULT_BLUETOOTH_HEAP(MAX_CONNECTIONS) + BTMESH_HEAP_SIZE + 1760];

// Bluetooth advertisement set configuration
//
// At minimum the following is required:
// * One advertisement set for Bluetooth LE stack (handle number 0)
// * One advertisement set for Mesh data (handle number 1)
// * One advertisement set for Mesh unprovisioned beacons (handle number 2)
// * One advertisement set for Mesh unprovisioned URI (handle number 3)
// * N advertisement sets for Mesh GATT service advertisements
// (one for each network key, handle numbers 4 .. N+3)
//
#define MAX_ADVERTISERS (4 + MESH_CFG_MAX_NETKEYS)  //Change ?

static gecko_bluetooth_ll_priorities linklayer_priorities = GECKO_BLUETOOTH_PRIORITIES_DEFAULT;

// bluetooth stack configuration
extern const struct bg_gattdb_def bg_gattdb_data;

// Flag for indicating DFU Reset must be performed
uint8_t reboot = 0;

const gecko_configuration_t config =
{
  .sleep.flags=SLEEP_FLAGS_DEEP_SLEEP_ENABLE,
  .bluetooth.max_connections = MAX_CONNECTIONS,
  .bluetooth.max_advertisers = MAX_ADVERTISERS,
  .bluetooth.heap = bluetooth_stack_heap,
  .bluetooth.heap_size = sizeof(bluetooth_stack_heap) - BTMESH_HEAP_SIZE,
  .bluetooth.sleep_clock_accuracy = 100,
  .bluetooth.linklayer_priorities = &linklayer_priorities,
  .gattdb = &bg_gattdb_data,
  .btmesh_heap_size = BTMESH_HEAP_SIZE,
#if (HAL_PA_ENABLE)
  .pa.config_enable = 1, // Set this to be a valid PA config
#if defined(FEATURE_PA_INPUT_FROM_VBAT)
  .pa.input = GECKO_RADIO_PA_INPUT_VBAT, // Configure PA input to VBAT
#else
  .pa.input = GECKO_RADIO_PA_INPUT_DCDC,
#endif // defined(FEATURE_PA_INPUT_FROM_VBAT)
#endif // (HAL_PA_ENABLE)
  .max_timers = 16,
};

/**
 * See light switch app.c file definition
 */
void gecko_bgapi_classes_init_server_friend(void)
{
  gecko_bgapi_class_dfu_init();
  gecko_bgapi_class_system_init();
  gecko_bgapi_class_le_gap_init();
  gecko_bgapi_class_le_connection_init();
  gecko_bgapi_class_gatt_init();        //Additional
  gecko_bgapi_class_gatt_server_init();
//  gecko_bgapi_class_endpoint_init();//not found in native_gecko.h
  gecko_bgapi_class_hardware_init();
  gecko_bgapi_class_flash_init();
  gecko_bgapi_class_test_init();
  //gecko_bgapi_class_sm_init();//for oob
  mesh_native_bgapi_init();         //Additional
  gecko_bgapi_class_mesh_node_init();
  //gecko_bgapi_class_mesh_prov_init();
  gecko_bgapi_class_mesh_proxy_init();
  gecko_bgapi_class_mesh_proxy_server_init();
  //gecko_bgapi_class_mesh_proxy_client_init();
  gecko_bgapi_class_mesh_generic_client_init();
  gecko_bgapi_class_mesh_generic_server_init();
  //gecko_bgapi_class_mesh_vendor_model_init();
  //gecko_bgapi_class_mesh_health_client_init();
  //gecko_bgapi_class_mesh_health_server_init();
  //gecko_bgapi_class_mesh_test_init();
  //gecko_bgapi_class_mesh_lpn_init();
  gecko_bgapi_class_mesh_friend_init();
}


/**
 * See main function list in soc-btmesh-switch project file
 */
void gecko_bgapi_classes_init_client_lpn(void)
{
  gecko_bgapi_class_dfu_init();
  gecko_bgapi_class_system_init();
  gecko_bgapi_class_le_gap_init();
  gecko_bgapi_class_le_connection_init();
  gecko_bgapi_class_gatt_init();
  gecko_bgapi_class_gatt_server_init();
  gecko_bgapi_class_hardware_init();
  gecko_bgapi_class_flash_init();
  gecko_bgapi_class_test_init();
  mesh_native_bgapi_init();
  gecko_bgapi_class_mesh_node_init();
  gecko_bgapi_class_mesh_proxy_init();
  gecko_bgapi_class_mesh_proxy_server_init();
  gecko_bgapi_class_mesh_generic_server_init();
  gecko_bgapi_class_mesh_lpn_init();
}


void Factory_Reset(void)
{
  LOG_INFO("Factory Reset:[TIME]%f\n",log_val());
  displayPrintf(DISPLAY_ROW_ACTION,"Factory Reset");

  if (connHand != 0xFF) {
    gecko_cmd_le_connection_close(connHand);
  }

  gecko_cmd_flash_ps_erase_all();
  gecko_cmd_hardware_set_soft_timer(2*32768, ResetID, 1);
}


void set_device_name(bd_addr *DeviceAddress)
{
 sprintf(DisplayString, "Subscriber: %x:%x", DeviceAddress->addr[1], DeviceAddress->addr[0]);
 displayPrintf(DISPLAY_ROW_NAME,"Subscriber");
 displayPrintf(DISPLAY_ROW_BTADDR2,"5823Sub%02x:%02x",DeviceAddress->addr[1],DeviceAddress->addr[0]);
 resp = gecko_cmd_gatt_server_write_attribute_value(gattdb_device_name, 0, strlen(DisplayString), (uint8 *)DisplayString)->result;
 if (resp) {
    LOG_INFO("Write attribute failed,[ERROR]%x:[TIME]%f\n",resp,log_val());
 }
 LOG_INFO("Device: '%s':[TIME]%f\n", DisplayString,log_val());
}

/****************************************************************************
 * @brief: Initializes LPN node by calling gecko_cmd_mesh_lpn_init()
 * 		   gecko_cmd_mesh_lpn_init(),gecko_cmd_mesh_lpn_establish_friendship().
 ****************************************************************************/
void Init_LPN(void)
{
    struct gecko_msg_mesh_lpn_init_rsp_t *lpnInit = gecko_cmd_mesh_lpn_init();
    if(lpnInit->result)
    	LOG_INFO("[ERROR] Initializing node");
    else
    	LOG_INFO("Initializing LPN node");

    struct gecko_msg_mesh_lpn_configure_rsp_t *lpnConfig = gecko_cmd_mesh_lpn_init(2,5000);
    if(lpnConfig->result)
    	LOG_INFO("[ERROR] configuring LPN node");
    else
    	LOG_INFO("configuring LPN node Success");

    struct gecko_msg_mesh_lpn_establish_friendship_rsp_t *lpnEst=gecko_cmd_mesh_lpn_establish_friendship(0);
    if(lpnEst->result)
    	LOG_INFO("Finding friend command error");
    else
    	LOG_INFO(" LPN node Success");
}

/****************************************************************************
 * @brief: Initializes all baic I/O devices sets board Clock etc.
 ****************************************************************************/

void gecko_main_init()
{
	initMcu();
	initBoard();
	initApp();
	GPIO_Int_Enable();
	initADC();
	BuzzerInit();
	PWMInit();
	elemID = 0xffff;
	AppKey = 0;
	transID = 0;
	adc_convert=false;
	connHand = 0xFF;        /* handle of the last opened LE connection */
	DisplayString[20];
	primary_address = 0;    /* Address of the Primary Element of the Node */
	linklayer_priorities.scan_max = linklayer_priorities.adv_min + 1;
	gecko_stack_init(&config);
	gecko_bgapi_classes_init_client_lpn();
	gecko_initCoexHAL();
}

void handle_gecko_event(uint32_t evt_id, struct gecko_cmd_packet *evt)
{

  struct gecko_msg_mesh_node_provisioning_failed_evt_t  *FailedProvEvent;

  switch (evt_id) {
    case gecko_evt_system_boot_id:
    if(!(GPIO_PinInGet(PortB1,PinB1))|| !(GPIO_PinInGet(PortB0,PinB0))) Factory_Reset();

    else
    {
	  struct gecko_msg_flash_ps_load_rsp_t *load_val=gecko_cmd_flash_ps_load(STORE_KEY);
	  if(load_val->result)
	  {
		  servo_state=0;
	  }
	  else
	  {
		  servo_state=load_val->value.data[0];
	  }

      LOG_INFO("Booted ID\n");

      struct gecko_msg_system_get_bt_address_rsp_t *BT_Addr = gecko_cmd_system_get_bt_address();
      set_device_name(&BT_Addr->address);
      resp = gecko_cmd_mesh_node_init()->result;
      if(resp != 0)
    	  LOG_INFO("[ERROR]: 0x%x:[TIME]%f\n", resp,log_val());
    }
      break;

/****************************************************************************
* @brief: Cases for different hardware soft-timer cases.
* ResetID     -  Calls system factory reset.
* provisionID -  Indicates provisioning started.
* NoFriendID  -  Calls Initializing LPN again to scan for friend.
****************************************************************************/
  case gecko_evt_hardware_soft_timer_id:

        switch(evt->data.evt_hardware_soft_timer.handle)
        {

          case ResetID:
            gecko_cmd_system_reset(0);
            break;

          case  provisionID:
            LOG_INFO("\n\rBEGIN\n:[TIME]%f\n\r",log_val());
            break;
          case NoFriendID:
            Init_LPN();
            break;

          default:
            break;
        }
        break;

/****************************************************************************
* @brief: Initializes Node and sets it up as LPN
****************************************************************************/
case gecko_evt_mesh_node_initialized_id:
	LOG_INFO("Node Init:[TIME]%f\n",log_val());
	if(gecko_cmd_mesh_generic_server_init()->result)
      LOG_INFO("Client Init Fail==[ERROR]CODE: %x:[TIME]%f\n",resp,log_val());

	struct gecko_msg_mesh_node_initialized_evt_t *evt_data = (struct gecko_msg_mesh_node_initialized_evt_t *)&(evt->data);
	if(evt_data->provisioned)
	{
	  LOG_INFO("Provision Succcess:%x, ivi:%ld:[TIME]%f\n", evt_data->address, evt_data->ivi,log_val());
	  displayPrintf(DISPLAY_ROW_CONNECTION,"Provisioned");
	  primary_address = evt_data->address;
	  elemID = 0;
	  mesh_lib_init(malloc, free, 8);
	  Init_LPN();
	}

	else
	{
	  LOG_INFO("Node Unprovisioned:[TIME]%f\n",log_val());
	  LOG_INFO("Unprovisioned Beaconing Started...:[TIME]%f\n",log_val());
	  gecko_cmd_mesh_node_start_unprov_beaconing(0x3);
	}
	break;

/****************************************************************************
* @brief: Indicates provisioning has begun. Displays the same on the LCD.
****************************************************************************/
case gecko_evt_mesh_node_provisioning_started_id:
	LOG_INFO("Provisioning Begun:[TIME]%f\n",log_val());
	displayPrintf(DISPLAY_ROW_CONNECTION,"Provisioning");
	break;

/****************************************************************************
* @brief: Event when friendship fails. Scans for friend after timeout.
****************************************************************************/
case gecko_evt_mesh_lpn_friendship_failed_id:
      LOG_INFO("!!!!!!!!!!!!!!!!Friendship Failed!!!!!!!!!!!!!!!!!!!");
      gecko_cmd_hardware_set_soft_timer(2*32768,NoFriendID,1);
  break;

/****************************************************************************
* @brief: Indicates Friendship established. Displays the Node name on LCD.
****************************************************************************/
case gecko_evt_mesh_lpn_friendship_established_id:
	LOG_INFO(">>>>>>>>>>>Friendship Done>>>>>>>>>>>>>>>>>");
	displayPrintf(DISPLAY_ROW_CONNECTION,"LowPower-Node1");
  break;


/****************************************************************************
* @brief: Indicates Friendship terminated. Displays the code or the same on UART.
****************************************************************************/
case gecko_evt_mesh_lpn_friendship_terminated_id:
	LOG_INFO("!!!!!!!!!!!!!!!!Friendship Terminated!!!!!!!!!!!!!!!!");
	displayPrintf(DISPLAY_ROW_CONNECTION,"Terminated");
	uint16_t reason;
	reason = evt->data.evt_mesh_lpn_friendship_terminated.reason;
	LOG_INFO("code:%x",reason);
	gecko_cmd_hardware_set_soft_timer(2*32768,NoFriendID,1);
	break;

/****************************************************************************
* @brief: Indicates status of Friendship.
****************************************************************************/
case gecko_evt_mesh_config_client_lpn_polltimeout_status_id:
	{
	uint32_t timeout;
	timeout = evt->data.evt_mesh_config_client_lpn_polltimeout_status.poll_timeout_ms;
	if(!timeout)
		LOG_INFO("Friend-ship broken\n");
	else
		LOG_INFO("Friendship exists\n");
	break;
	}


/****************************************************************************
* @brief: Indicates provisioning of node.
****************************************************************************/
case gecko_evt_mesh_node_provisioned_id:
        elemID = 0;
        mesh_lib_init(malloc, free, 8);
        LOG_INFO("Provisioned with Address:%x:[TIME]%f\n", evt->data.evt_mesh_node_provisioned.address,log_val());
        displayPrintf(DISPLAY_ROW_CONNECTION,"Provisioned");
        Init_LPN();
        break;

/****************************************************************************
* @brief: Indicates provisioning failed.
****************************************************************************/
case gecko_evt_mesh_node_provisioning_failed_id:

	FailedProvEvent = (struct gecko_msg_mesh_node_provisioning_failed_evt_t  *)&(evt->data);
	LOG_INFO("Provisioning failed, code %x:[TIME]%f\n", FailedProvEvent->result,log_val());
	displayPrintf(DISPLAY_ROW_CONNECTION,"Provision Failed");
	gecko_cmd_hardware_set_soft_timer(2*32768, ResetID, 1);
	break;

/****************************************************************************
* @brief: Mesh node key added event.
****************************************************************************/
case gecko_evt_mesh_node_key_added_id:

	if(evt->data.evt_mesh_node_key_added.type)
	  LOG_INFO("App Key added is %f",evt->data.evt_mesh_node_key_added.index,log_val());
	else
	  LOG_INFO("Net Key added is %f",evt->data.evt_mesh_node_key_added.index,log_val());

	if(evt->data.evt_mesh_node_key_added.type == Application_Key_Type)
	{
		AppKey = evt->data.evt_mesh_node_key_added.index;
		if(AppKey==0)
			NetKey=0;
	}
	break;

/****************************************************************************
* @brief: Connection opened event
****************************************************************************/
  case gecko_evt_le_connection_opened_id:
      LOG_INFO("Conn Open ID:[TIME]%f\n",log_val());
      connHand = evt->data.evt_le_connection_opened.connection;
      break;

/****************************************************************************
* @brief: Connection parameters event.
****************************************************************************/
case gecko_evt_le_connection_parameters_id:
  LOG_INFO("Connection_Parameters_ID:[TIME]%f\n",log_val());
  break;

/****************************************************************************
* @brief: Event to detect data from client. Friend-Node.
****************************************************************************/
case gecko_evt_mesh_generic_server_client_request_id:
{
  CORE_DECLARE_IRQ_STATE;
		static int16_t  StoreVal = 0;
		uint16_t model_ID = evt->data.evt_mesh_generic_server_client_request.model_id;
		LOG_INFO("Model_id:%x",model_ID);
		if(model_ID==MESH_GENERIC_ON_OFF_SERVER_MODEL_ID)			//Checks if model id belongs to ON_OFF model.
		{															//If yes, it regards it as door open command.
			LOG_INFO("Server_Client_Request_ID:[TIME]%f\n",log_val());
			LOG_INFO("Door Open Command Received\n");
			CORE_ENTER_CRITICAL();
				door_open=true;
			CORE_EXIT_CRITICAL();
			ServoPosition(unlock);//PWMFunction(true,true);//UnlockDoor();
			displayPrintf(DISPLAY_ROW_ACTION, "Door Open");
			IRStart();
		}
		else														//If model is is level, it checks what the level is.
		{
			StoreVal = evt->data.evt_mesh_generic_server_client_request.parameters.data[1]<<8;
			StoreVal |= evt->data.evt_mesh_generic_server_client_request.parameters.data[0];
			LOG_INFO("Data:%d",StoreVal);

			 if(StoreVal == SMOKE)									//Servo is locked and "Smoke" is displayed on LCD.
			  {
					LOG_INFO("Server_Client_Request_ID:[TIME]%f\n",log_val());
					LOG_INFO("Smoke Detected\n");
					ServoPosition(unlock);//PWMFunction(true,true);//UnlockDoor();
					displayPrintf(DISPLAY_ROW_ACTION, "Smoke");
			  }
			  else if(StoreVal == OVERRIDE)						//Servo is unlocked and "Override" is displayed on LCD.
			  {
					LOG_INFO("Server_Client_Request_ID:[TIME]%f\n",log_val());
					LOG_INFO("Override Detected\n");
					ServoPosition(lock);//PWMFunction(true,false);//LockDoor();
					displayPrintf(DISPLAY_ROW_ACTION, "Override");
			  }
		}

	  break;
  }

/****************************************************************************
* @brief: Reboots system on shut-down.
****************************************************************************/
  case gecko_evt_le_connection_closed_id:
      if (reboot)
        gecko_cmd_system_reset(2);
      break;


  case gecko_evt_system_external_signal_id:
	  LOG_INFO("External ID");
	  static uint8_t i=0;
      DetectEvent = evt->data.evt_system_external_signal.extsignals;
      CORE_DECLARE_IRQ_STATE;
      if(DetectEvent & DispUpdate)
      {

        CORE_ENTER_CRITICAL();
        event_mask &= ~DispUpdate;
        CORE_EXIT_CRITICAL();
  //      TIMER_Enable(TIMER0,false);
        displayUpdate();
        clearDisplay();
//        struct gecko_msg_flash_ps_load_rsp_t *load_val=gecko_cmd_flash_ps_load(STORE_KEY);
//		if(load_val->result==0)
//			LOG_INFO("Door Unlock count:%d",servo_state);
      }

       if(DetectEvent & timer_trigg)
       {
    	   CORE_ENTER_CRITICAL();
    	   	   event_mask &= ~timer_trigg;
		  CORE_EXIT_CRITICAL();
		  IRRead();

       }

      if(DetectEvent & button_event)
      {
		  CORE_ENTER_CRITICAL();
		  	  event_mask &= ~button_event;
		  CORE_EXIT_CRITICAL();
		  struct gecko_msg_flash_ps_load_rsp_t *load_val=gecko_cmd_flash_ps_load(STORE_KEY);
		  if(load_val->result==0)
				LOG_INFO("Door Unlock count:%d",servo_state);
		  LOG_INFO("%sPublishing:[TIME]%f\n",ButtonState[ButtonToggle],log_val());  //display printf state.
		  displayPrintf(DISPLAY_ROW_TEMPVALUE,"%s",ButtonState[ButtonToggle]);
		  transID+=1;


	struct mesh_generic_state current;
	errorcode_t response_update, response_publish;

	current.kind = mesh_generic_state_on_off;
	current.on_off.on = LED_ON;//ButtonToggle;

	response_update = mesh_lib_generic_server_update(MESH_GENERIC_ON_OFF_SERVER_MODEL_ID,0,&current,NULL,0);

     if (response_update)
     {
    	 LOG_INFO("gecko_cmd_mesh_generic_client_publish failed,code %d:TimeStamp=>%f\n", response_update,log_val());

     }
	  else
	  {
		 LOG_INFO("Server Updated:TimeStamp=>%f\n",log_val());
		 response_publish=mesh_lib_generic_server_publish(MESH_GENERIC_ON_OFF_SERVER_MODEL_ID,
				 0, mesh_generic_state_on_off);
		 if(response_publish)
			 LOG_INFO("Not pubished,code:%x,Time:%f\n",response_publish,log_val());
		 else
			 LOG_INFO("Server Published:%f\n",log_val());
      }
    }
    break;
  default:
      break;
  }
}

void GPIO_Int_Enable(void)
{
    NVIC_EnableIRQ(GPIO_EVEN_IRQn);
    NVIC_EnableIRQ(GPIO_ODD_IRQn);
}
