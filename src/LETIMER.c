/* Name :- Raj Lavingia
Credits : Dan Walkes
Date :- 3/13/19
*/

/*Include Header File*/

#include "LETIMER.h"
#include "button.h"

const LETIMER_Init_TypeDef letimer =
 {
 		.enable=true,
 		.debugRun=false,
 		.comp0Top=true,
 		.bufTop=false,
 		.out0Pol=false,
 		.out1Pol=false,
		.ufoa0= false,         /* PWM output on output 0 */
		.ufoa1= false,       /* PWM output on output 1*/
 		.repMode= letimerRepeatFree
  };

//Clock Initialisation function
void intialization(void)
{

	  CMU_OscillatorEnable(frequency_1,true,true);
	  CMU_ClockSelectSet(cmuClock_LFA,selection);
	  CMU_ClockEnable(cmuClock_CORELE, true);
	  CMU_ClockEnable(cmuClock_LETIMER0, true);
	  CMU_ClockDivSet(cmuClock_LETIMER0,divide);
	  CMU_ClockEnable(cmuClock_GPIO, true);

	  LETIMER_Enable(LETIMER0,false);
	  LETIMER_Init(LETIMER0,&letimer);
	  LETIMER_CompareSet(LETIMER0,0,(((CMU_ClockFreqGet(cmuClock_LFA)/(divide*1))*(TOTAL_P))));
	  LETIMER_IntSet(LETIMER0,LETIMER_IFC_COMP0);
	  NVIC_EnableIRQ(LETIMER0_IRQn);

	  LETIMER_IntEnable(LETIMER0,LETIMER_IFC_COMP0);
	  LETIMER_Enable(LETIMER0,true);

 }


void LETIMER0_IRQHandler(void)
{
	if(LETIMER_IntGet(LETIMER0)&LETIMER_IFC_COMP0)
	{
		LETIMER_IntClear(LETIMER0,_LETIMER_IF_COMP0_MASK);
		LETIMER_CompareSet(LETIMER0,0,(((CMU_ClockFreqGet(cmuClock_LFA)/(divide*1))*(TOTAL_P))));
		event = On;
		mask |= update_display;
		gecko_external_signal(mask);
		++roll;
		//Check if Server or client , if 1 then server options
		#if (DEVICE_IS_BLE_SERVER==1)
		if(roll%3 == 0)
		{
			mask |= Para_Passed;
			gecko_external_signal(mask); //call the gecko function
		}
		#endif
		//1st if and endif is finished here w
	}
	else
		{
			LOG_INFO("\n 80 ms complete\n");
			LETIMER_IntClear(LETIMER0,LETIMER_IF_COMP1);
			event = InitWrite;
			mask |= Para_Passed;
			gecko_external_signal(mask);
		}
}



uint8_t Status_Return(uint8_t connection)
{
    if (connProperties.connectionHandle != connection)
    return STATUS_INVALID;
    return 1;
}

void AddConn(uint8_t connection, uint16_t address)
{
  connProperties.connectionHandle = connection;
  connProperties.serverAddress    = address;
}

void Event_Handler(void)
{
	if(event == On)
		{
		LOG_INFO("Event on:\t");

		I2C_Begin();


		}
	else if(event == InitWrite)
		{
		LOG_INFO("initwrite event:\t");

			I2C_Write();
		//	EMU_EnterEM1();
		}
	else if(event == I2CComplete)

		{

		if(Write_Read==Write)

			  	{
					LOG_INFO("Time:%d\t",loggerGetTimestamp(roll));

					State_Write();
			  	}
			  	else if(Write_Read==Read)
			  	{
					LOG_INFO("Time:%d\t",loggerGetTimestamp(roll));

			  		State_Read();
			  		event=SleepDeep;
			  	}
		}

		else if(event == Idle)
		{
			LOG_INFO("Time:%d\t",loggerGetTimestamp(roll));
		}
		else if(event == In_progress)
		{
			LOG_INFO("Time:%d\t",loggerGetTimestamp(roll));

		}

		else if(event == SleepDeep)
		{
			LOG_INFO("sleepdeep\t");

		}
		else if(event == Error)
		{
			LOG_INFO("Time:%d\t",loggerGetTimestamp(roll));

			LOG_INFO("\nEntered in the error state\n");

		}

	}


