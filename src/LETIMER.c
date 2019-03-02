/* Name :- Raj Lavingia
Credits : Dan Walkes
*/

/*Include Header File*/

#include "LETIMER.h"


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

void tick_delay(uint32_t us_wait)
{
	uint32_t temp1=LETIMER_CounterGet(LETIMER0);
	uint32_t temp2 = CounterGet(us_wait);
			if(us_wait==0)temp2=1;
			NVIC_DisableIRQ(LETIMER0_IRQn);
			if(temp1>temp2)
			{
				LETIMER_CompareSet(LETIMER0,1,temp2);
			}
			else
			{
				LETIMER_CompareSet(LETIMER0,1,((((CMU_ClockFreqGet(cmuClock_LFA)/(divide*1))*(TOTAL_P)))-temp2-temp1));
			}
			NVIC_EnableIRQ(LETIMER0_IRQn);
			LETIMER_IntSet(LETIMER0,LETIMER_IFC_COMP0|LETIMER_IFC_COMP1);
			LETIMER_IntEnable(LETIMER0,LETIMER_IFC_COMP0|LETIMER_IFC_COMP1);

}

void I2C_Begin(void)
{
	//Disable all interuppts
	  LETIMER_IntDisable(LETIMER0,_LETIMER_IF_COMP0_MASK); //Disable Interrupts
    I2CSPM_Init_TypeDef* I2CSPM_ptr;
	//For I2c transfer SDA and SCL pins defined
    I2CSPM_ptr->sclPin=10;
	I2CSPM_ptr->sdaPin=11;
	I2CSPM_ptr->portLocationScl=14;
	I2CSPM_ptr->portLocationScl=16;
	I2CSPM_Init(I2CSPM_ptr);
	//On->1
    GPIO_PinModeSet(gpioPortD, 15, gpioModePushPull, 1);	//Enable the I2C module
    tick_delay(80000);
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
		if(roll%3 == 0)
		{
			mask |= Para_Passed;
			gecko_external_signal(mask); //call the gecko function
		}
		//flag = 0;
		mask |= Para_Passed;
		gecko_external_signal(mask); //call the gecko function
	}
	else
		{
			LOG_INFO("\n 80 ms complete\n");
			LETIMER_IntClear(LETIMER0,LETIMER_IF_COMP1);
			event = InitWrite;
			gecko_external_signal(Para_Passed);

		}

}

void I2C_Write(void)
{
	LOG_INFO("\nIn the I2c_Write function\n");
	address = 0x80;
	address_2 = 0xE3;
  	seq.addr  = address;
  	seq.flags = I2C_FLAG_WRITE;
  	i2c_write_data[0] = address_2;
  	seq.buf[0].data   = i2c_write_data;
  	seq.buf[0].len    = 1;
  	NVIC_EnableIRQ(I2C0_IRQn);
   	return_check = I2C_TransferInit(I2C0, &seq);


}

void State_Write(void)
{
    LOG_INFO("\nIn the State_Write function\n");
	seq.flags = I2C_FLAG_READ;
	seq.buf[0].data   = i2c_read_data;
	seq.buf[0].len    = 2;
	return_check = I2C_TransferInit(I2C0,&seq);
}

void State_Read(void)
{
	CORE_DECLARE_IRQ_STATE;
				    	  // mask &= ~Para_Passed;
	CORE_ENTER_CRITICAL();
	data |= (i2c_read_data[0]<<8)|(i2c_read_data[1]) ;
	CORE_EXIT_CRITICAL();

	temp=(((175.72*data)/65535)-46.85)*1000;
	LOG_INFO("Temperature in Celsius:%f\n",(float)(temp/1000));
	displayPrintf(DISPLAY_ROW_TEMPVALUE,"%f",(temp/1000));
	//GPIO_PinModeSet(gpioPortD, 15, gpioModePushPull, 0);	//disEnable the I2C module
	LETIMER_IntEnable(LETIMER0,_LETIMER_IF_COMP0_MASK);
	flag = 1;

}

void Conversion_BLE(void)
{
    uint8_t flags = 0x00;
    uint8_t *p = temp11;
    UINT8_TO_BITSTREAM(p, flags);
    temperature = FLT_TO_UINT32(temp, -3);
    UINT32_TO_BITSTREAM(p, temperature);
    gecko_cmd_gatt_server_send_characteristic_notification(0xFF, gattdb_Temperature, 5, temp11);
}

void I2C0_IRQHandler(void)
{
	LOG_INFO("\nI2C IRQ Loop\n");
	return_check = I2C_Transfer(I2C0);
	if(return_check!=i2cTransferInProgress)
	{
		if(return_check==i2cTransferDone)
			{
				LOG_INFO("In the I2c handler irq\n");
				event = I2CComplete;
				Write_Read^=1;
//				mask |= mask_I2C;
//				gecko_external_signal(mask); //call the gecko function

			}
		else
			{
				event=Error;
			}
		mask |= mask_I2C;
		gecko_external_signal(mask); //call the gecko function
	}
	else
	{
		LOG_INFO("Sleep mode deep\n");
		event = SleepDeep;

		mask |= mask_I2C;
		gecko_external_signal(mask); //call the gecko function
	}
	NVIC_ClearPendingIRQ(I2C0_IRQn);
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
			  		//EMU_EnterEM1();//EMU_EnterEM1();
			  	}
			  	else if(Write_Read==Read)
			  	{
					LOG_INFO("Time:%d\t",loggerGetTimestamp(roll));

			  		State_Read();
			  		//Conversion_BLE();
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

			//EMU_EnterEM1();
		}

		else if(event == SleepDeep)
		{
			LOG_INFO("sleepdeep\t");

			////SLEEP_SleepBlockEnd(sleepEM2);
		  	//EMU_EnterEM3(true);
		}
		else if(event == Error)
		{
			LOG_INFO("Time:%d\t",loggerGetTimestamp(roll));

			LOG_INFO("\nEntered in the error state\n");
		  	//NVIC_DisableIRQ(I2C0_IRQn);
		  	//SLEEP_SleepBlockEnd(sleepEM2);
		  	//EMU_EnterEM3(true);
		}

	}


