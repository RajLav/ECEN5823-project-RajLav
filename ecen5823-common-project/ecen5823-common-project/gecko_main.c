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


/*******************************************************************************
 * COMMON: This code is common between Raj Lavingia and Yash Gupte - Project
 * Partners.
 ******************************************************************************/

#include "src/gpio.h"
#include "native_gecko.h"
#include "src/display.h"
#include "src/LETIMER.h"
#include "src/Global_Defines.h"
#include "src/adc.h"
#include "src/smoke.h"

uint8_t tmp_state = 0;


const char* ButtonState[] = {"Button Pressed","Button Released"};

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


uint8_t BootID = 0;

const gecko_configuration_t config =
{
  .sleep.flags=SLEEP_FLAGS_DEEP_SLEEP_ENABLE,                                   //Ensures Sleep mode is achieved.
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
  gecko_bgapi_class_gatt_init();//Additional
  gecko_bgapi_class_gatt_server_init();
  //  gecko_bgapi_class_endpoint_init();//not found in native_gecko.h
  gecko_bgapi_class_hardware_init();
  gecko_bgapi_class_flash_init();
  gecko_bgapi_class_test_init();
  //gecko_bgapi_class_sm_init();//for oob
  mesh_native_bgapi_init();     //Additional
  gecko_bgapi_class_mesh_node_init();
  //gecko_bgapi_class_mesh_prov_init();
  gecko_bgapi_class_mesh_proxy_init();
  gecko_bgapi_class_mesh_proxy_server_init();
  //gecko_bgapi_class_mesh_proxy_client_init();
//  gecko_bgapi_class_mesh_generic_client_init();
  gecko_bgapi_class_mesh_generic_server_init();
  //gecko_bgapi_class_mesh_vendor_model_init();
  //gecko_bgapi_class_mesh_health_client_init();
  //gecko_bgapi_class_mesh_health_server_init();
  //gecko_bgapi_class_mesh_test_init();
  gecko_bgapi_class_mesh_lpn_init();
  //gecko_bgapi_class_mesh_friend_init();
}


void FactoryReset(void)
{
  LOG_INFO("Factory Reset:[TIME]%f\n",Logging());
  displayPrintf(DISPLAY_ROW_ACTION,"Factory Reset");

  if (conn_handle != 0xFF) {
    gecko_cmd_le_connection_close(conn_handle);
  }

  gecko_cmd_flash_ps_erase_all();
  gecko_cmd_hardware_set_soft_timer(RESET_TIME, factoryID, 1);
}


void set_device_name(bd_addr *DeviceAddress)
{

 sprintf(StringToDisplay, "Subscriber: %x:%x", DeviceAddress->addr[1], DeviceAddress->addr[0]);
 displayPrintf(DISPLAY_ROW_NAME,"Subscriber");
 displayPrintf(DISPLAY_ROW_BTADDR2,"5823Sub%02x:%02x",DeviceAddress->addr[1],DeviceAddress->addr[0]);
 resp = gecko_cmd_gatt_server_write_attribute_value(gattdb_device_name, 0, strlen(StringToDisplay), (uint8 *)StringToDisplay)->result;
 if (resp) {
    LOG_INFO("Write attribute failed,[ERROR]%x:[TIME]%f\n",resp,Logging());
 }
 LOG_INFO("Device: '%s':[TIME]%f\n", StringToDisplay,Logging());
}

void lpn_init(void)
{
    struct gecko_msg_mesh_lpn_init_rsp_t *pLPNInit = gecko_cmd_mesh_lpn_init();//friend_node_init();
    if(pLPNInit->result)
    LOG_INFO("Error Init LPN node\n");
    else
    LOG_INFO("Init LPN node Success\n");

    struct gecko_msg_mesh_lpn_configure_rsp_t *pLPNConfig = gecko_cmd_mesh_lpn_configure(2,5000);
    if(pLPNConfig->result)
    LOG_INFO("Error configuring LPN node\n");
    else
    LOG_INFO("configuring LPN node Success\n");
    struct gecko_msg_mesh_lpn_establish_friendship_rsp_t *pLPNest=gecko_cmd_mesh_lpn_establish_friendship(0);
    if(pLPNest->result)
    LOG_INFO("Finding friend command error\n");
    else
    LOG_INFO(" LPN node Success\n");
}

void gecko_main_init()
{

  initMcu();

  initBoard();

  initApp();

  initDefines();

  EnableGPIOInterrupts();

  initADC();

  SmokeInit();

  linklayer_priorities.scan_max = linklayer_priorities.adv_min + 1;

  gecko_stack_init(&config);

  gecko_bgapi_classes_init_client_lpn();
  gecko_initCoexHAL();


}

