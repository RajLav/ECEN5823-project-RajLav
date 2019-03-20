/* Name :- Raj Lavingia
Credits : Dan Walkes
Date :- 3/20/19
*/

/*Include Header File*/

#include "LETIMER.h"
#include "button.h"
char* Button_Status[]={"Button Release","Button Pressed"};

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

void Button_UUID()
{
			buttonSERVICE[0]= 0x89;
			buttonSERVICE[1]=0x62;
			buttonSERVICE[2]=0x13;
			buttonSERVICE[3]=0x2d;
			buttonSERVICE[4]=0x2a;
			buttonSERVICE[5]=0x65;
			buttonSERVICE[6]=0xec;
			buttonSERVICE[7]=0x87;
			buttonSERVICE[8]=0x3e;
			buttonSERVICE[9]=0x43;
			buttonSERVICE[10]=0xc8;
			buttonSERVICE[11]=0x38;
			buttonSERVICE[12]=0x01;
			buttonSERVICE[13]=0x00;
			buttonSERVICE[14]=0x00;
			buttonSERVICE[15]=0x00;

			buttonCHARACTERS[0]= 0x89;
			buttonCHARACTERS[1]=0x62;
			buttonCHARACTERS[2]=0x13;
			buttonCHARACTERS[3]=0x2d;
			buttonCHARACTERS[4]=0x2a;
			buttonCHARACTERS[5]=0x65;
			buttonCHARACTERS[6]=0xec;
			buttonCHARACTERS[7]=0x87;
			buttonCHARACTERS[8]=0x3e;
			buttonCHARACTERS[9]=0x43;
			buttonCHARACTERS[10]=0xc8;
			buttonCHARACTERS[11]=0x38;
			buttonCHARACTERS[12]=0x02;
			buttonCHARACTERS[13]=0x00;
			buttonCHARACTERS[14]=0x00;
			buttonCHARACTERS[15]=0x00;

}

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
	GPIO_Return_Key = GPIO_IntGet();
	GPIO_IntClear(GPIO_Return_Key);
	// check_status();
	LOG_INFO("\nEntered in check status function \n");
	temperory_variable = GPIO_PinInGet(gpioPortF,6);
	temperory_variable^=1;
	mask |= Button_Press_Pin;
	ptr = &temperory_variable;
	Flag_Status_Check = 0x00;
	ptr3 = flag_array_define;
	UINT8_TO_BITSTREAM(ptr3, Flag_Status_Check);
	LOG_INFO("\nSuccessful flag status check\n");
	UINT8_TO_BITSTREAM(ptr3, temperory_variable);
	LOG_INFO("\nSuccessful Temperory Variable\n");
    gecko_cmd_gatt_server_send_characteristic_notification(0xFF, gattdb_Button_State,2,flag_array_define);
	displayPrintf(DISPLAY_ROW_ACTION,"Button:%d",temperory_variable);
	gecko_external_signal(mask);

}
void gecko_custom_update(struct gecko_cmd_packet* evt)
{
	gecko_update(evt);
	switch(BGLIB_MSG_ID(evt->header))
	{
		case gecko_evt_system_boot_id:
		//For client make server as 0
		#if (DEVICE_IS_BLE_SERVER==0) //check for server and client
        LOG_INFO("System Initiated in Client Mode\n");
        uint16_t check1 =  gecko_cmd_le_gap_set_discovery_type(le_gap_phy_1m, PASSIVE);
        if(check1!=0)
       {
    	   LOG_INFO("corrupt 1 \n");
       }
        else
        {
        	LOG_INFO("No Corrupt \n");
        }
       uint16_t check2 = gecko_cmd_le_gap_set_discovery_timing(le_gap_phy_1m, INTERVAL, WINDOW);
       if(check2!=0)
            {
         	   LOG_INFO("corrupt 2 \n");
            }
             else
             {
             	LOG_INFO("No Corrupt \n");
             }

       //display on lcd on second row ble address of another board
        uint16_t check3 = gecko_cmd_le_gap_start_discovery(le_gap_phy_1m, le_gap_discover_generic);
        if(check3!=0)
             {
          	   LOG_INFO("corrupt 3 \n");
             }
              else
              {
              	LOG_INFO("No Corrupt \n");
              }

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
		gecko_cmd_sm_delete_bondings();
		gecko_cmd_sm_set_bondable_mode(1);
		gecko_cmd_sm_configure(0x07, sm_io_capability_displayyesno);
        present_state = scanning;
        //server code in else loop
        #else
        gecko_cmd_sm_delete_bondings();
		LOG_INFO("\r\nBLE Central started as server mode\r\n");
		//random passkeys are generated
		gecko_cmd_sm_set_passkey(-1);
		//0x07 (0111) for conditions check
		gecko_cmd_sm_set_bondable_mode(1);
		uint16_t check4 = gecko_cmd_sm_configure(0x07, sm_io_capability_displayyesno);
		 if(check4!=0)
		             {
		          	   LOG_INFO("corrupt 4 \n");
		             }
		              else
		              {
		              	LOG_INFO("No Corrupt \n");
		              }

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
				LOG_INFO("\n Scanning is on in scan response id function\n");
				if(memcmp(&evt->data.evt_le_gap_scan_response.address.addr[0], 	&ServerAddress.addr[0], 6) == 0)
				{
				//check for all 8*6 = 48 bits on both the ends for proper verification
				LOG_INFO("\n Scanning is on part 2\n");
				gecko_cmd_le_gap_end_procedure(); //interval
				gecko_cmd_le_gap_connect(evt->data.evt_le_gap_scan_response.address,evt->data.evt_le_gap_scan_response.address_type,
														le_gap_phy_1m);
				present_state = opening; //if everything is successful then open the port for communication

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
				LOG_INFO("\n Passkey id confirmation in Confirm passkey id function\n");
	    		Passkey_Status_Check = true; //check if status is true or false
	    		passphrase_key = evt->data.evt_sm_confirm_passkey.passkey;
	    		displayPrintf(DISPLAY_ROW_PASSKEY,"passkey- %d",passphrase_key);
	    		displayPrintf(DISPLAY_ROW_ACTION,"Confirm With PB0");
	    		break;
	    	}

case gecko_evt_sm_bonded_id:

				LOG_INFO("\n In bonded ID loop\n");
	    		displayPrintf(DISPLAY_ROW_PASSKEY,"Bonded Successfully");
	    		displayPrintf(DISPLAY_ROW_ACTION,"Connected");
	    		#if(DEVICE_IS_BLE_SERVER==0)
				ButtonState=1;
				LOG_INFO("Bonding successful for button state\n");
				#endif
	    		break;


case gecko_evt_sm_bonding_failed_id:

				LOG_INFO("\n Bodninf failed loop\n");
				displayPrintf(DISPLAY_ROW_ACTION,"Bonding Failed");
				gecko_cmd_le_connection_close(Open_Close_Connection);
				break;


case gecko_evt_le_connection_opened_id:
			#if(DEVICE_IS_BLE_SERVER==0) //after opening a connection
	        addrValue = (uint16_t)(evt->data.evt_le_connection_opened.address.addr[1] << 8)+ evt->data.evt_le_connection_opened.address.addr[0];
	        AddConn(evt->data.evt_le_connection_opened.connection, addrValue); //add connection, shift bits
	        uint16_t check5 = gecko_cmd_gatt_discover_primary_services_by_uuid(evt->data.evt_le_connection_opened.connection,2,(const uint8_t*)SERVICE);
	        if(check5!=0)
	                    {
	                 	   LOG_INFO("corrupt 5 \n");

	                    }
	                     else
	                     {
	                     	LOG_INFO("No Corrupt \n");
	                     }

	        present_state = discoverServices; //discovreing state
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
			LOG_INFO("Service ID\n");
			if (statusRet != STATUS_INVALID) {
			if(Button_Status_Start_Time==0)
			{
				connProperties.thermometerServiceHandle = evt->data.evt_gatt_service.service;
				LOG_INFO("\nEntered in thermometer handler\n");
			}
			else
			{
				connProperties.buttonServiceHandle = evt->data.evt_gatt_service.service;
				LOG_INFO("\nEntered in the button state\n");
			}
			}
			break;

case gecko_evt_gatt_characteristic_id:
			LOG_INFO("Characteristic ID\n");
			statusRet = Status_Return(evt->data.evt_gatt_characteristic.connection);
			if (statusRet != STATUS_INVALID) {
			if(Button_Status_Start_Time==0)
			{
				LOG_INFO("\nEntered in the button state when 0\n");
				connProperties.thermometerCharacteristicHandle = evt->data.evt_gatt_characteristic.characteristic;
			}
				else
				{
				LOG_INFO("\nEntered in the button state when 1\n");
				connProperties.buttonCharacteristicHandle = evt->data.evt_gatt_characteristic.characteristic;
				}
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
				if(connProperties.thermometerCharacteristicHandle==evt->data.evt_gatt_characteristic_value.characteristic)
				{
					LOG_INFO("\nEntered in the thrmo checking\n");
					//check for temperature on both ends
					connProperties.temperature = (charValue[1] << 0) + (charValue[2] << 8) + (charValue[3] << 16);
				}
				else if(connProperties.buttonCharacteristicHandle==evt->data.evt_gatt_characteristic_value.characteristic)
				{
					LOG_INFO("\nEntered in the button checking \n");
					//check for button state
					connProperties.button = charValue[1];
				}
			}
			gecko_cmd_gatt_send_characteristic_confirmation(evt->data.evt_gatt_characteristic_value.connection);
			gecko_cmd_le_connection_get_rssi(evt->data.evt_gatt_characteristic_value.connection);
			break;

case gecko_evt_gatt_procedure_completed_id:
Open_Close_Connection=evt->data.evt_gatt_procedure_completed.connection;
			statusRet = Status_Return(evt->data.evt_gatt_procedure_completed.connection);
			if (statusRet == STATUS_INVALID)
			{
			break;
			}
			if (present_state == discoverServices)
			{
			//button state first set to 0 and then when it goes into loop change it to 1
			if(Button_Status_Start_Time==0 && connProperties.thermometerServiceHandle != SERVICE_HANDLE_INVALID)
			{
			Button_Status_Start_Time=1; //value changed for else if loop
			uint16_t check6 = gecko_cmd_gatt_discover_primary_services_by_uuid(evt->data.evt_gatt_procedure_completed.connection, 16,(const uint8_t*)buttonSERVICE);
			//16 bytes used because  custom uuid made for button is of 128 bits
			 if(check6!=0)
			             {
			          	   LOG_INFO("corrupt 6 \n");
			             }
			              else
			              {
			              	LOG_INFO("No Corrupt \n");
			              }

			}
			else if(Button_Status_Start_Time==1 && connProperties.buttonServiceHandle != SERVICE_HANDLE_INVALID)
			{
			Button_Status_Start_Time=0;
			uint16_t check7 = gecko_cmd_gatt_discover_characteristics_by_uuid(evt->data.evt_gatt_procedure_completed.connection,connProperties.thermometerServiceHandle,2,(const uint8_t*)CHARACTERS);
			 if(check7!=0)
			             {
			          	   LOG_INFO("corrupt 7 \n");
			          	 }
			              else
			              {
			              	LOG_INFO("No Corrupt \n");
			              }

			//2 bytes because ready made uuid in temperature
			present_state = discoverCharacteristics;			//Update the state only after both the conditions have been met.
			}
			break;
			}

			if (present_state == discoverCharacteristics)
			{
				//same logic as mentioned in service loop
			if(Button_Status_Start_Time==0 && connProperties.thermometerCharacteristicHandle != CHARACTERISTIC_HANDLE_INVALID)
			{
			Button_Status_Start_Time=1;
			uint16_t check8 =gecko_cmd_gatt_discover_characteristics_by_uuid(evt->data.evt_gatt_procedure_completed.connection,connProperties.buttonServiceHandle,16,(const uint8_t*)buttonCHARACTERS);
			if(check8!=0)
						             {
						          	   LOG_INFO("corrupt 8 \n");
						          	 }
						              else
						              {
						              	LOG_INFO("No Corrupt \n");
						              }

			}
			else if(Button_Status_Start_Time == 1 && connProperties.buttonCharacteristicHandle != CHARACTERISTIC_HANDLE_INVALID)
			{
				Button_Status_Start_Time=0;
				uint16_t check9 = gecko_cmd_gatt_set_characteristic_notification(evt->data.evt_gatt_procedure_completed.connection,connProperties.thermometerCharacteristicHandle,gatt_indication);
				if(check9!=0)
							             {
							          	   LOG_INFO("corrupt 9 \n");
							          	 }
							              else
							              {
							              	LOG_INFO("No Corrupt \n");
							              }

				present_state = enableIndication;
			}
			break;
			}

			if (present_state == enableIndication)
			{
				if(Button_Status_Start_Time==0)
				{
					Button_Status_Start_Time=1;
					//Not needed
					gecko_cmd_sm_increase_security(evt->data.evt_gatt_procedure_completed.connection);
					gecko_cmd_gatt_set_characteristic_notification(evt->data.evt_gatt_procedure_completed.connection,connProperties.buttonCharacteristicHandle,gatt_indication);
				}
				else
				{
					Button_Status_Start_Time=0;
					gecko_cmd_sm_increase_security(evt->data.evt_gatt_procedure_completed.connection); ///button passkey here only not for temperature,
					present_state = running;
				}
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
        ButtonState=0;
		displayPrintf(DISPLAY_ROW_NAME,"Client");
		displayPrintf(DISPLAY_ROW_TEMPVALUE,"%s"," ");
		displayPrintf(DISPLAY_ROW_PASSKEY,"%s"," ");
		displayPrintf(DISPLAY_ROW_ACTION,"%s"," ");
        gecko_cmd_le_gap_start_discovery(le_gap_phy_1m, le_gap_discover_generic);
        present_state = scanning;
        #else
		Connection_Established = 0;
		gecko_cmd_le_gap_start_advertising(0, le_gap_general_discoverable, le_gap_connectable_scannable);
		displayPrintf(DISPLAY_ROW_CONNECTION,"Advertising");
		displayPrintf(DISPLAY_ROW_NAME,"Server");
		displayPrintf(DISPLAY_ROW_TEMPVALUE,"%s"," ");
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
				LETIMER_Enable(LETIMER0, true);
	    		LETIMER_IntEnable(LETIMER0,LETIMER_IF_COMP0);
			}
	      break;


case gecko_evt_system_external_signal_id:
Event_Status_Retun_Back = evt->data.evt_system_external_signal.extsignals;
#if (DEVICE_IS_BLE_SERVER==1)
if(Event_Status_Retun_Back & Button_Press_Pin)
					{
						LOG_INFO("\nButton pressed interrupt loop\n\r");

						mask &= ~Button_Press_Pin;				//Clear the Event Mask
						displayUpdate();
						if(Passkey_Status_Check == true)
						{
							Passkey_Status_Check = false;
							gecko_cmd_sm_passkey_confirm(Open_Close_Connection, true);
							displayPrintf(DISPLAY_ROW_ACTION,"PassKey Accepted");
						}
								}

	    LOG_INFO("Clock Frequency:%d",CMU_ClockFreqGet(cmuClock_LFA));
	    if(Connection_Established == 1)
		    {

			    if(Event_Status_Retun_Back & Para_Passed )
			    	{
			    	  mask &= ~Para_Passed ;
			    	  LOG_INFO("Para passed loop\n");
			    	  Event_Handler();
			    	}
			    if(Event_Status_Retun_Back & mask_I2C )
			    {
			    	LOG_INFO("\nmask i2c loop\n");
			        mask &= ~mask_I2C ;
			        Event_Handler();
			        if(flag==1)
			         		{
			        			Conversion_BLE();
			           			LETIMER_Enable(LETIMER0, true);
			           		}
			    }

			    if(Event_Status_Retun_Back & update_display)
			    {
			    	mask &= ~update_display;
					displayUpdate();
			    }
			    else
			    {
			    	LOG_INFO("In the else loop of event handler\n");
			    	Event_Handler();
			    }
			    }
	    #else
	    //client

if(Event_Status_Retun_Back & Button_Press_Pin)
					{
						mask &= ~Button_Press_Pin;				//Clear the Event Mask
						displayUpdate();
						gecko_cmd_sm_passkey_confirm(Open_Close_Connection, true)->result;
					}


			    if (Event_Status_Retun_Back & bit_mask_external_event)
			    	{
						mask &= ~bit_mask_external_event;
						displayPrintf(DISPLAY_ROW_TEMPVALUE,"%f",(float)connProperties.temperature*0.001);
						if(ButtonState==1)
						displayPrintf(DISPLAY_ROW_ACTION,"%s",Button_Status[(uint8_t)connProperties.button]);
					}


					 if(Event_Status_Retun_Back & update_display)
			    	{
						mask &= ~update_display;
			    		if(ButtonState==1)
			    		displayPrintf(DISPLAY_ROW_ACTION,"%s",Button_Status[(uint8_t)connProperties.button]);
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
         gecko_external_signal(bit_mask_external_event);
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