void timerEnable1HzSchedulerEvent(uint32_t Display)
{
	update_display = Display;
}

void gecko_custom_update(struct gecko_cmd_packet* evt)
{
	gecko_update(evt);
	switch(BGLIB_MSG_ID(evt->header))
	{
    // This boot event is generated when the system boots up after reset
		case gecko_evt_system_boot_id:
		LOG_INFO("\r\nBLE Central started\r\n");
		displayPrintf(DISPLAY_ROW_CONNECTION,"Advertising");
					displayPrintf(DISPLAY_ROW_NAME,"Server");
					Addressptr = gecko_cmd_system_get_bt_address();
					displayPrintf(DISPLAY_ROW_BTADDR,"%02x:%02x:%02x:%02x:%02x:%02x",Addressptr->address.addr[5],
							Addressptr->address.addr[4],
							Addressptr->address.addr[3],
							Addressptr->address.addr[2],
							Addressptr->address.addr[1],
							Addressptr->address.addr[0]);

		gecko_cmd_system_set_tx_power(0); //Initial power set to 0
		gecko_cmd_le_gap_set_advertise_timing(0, 400, 400, 0, 0);
		gecko_cmd_le_gap_start_advertising(0, le_gap_general_discoverable, le_gap_connectable_scannable);
		break;

        // This event is generated when a new connection is established
		case gecko_evt_le_connection_opened_id:
		// Add connection to the connection_properties array
		Open_Close_Connection = evt->data.evt_le_connection_opened.connection;
		gecko_cmd_le_connection_set_parameters(Open_Close_Connection, 60, 60,3,300);
		LETIMER_Enable(LETIMER0,true);
		displayPrintf(DISPLAY_ROW_CONNECTION,"Connected");
		displayPrintf(DISPLAY_ROW_NAME,"Server");
		LOG_INFO("\nSuccessful Opened\n");
		break;

		case gecko_evt_le_connection_closed_id:
		Connection_Established = 0;
		gecko_cmd_le_gap_start_advertising(0, le_gap_general_discoverable, le_gap_connectable_scannable);
		displayPrintf(DISPLAY_ROW_CONNECTION,"Advertising");
		displayPrintf(DISPLAY_ROW_NAME,"Server");
		displayPrintf(DISPLAY_ROW_TEMPVALUE,"%s"," ");
		LOG_INFO("\nSuccessful Closed\n");
		LETIMER_Enable(LETIMER0,false);
		break;

		case gecko_evt_gatt_server_characteristic_status_id:
		//If changed by temperature value change the value continuously
		displayPrintf(DISPLAY_ROW_CONNECTION,"Handling Indications");
		if((evt->data.evt_gatt_server_characteristic_status.characteristic == gattdb_Temperature)
	    &&(evt->data.evt_gatt_server_characteristic_status.status_flags == 0x01)) //book reference
			{
				LOG_INFO("\nStatus id loop\n\r");
				Connection_Established = 1;
			}
	      break;


		case gecko_evt_system_external_signal_id:
	    LOG_INFO("Clock Frequency:%d",CMU_ClockFreqGet(cmuClock_LFA));
	    if(Connection_Established == 1)
		    {
			    Event_Status_Retun_Back = evt->data.evt_system_external_signal.extsignals;

			    if(Event_Status_Retun_Back & Para_Passed )
			    	{
			    	  CORE_DECLARE_IRQ_STATE;
			    	  // mask &= ~Para_Passed;
			    	  CORE_ENTER_CRITICAL();
			    	  mask &= ~Para_Passed ;
			    	  CORE_EXIT_CRITICAL();
			    	  LOG_INFO("Para passed loop\n");
			    	  //I2C_Begin();
			    	 //
			    	  Event_Handler();
			    	  //Softtime is on
			    	  //gecko_cmd_hardware_set_soft_timer(2621, 0, 0);
			    	}
			    if(Event_Status_Retun_Back & mask_I2C )
			    {
			    	  LOG_INFO("\nmask i2c loop\n");

			    	CORE_DECLARE_IRQ_STATE;
			    	// mask &= ~Para_Passed;
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
			    				    	// mask &= ~Para_Passed;
			    	CORE_ENTER_CRITICAL();
			    	mask &= ~update_display;
					CORE_EXIT_CRITICAL();
					displayUpdate();
					LOG_INFO("Disp update 1s\n");
			    }
			    else
			    {
			    	LOG_INFO("In the else loop of event handler\n");
			    	Event_Handler();
			    }
			    }
	    break;

//		case gecko_evt_hardware_soft_timer_id:
//
//				I2C_Write();
//				State_Write();
//				State_Read();
//			    Conversion_BLE();
//			    //Soft timer is off
//			    gecko_cmd_hardware_set_soft_timer(0, 0, 0);
//			    gecko_cmd_le_connection_get_rssi(Open_Close_Connection);
//			    LETIMER_Enable(LETIMER0, true);
//			    LOG_INFO("\nTemperature is sent successfully\n");
//			    break;

		case gecko_evt_le_connection_rssi_id:
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

	}
}