void handle_gecko_event(uint32_t evt_id, struct gecko_cmd_packet *evt)
{

    struct gecko_msg_mesh_node_provisioning_failed_evt_t  *FailedProv;

  switch (evt_id) {
/***************************************************************************
  @brief: Performs factory reset if either button is held down with reset.
          If read from Flash fails, persistent data is reset. Mesh node init
          is called.
***************************************************************************/
    case gecko_evt_system_boot_id:
      if(!(GPIO_PinInGet(PortB1,PinB1))|| !(GPIO_PinInGet(PortB0,PinB0)))
      {
            FactoryReset();
      }
      else
      {
      		  struct gecko_msg_flash_ps_load_rsp_t *response_1=gecko_cmd_flash_ps_load(0x4000);
      		  if(response_1->result)
      		  {
                override_count=0;
                struct gecko_msg_flash_ps_save_rsp_t *response = gecko_cmd_flash_ps_save(0x4000,1,&override_count);
                if(response->result)
                {
                    LOG_INFO("Error:%d",response->result);
                }
      		  }
      		  else
      		  {
      			  override_count=response_1->value.data[0];
      		  }
      		  LOG_INFO("System Boot ID\n");
      		  struct gecko_msg_system_get_bt_address_rsp_t *Device_Addr_Struct = gecko_cmd_system_get_bt_address();
      		  set_device_name(&Device_Addr_Struct->address);
      		  resp = gecko_cmd_mesh_node_init()->result;

      		  if(resp != 0)
      			  LOG_INFO("Error: 0x%x:[TIME]%f\n", resp,Logging());
      		  else
      			  LOG_INFO("Node Init");
       }
      break;

/***************************************************************************
  @brief: 1. factoryID        =>  Factory reset.
          2. debounceID       =>  Enables GPIO interrupts.
          3. restartID        =>  Factory reset.
          4. provisionID      =>  Prints operation started after provisioning.
          5. FriendNotFoundID => Re-initializes Low Power Node to search for
                                 a friend.
***************************************************************************/
  case gecko_evt_hardware_soft_timer_id:

        switch(evt->data.evt_hardware_soft_timer.handle)
        {
          case factoryID:
            gecko_cmd_system_reset(0);
            break;

          case debounceID:
            NVIC_EnableIRQ(GPIO_ODD_IRQn);
            NVIC_EnableIRQ(GPIO_EVEN_IRQn);
            break;

          case restartID:
            gecko_cmd_system_reset(0);
            break;

          case  provisionID:
            LOG_INFO("\n\r________________OPERATION-START________________\n:[TIME]%f\n\r",Logging());
            break;
          case FriendNotFoundID:
            LOG_INFO("retrying\n");
            lpn_init();
            break;

          default:
            break;
        }
        break;


/***************************************************************************
  @brief: Sets up LPN as a server. Initiates node as LPN if provisioned. If
          not, starts beaconing.
***************************************************************************/
case gecko_evt_mesh_node_initialized_id:
	LOG_INFO("node initialized:[TIME]%f\n",Logging());
	resp = gecko_cmd_mesh_generic_server_init()->result;
    if(resp)
    {
      LOG_INFO("Client Initialization Failed:[TIME]%f\n",Logging());
      LOG_INFO("[ERROR]CODE: %x:[TIME]%f\n", resp,Logging());
    }

        struct gecko_msg_mesh_node_initialized_evt_t *PtrData = (struct gecko_msg_mesh_node_initialized_evt_t *)&(evt->data);
        if(PtrData->provisioned)
        {
          LOG_INFO("Node Provisioned with Address:%x, ivi:%ld:[TIME]%f\n", PtrData->address, PtrData->ivi,Logging());
          displayPrintf(DISPLAY_ROW_CONNECTION,"Provisioned");
          primary_address = PtrData->address;
          ElementID = 0;
          mesh_lib_init(malloc, free, 8);
          lpn_init();
        }

        else
        {
          LOG_INFO("Node Unprovisioned:[TIME]%f\n",Logging());
          LOG_INFO("Unprovisioned Beaconing Started...:[TIME]%f\n",Logging());
          gecko_cmd_mesh_node_start_unprov_beaconing(0x3);
        }
        break;


/***************************************************************************
  @brief: Called when provisioning starts.
***************************************************************************/
case gecko_evt_mesh_node_provisioning_started_id:
        LOG_INFO("Provisioning Started:[TIME]%f\n",Logging());
        displayPrintf(DISPLAY_ROW_CONNECTION,"Provisioning");
        break;


/***************************************************************************
  @brief: Called when friendship is established.
***************************************************************************/
case gecko_evt_mesh_friend_friendship_established_id:
    LOG_INFO("FRIENDSHIP ESTABLISHED\n");
    break;


/***************************************************************************
  @brief: Called when friendship establishment fails.
***************************************************************************/
case gecko_evt_mesh_lpn_friendship_failed_id:
      LOG_INFO("FRIENDSHIP FAILED\n");
      gecko_cmd_hardware_set_soft_timer(RESET_TIME,FriendNotFoundID,1);
  break;


/***************************************************************************
  @brief: Called when friendship is terminated. Searches for friend again.
***************************************************************************/
case gecko_evt_mesh_lpn_friendship_terminated_id:
	LOG_INFO("FRIENDSHIP TERMINATED");
	displayPrintf(DISPLAY_ROW_CONNECTION,"Terminated");
	uint16_t reason;
	reason = evt->data.evt_mesh_lpn_friendship_terminated.reason;
	LOG_INFO("code:%x",reason);
	gecko_cmd_hardware_set_soft_timer(RESET_TIME,FriendNotFoundID,1);
	break;


/***************************************************************************
  @brief: Called when friendship is established.
***************************************************************************/
case gecko_evt_mesh_lpn_friendship_established_id:
  LOG_INFO("FRIENDSHIP ESTABLISHED-LPN\n");
  displayPrintf(DISPLAY_ROW_CONNECTION,"LowPower-Node2");
  break;


/***************************************************************************
  @brief: Called when node is provisioned.
***************************************************************************/
case gecko_evt_mesh_node_provisioned_id:
    LOG_INFO("Node provisioned");
    ElementID = 0;
    mesh_lib_init(malloc, free, 8);
    LOG_INFO("Node provisioned with Address:%x:[TIME]%f\n", evt->data.evt_mesh_node_provisioned.address,Logging());
    displayPrintf(DISPLAY_ROW_CONNECTION,"Provisioned");
    lpn_init();
    break;


/***************************************************************************
  @brief: Called when provisioning fails.
***************************************************************************/
case gecko_evt_mesh_node_provisioning_failed_id:
    LOG_INFO("Provisioning failed");
    FailedProv = (struct gecko_msg_mesh_node_provisioning_failed_evt_t  *)&(evt->data);
    LOG_INFO("provisioning failed, code %x:[TIME]%f\n", FailedProv->result,Logging());
    displayPrintf(DISPLAY_ROW_CONNECTION,"Provision Fail");
    gecko_cmd_hardware_set_soft_timer(RESET_TIME, restartID, 1);
    break;


/***************************************************************************
  @brief: Called when connection is opened.
***************************************************************************/
case gecko_evt_le_connection_opened_id:
      LOG_INFO("Connection_Opened_ID:[TIME]%f\n",Logging());
      NumOfConn++;
      conn_handle = evt->data.evt_le_connection_opened.connection;
      break;


/***************************************************************************
  @brief: Connection paramters established.
***************************************************************************/
case gecko_evt_le_connection_parameters_id:
      LOG_INFO("Connection_Parameters_ID:[TIME]%f\n",Logging());
      break;


/***************************************************************************
  @brief: Called when server receives a message from client.
***************************************************************************/
case gecko_evt_mesh_generic_server_client_request_id:

        LOG_INFO("Server Client Request id");
	      int16_t  Variables = 0;
	      Variables = evt->data.evt_mesh_generic_server_client_request.parameters.data[1]<<8;
	      Variables |= evt->data.evt_mesh_generic_server_client_request.parameters.data[0];

	  	  if(Variables == OVERRIDE)
	  	  {
    				displayPrintf(DISPLAY_ROW_ACTION, "Override");
    				LOG_INFO("Server_Client_Request_ID:[TIME]%f\n",Logging());
    				LOG_INFO("Override Detected\n");
    				CORE_DECLARE_IRQ_STATE;
    				CORE_ENTER_CRITICAL();
    					 OverrideFlag=true;
    					 override_count+=1;
    					 gecko_cmd_flash_ps_save(0x4000,1,&override_count);
    				CORE_EXIT_CRITICAL();

	  	  }
	      break;


/***************************************************************************
  @brief: Called when connection is closed.
***************************************************************************/
case gecko_evt_le_connection_closed_id:
      if (BootID)
        gecko_cmd_system_reset(2);
      break;


/***************************************************************************
  @brief: Checks for updating display. Also checks ADC value by calling
          smokeread(). If ADC value is > threshold, "SMOKE ALERT" is
          published. If an override has been received, the "SMOKE ALERT" is
          not sent for the next 20s. After 20s if the smoke has receded below
          threshold, "SMOKE ALERT" will not be sent.
***************************************************************************/
case gecko_evt_system_external_signal_id:

      DetectEvent = evt->data.evt_system_external_signal.extsignals;
      if(DetectEvent & update_display)
      {
        CORE_DECLARE_IRQ_STATE;
        CORE_ENTER_CRITICAL();
        mask &= ~update_display;
        CORE_EXIT_CRITICAL();
        displayUpdate();
		static struct gecko_msg_flash_ps_save_rsp_t *save_resp;
    static struct gecko_msg_flash_ps_load_rsp_t *load_resp;
    //PINON
  	load_resp=gecko_cmd_flash_ps_load(0x4000);
  	if(load_resp->result==0)
  		LOG_INFO("Override Counts:%d",load_resp->value.data[0]);
  	else
  		LOG_INFO("Error loading\n");
  	clearDisplay();
		if(OverrideFlag==false)
		{		//ONNNNNNNNNNNN
			//GPIO_PinOutClear(gpioPortD,12);
  				SmokeRead();
  				if(WarnFlag==true)
  				{

      					WarnFlag=false;
      					LOG_INFO("SMOKE ALERT!");
      					displayPrintf(DISPLAY_ROW_ACTION,"SMOKE ALERT!");
      					struct mesh_generic_state current;
      					errorcode_t response_update, response_publish;
      					current.kind = mesh_generic_state_level;
      					current.level.level = SMOKE;

      					response_update = mesh_lib_generic_server_update(MESH_GENERIC_LEVEL_SERVER_MODEL_ID,0,&current,NULL,0);

      					if (response_update)
                     LOG_INFO("gecko_cmd_mesh_generic_client_publish failed,code %d:TimeStamp=>%f\n", response_update,Logging());
      					else
      					{
          					 LOG_INFO("Server Updated:TimeStamp=>%f\n",Logging());
          					 response_publish=mesh_lib_generic_server_publish(MESH_GENERIC_LEVEL_SERVER_MODEL_ID,
          							 0, mesh_generic_state_level);
          					 if(response_publish)
          						 LOG_INFO("Not pubished,code:%x,Time:%f\n",response_publish,Logging());
          					 else
          						 LOG_INFO("Server Published:%f\n",Logging());
      					}
  				}
		}


      if(OverrideFlag==true)
      {
        	if(OverrideCounts%2==0)
  		    {
          		OverrideCounts=1;
          		OverrideFlag=false;
          		SmokeRead();
            			if(WarnFlag==true)
            			{
                				WarnFlag=false;
                				LOG_INFO("SMOKE ALERT!");
                				displayPrintf(DISPLAY_ROW_ACTION,"SMOKE ALERT!");
                				struct mesh_generic_state current;
                				errorcode_t response_update, response_publish;
                				current.kind = mesh_generic_state_level;
                				current.level.level = SMOKE;

                				response_update = mesh_lib_generic_server_update(MESH_GENERIC_LEVEL_SERVER_MODEL_ID,0,&current,NULL,0);

                				if (response_update)
                              LOG_INFO("gecko_cmd_mesh_generic_client_publish failed,code %d:TimeStamp=>%f\n", response_update,Logging());


                				else
                				{
                    				 LOG_INFO("Server Updated:TimeStamp=>%f\n",Logging());
                    				 response_publish=mesh_lib_generic_server_publish(MESH_GENERIC_LEVEL_SERVER_MODEL_ID,
                    						 0, mesh_generic_state_level);
                    				 if(response_publish)
                    					 LOG_INFO("Not pubished,code:%x,Time:%f\n",response_publish,Logging());
                    				 else
                    					 LOG_INFO("Server Published:%f\n",Logging());
                				}
            			}
        		}
        	else
        	{
        			displayPrintf(DISPLAY_ROW_ACTION,"Disabled");
        			OverrideCounts+=1;
        	}
      }

			displayPrintf(DISPLAY_ROW_ACTION,"Normal");
		}

    break;

  default:
      break;

  }

}

/***************************************************************************
  @brief: Initializes variables.
***************************************************************************/
void initDefines(void)
{
    ElementID = 0xffff;
    appkeyindex = 0;
    transaction_id = 0;
    conn_handle = 0xFF;
    StringToDisplay[20];
    primary_address = 0;
    NumOfConn = 0;
    OverrideCounts=1;
}

/***************************************************************************
  @brief: Function to enable GPIO interrupts.
***************************************************************************/
void EnableGPIOInterrupts(void)
{
    NVIC_EnableIRQ(GPIO_EVEN_IRQn);
    NVIC_EnableIRQ(GPIO_ODD_IRQn);
}