void timerEnable1HzSchedulerEvent(uint32_t Display)
{
	update_display = Display;
}
void check_status()
{
		LOG_INFO("\nEntered in check status function \n");
		temperory_variable = GPIO_PinInGet(gpioPortF,6);
		temperory_variable^=1;
		//check for temp variable value it will toggle between 0 and 1 , if 0 then button released , if 1 then button pressed
		//Here take care of debounce issue, press button for a little bit longer time in order to see the value change

		if(GPIO_Return_Key == Button_Port_Mask)
		{
			button_return_value = true;
		}

}

void GPIO_EVEN_IRQHandler(void)
{
	CORE_AtomicDisableIrq();
	GPIO_Return_Key = GPIO_IntGet();
	GPIO_IntClear(GPIO_Return_Key);
	check_status();
	mask |= Button_Press_Pin;
	gecko_external_signal(mask);
	CORE_AtomicEnableIrq();
}
void gecko_custom_update(struct gecko_cmd_packet* evt)
{
	gecko_update(evt);
	switch(BGLIB_MSG_ID(evt->header))
	{

		case gecko_evt_system_boot_id:
#if (DEVICE_IS_BLE_SERVER==0) //check for server and client
      LOG_INFO("System Initiated\n");
        gecko_cmd_le_gap_set_discovery_type(le_gap_phy_1m, PASSIVE);
        gecko_cmd_le_gap_set_discovery_timing(le_gap_phy_1m, INTERVAL, WINDOW);
        //display on lcd on second row ble address of another board
        gecko_cmd_le_gap_start_discovery(le_gap_phy_1m, le_gap_discover_generic);
        displayPrintf(DISPLAY_ROW_CONNECTION,"Scanning");
        displayPrintf(DISPLAY_ROW_NAME,"Client");
        displayPrintf(DISPLAY_ROW_BTADDR2,"%02x:%02x:%02x:%02x:%02x:%02x",ServerAddress.addr[5],ServerAddress.addr[4],
		ServerAddress.addr[3],ServerAddress.addr[2],
		ServerAddress.addr[1],ServerAddress.addr[0]);
        //display on lcd on first row ble address of  board
        //Reference taken from demo example
        Addressptr = gecko_cmd_system_get_bt_address();
		displayPrintf(DISPLAY_ROW_BTADDR,"%02x:%02x:%02x:%02x:%02x:%02x",Addressptr->address.addr[5],
		Addressptr->address.addr[4],Addressptr->address.addr[3],
		Addressptr->address.addr[2],Addressptr->address.addr[1],
		Addressptr->address.addr[0]);
        connState = scanning;
//server code in else loop
        #else
        gecko_cmd_sm_delete_bondings();
		LOG_INFO("\r\nBLE Central started\r\n");
		//random passkeys are generated
		gecko_cmd_sm_set_passkey(-1);
		//0x0B (1101) for conditions check
		gecko_cmd_sm_set_bondable_mode(1);
		gecko_cmd_sm_configure(0x07, sm_io_capability_displayyesno);
		displayPrintf(DISPLAY_ROW_CONNECTION,"Advertising");
		displayPrintf(DISPLAY_ROW_NAME,"Server");
		Addressptr = gecko_cmd_system_get_bt_address();
		displayPrintf(DISPLAY_ROW_BTADDR,"%02x:%02x:%02x:%02x:%02x:%02x",Addressptr->address.addr[5],
		Addressptr->address.addr[4],
		Addressptr->address.addr[3],
		Addressptr->address.addr[2],
		Addressptr->address.addr[1],
		Addressptr->address.addr[0]);

		gecko_cmd_system_set_tx_power(0);
		gecko_cmd_le_gap_set_advertise_timing(0, 400, 400, 0, 0);
		gecko_cmd_le_gap_start_advertising(0, le_gap_general_discoverable, le_gap_connectable_scannable);
		#endif
		break;

case gecko_evt_le_gap_scan_response_id:

			if (evt->data.evt_le_gap_scan_response.packet_type == 0)
			{
				LOG_INFO("\n Scanning is on\n");
				if(memcmp(&evt->data.evt_le_gap_scan_response.address.addr[0], 	&ServerAddress.addr[0], 6) == 0)
				{//check for all 8*6 = 48 bits on both the ends for proper verification
				LOG_INFO("\n Scanning is on part 2\n");
				gecko_cmd_le_gap_end_procedure(); //interval
				gecko_cmd_le_gap_connect(evt->data.evt_le_gap_scan_response.address,
														evt->data.evt_le_gap_scan_response.address_type,
														le_gap_phy_1m);
				connState = opening; //if everything is successful then open the port for communication

				}
				else
				{
					//error sttae
					LOG_INFO("\n Scanning is not successfully started\n");
				}
			}
			break;

case gecko_evt_sm_confirm_passkey_id:
	    	{
				LOG_INFO("\n Passkey id confirmation\n");
	    		Passkey_Status_Check = true; //check if status is true or false
	    		passphrase_key = evt->data.evt_sm_confirm_passkey.passkey;
	    		displayPrintf(DISPLAY_ROW_PASSKEY,"passkey- %d",passphrase_key);
	    		displayPrintf(DISPLAY_ROW_ACTION,"Confirm With PB0");
	    		break;
	    	}

case gecko_evt_sm_bonded_id:
	    	{
				LOG_INFO("\n In bonded ID loop\n");
	    		displayPrintf(DISPLAY_ROW_PASSKEY,"Bonded Successfully");
	    		displayPrintf(DISPLAY_ROW_ACTION,"Connected");
	    		Passkey_Bonding_Status_Fail = false; //Bonding status fail so that go into next function
	    		break;
	    	}

	    	case gecko_evt_sm_bonding_failed_id:
	    	{
				LOG_INFO("\n Bodninf failed loop\n");

	    		displayPrintf(DISPLAY_ROW_ACTION,"Bonding Failed");
	    		Passkey_Bonding_Status_Fail = true; //Bonding status true
	    		gecko_cmd_le_connection_close(Open_Close_Connection);
	    		break;
	    	}

case gecko_evt_le_connection_opened_id:
			#if(DEVICE_IS_BLE_SERVER==0) //after opening a connection
	        addrValue = (uint16_t)(evt->data.evt_le_connection_opened.address.addr[1] << 8)+ evt->data.evt_le_connection_opened.address.addr[0];
	        AddConn(evt->data.evt_le_connection_opened.connection, addrValue); //add connection, shift bits
	        gecko_cmd_gatt_discover_primary_services_by_uuid(evt->data.evt_le_connection_opened.connection,2,(const uint8_t*)SERVICE);
	        connState = discoverServices; //discovreing state
        #else
		Open_Close_Connection = evt->data.evt_le_connection_opened.connection;
		gecko_cmd_le_connection_set_parameters(Open_Close_Connection, 60, 60,3,300);
		LETIMER_Enable(LETIMER0,true);
		displayPrintf(DISPLAY_ROW_CONNECTION,"Connected");
		displayPrintf(DISPLAY_ROW_NAME,"Server");
		LOG_INFO("\nSuccessful Opened\n");
		Connection_Established_Server_Success = evt->data.evt_le_connection_opened.bonding;
   		if(Connection_Established_Server_Success != Connection_Not_Established_Server)
    		{
				LOG_INFO("\n Check for server access if connected or not\n");
    			displayPrintf(DISPLAY_ROW_ACTION,"Already Bonded");
    			displayPrintf(DISPLAY_ROW_PASSKEY,"Connected");
    		}
   		else
    		{
			LOG_INFO("\n Check for server access if connected or not part 2\n");

    		}
		#endif
		break;
//reference code taken from demo code given all the functions below are taken from that
case gecko_evt_gatt_service_id:
			statusRet = Status_Return(evt->data.evt_gatt_service.connection);
			if (statusRet != STATUS_INVALID) {
			connProperties.thermometerServiceHandle = evt->data.evt_gatt_service.service;
			}
			break;

case gecko_evt_gatt_characteristic_id:
			statusRet = Status_Return(evt->data.evt_gatt_characteristic.connection);
			if (statusRet != STATUS_INVALID) {
			connProperties.thermometerCharacteristicHandle = evt->data.evt_gatt_characteristic.characteristic;
			}
			break;

 case gecko_evt_gatt_characteristic_value_id:
			displayPrintf(DISPLAY_ROW_CONNECTION,"Handling Indications");
               displayPrintf(DISPLAY_ROW_BTADDR2,"%02x:%02x:%02x:%02x:%02x:%02x",ServerAddress.addr[5],	ServerAddress.addr[4],
				ServerAddress.addr[3],ServerAddress.addr[2],ServerAddress.addr[1],ServerAddress.addr[0]);
			charValue = &(evt->data.evt_gatt_characteristic_value.value.data[0]);
			statusRet = Status_Return(evt->data.evt_gatt_characteristic_value.connection);
			if (statusRet != STATUS_INVALID)
			{
			connProperties.temperature = (charValue[1] << 0) + (charValue[2] << 8) + (charValue[3] << 16);
			}
			gecko_cmd_gatt_send_characteristic_confirmation(evt->data.evt_gatt_characteristic_value.connection);
			gecko_cmd_le_connection_get_rssi(evt->data.evt_gatt_characteristic_value.connection);
			break;

case gecko_evt_gatt_procedure_completed_id:
			statusRet = Status_Return(evt->data.evt_gatt_procedure_completed.connection);
			if (statusRet == STATUS_INVALID) {
			break;
			}
			if (connState == discoverServices && connProperties.thermometerServiceHandle != SERVICE_HANDLE_INVALID) {
			gecko_cmd_gatt_discover_characteristics_by_uuid(evt->data.evt_gatt_procedure_completed.connection,
			                                  connProperties.thermometerServiceHandle,
			                                  2,
			                                  (const uint8_t*)CHARACTERS);
			connState = discoverCharacteristics;
			break;
			}
			if (connState == discoverCharacteristics && connProperties.thermometerCharacteristicHandle != CHARACTERISTIC_HANDLE_INVALID) {
			LOG_INFO("characteristic discovery finished\n");
			gecko_cmd_le_gap_end_procedure();
			gecko_cmd_gatt_set_characteristic_notification(evt->data.evt_gatt_procedure_completed.connection,
			                                 connProperties.thermometerCharacteristicHandle,
			                                 gatt_indication);
			connState = enableIndication;
			break;
			}
			if (connState == enableIndication) {
			connState = running;
			}
			break;


case gecko_evt_le_connection_closed_id:
		#if(DEVICE_IS_BLE_SERVER==0)
        LOG_INFO("connection closed ID\n"); //allindividual addresses are mentioned 8*6 = 48 bits
        Addressptr = gecko_cmd_system_get_bt_address();
		displayPrintf(DISPLAY_ROW_BTADDR,"%02x:%02x:%02x:%02x:%02x:%02x",Addressptr->address.addr[5],
		Addressptr->address.addr[4],Addressptr->address.addr[3],Addressptr->address.addr[2],Addressptr->address.addr[1],Addressptr->address.addr[0]);
		displayPrintf(DISPLAY_ROW_CONNECTION,"Discovery\n");
        displayPrintf(DISPLAY_ROW_BTADDR2,"%02x:%02x:%02x:%02x:%02x:%02x",ServerAddress.addr[5],
		ServerAddress.addr[4],ServerAddress.addr[3],ServerAddress.addr[2],ServerAddress.addr[1],ServerAddress.addr[0]);
		displayPrintf(DISPLAY_ROW_NAME,"Client");
		displayPrintf(DISPLAY_ROW_TEMPVALUE,"%s"," ");
		displayPrintf(DISPLAY_ROW_TEMPVALUE,"%s"," ");
		displayPrintf(DISPLAY_ROW_TEMPVALUE,"%s"," ");

        gecko_cmd_le_gap_start_discovery(le_gap_phy_1m, le_gap_discover_generic);
        connState = scanning;
        #else
		Connection_Established = 0;
		gecko_cmd_le_gap_start_advertising(0, le_gap_general_discoverable, le_gap_connectable_scannable);
		displayPrintf(DISPLAY_ROW_CONNECTION,"Advertising");
		displayPrintf(DISPLAY_ROW_NAME,"Server");
		displayPrintf(DISPLAY_ROW_TEMPVALUE,"%s"," ");
		LOG_INFO("\nSuccessful Closed\n");
		LETIMER_Enable(LETIMER0,false);
		#endif
		break;

case gecko_evt_gatt_server_characteristic_status_id:

		displayPrintf(DISPLAY_ROW_CONNECTION,"Handling Indications");
		if((evt->data.evt_gatt_server_characteristic_status.characteristic == gattdb_Temperature)
	    &&(evt->data.evt_gatt_server_characteristic_status.status_flags == 0x01))
			{
				LOG_INFO("\nStatus id loop\n\r");
				Connection_Established = 1;
			}
	      break;


case gecko_evt_system_external_signal_id:
Event_Status_Retun_Back = evt->data.evt_system_external_signal.extsignals;
#if (DEVICE_IS_BLE_SERVER==1)
if(Event_Status_Retun_Back & Button_Press_Pin)
					{
						LOG_INFO("\nButton pressed interrupt loop\n\r");
						pointer = &temperory_variable;
						/* Begin Critical Section */
						CORE_DECLARE_IRQ_STATE;
						CORE_ENTER_CRITICAL();
						mask &= ~Button_Press_Pin;				//Clear the Event Mask
						CORE_EXIT_CRITICAL();
						/* End Critical Section */
						displayUpdate();
						gecko_cmd_sm_passkey_confirm(Open_Close_Connection, true);
						displayPrintf(DISPLAY_ROW_ACTION,"PassKey Accepted");
						button_return_temp_status = false;
					    gecko_cmd_gatt_server_send_characteristic_notification(0xFF, gattdb_Button_State, 1, pointer);
					    //1 is the size defined
					}

	    LOG_INFO("Clock Frequency:%d",CMU_ClockFreqGet(cmuClock_LFA));
	    if(Connection_Established == 1)
		    {

			    if(Event_Status_Retun_Back & Para_Passed )
			    	{
			    	  CORE_DECLARE_IRQ_STATE;
			    	  CORE_ENTER_CRITICAL();
			    	  mask &= ~Para_Passed ;
			    	  CORE_EXIT_CRITICAL();
			    	  LOG_INFO("Para passed loop\n");
			    	  Event_Handler();

			    	}
			    if(Event_Status_Retun_Back & mask_I2C )
			    {
			    	LOG_INFO("\nmask i2c loop\n");
			    	CORE_DECLARE_IRQ_STATE;
			    	CORE_ENTER_CRITICAL();
			        mask &= ~mask_I2C ;
			        CORE_EXIT_CRITICAL();
			        Event_Handler();
			        if(flag==1)
			         		{
			        			Conversion_BLE();
			           			LETIMER_Enable(LETIMER0, true);

			           		}
			        else if(flag == 2)
			        {
			        	break;
			        }
			    }

			    if(Event_Status_Retun_Back & update_display)
			    {
			    	CORE_DECLARE_IRQ_STATE;
			    	CORE_ENTER_CRITICAL();
			    	mask &= ~update_display;
					CORE_EXIT_CRITICAL();
					displayUpdate();
			    }
			    else
			    {
			    	LOG_INFO("In the else loop of event handler\n");
			    	Event_Handler();
			    }
			    }
	    #else

			    if (Event_Status_Retun_Back & EXT_SIGNAL_PRINT_RESULTS) {

						CORE_DECLARE_IRQ_STATE;
			    		CORE_ENTER_CRITICAL();
			    		mask &= ~EXT_SIGNAL_PRINT_RESULTS;
			    		CORE_EXIT_CRITICAL();
						if (true == HEADER)
						{
						HEADER = false;
						}
						if ((TEMP_INVALID != connProperties.temperature)) {
						displayPrintf(DISPLAY_ROW_TEMPVALUE,"%f",(float)connProperties.temperature*0.001);
						LOG_INFO("Temperature:%d",connProperties.temperature);
						}
					}


					 if(Event_Status_Retun_Back & update_display)
			    	{
			    		CORE_DECLARE_IRQ_STATE;
			    		CORE_ENTER_CRITICAL();
			    		mask &= ~update_display;
			    		CORE_EXIT_CRITICAL();
			    		displayUpdate();
			    	}
			    	#endif
			    	break;

case gecko_evt_le_connection_rssi_id:
		 #if (DEVICE_IS_BLE_SERVER==0) //Client
       LOG_INFO("RSSI measured\n");
        statusRet = Status_Return(evt->data.evt_le_connection_rssi.connection);
        if (statusRet != STATUS_INVALID) {
          connProperties.rssi = evt->data.evt_le_connection_rssi.rssi;
          gecko_external_signal(EXT_SIGNAL_PRINT_RESULTS);
        }
       #else
   		rssi_value_owndefined = evt->data.evt_le_connection_rssi.rssi;
   		gecko_cmd_system_halt(1);
									if(rssi_value_owndefined > -35)
									  {
										gecko_cmd_system_set_tx_power(-260);
										LOG_INFO(" > -35\n");
									  }

			    		    		else if(-35 >= rssi_value_owndefined && rssi_value_owndefined > -45)
			    		    		{
			    		    			gecko_cmd_system_set_tx_power(-200);
			    		    			LOG_INFO("  > -45\n");
			    		    		}


			    		    		else if(-45 >= rssi_value_owndefined && rssi_value_owndefined > -55)
			    		    		{
			    		    			gecko_cmd_system_set_tx_power(-150);
			    		    			LOG_INFO("  > -55\n");
			    		    		}

			    		    		else if(-55 >= rssi_value_owndefined && rssi_value_owndefined > -65)
			    		    		{
			    		    			gecko_cmd_system_set_tx_power(-50);
			    		    			LOG_INFO("> -65\n");
			    		    		}

			    		    		else if(-65 >= rssi_value_owndefined && rssi_value_owndefined > -75)
			    		    		{
			    		    			gecko_cmd_system_set_tx_power(0);
			    		    			LOG_INFO(" > -75\n");
			    		    		}


			    		    		else if(-75 >= rssi_value_owndefined && rssi_value_owndefined > -85)
			    		    		{
			    		    			gecko_cmd_system_set_tx_power(50);
			    		    			LOG_INFO("  > -85\n");
			    		    		}


			    		    		else
			    		    		{

			    		    			gecko_cmd_system_set_tx_power(80);
			    		    			LOG_INFO(" largest \n");

			    		    		}

			    		    		gecko_cmd_system_halt(0);
			    		    		break;
			    		  #endif
			    		  break;

	}
}
