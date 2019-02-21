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
}

void LETIMER0_IRQHandler(void)
{
	if(LETIMER_IntGet(LETIMER0)&LETIMER_IFC_COMP0)
	LETIMER_IntClear(LETIMER0,_LETIMER_IF_COMP0_MASK);
	LETIMER_CompareSet(LETIMER0,0,(((CMU_ClockFreqGet(cmuClock_LFA)/(divide*1))*(TOTAL_P))));
	gecko_external_signal(Para_Passed); //call the gecko function
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
   	return_check = I2CSPM_Transfer(I2C0, &seq);

}

void State_Write(void)
{
    LOG_INFO("\nIn the State_Write function\n");
	seq.flags = I2C_FLAG_READ;
	seq.buf[0].data   = i2c_read_data;
	seq.buf[0].len    = 2;
	return_check = I2CSPM_Transfer(I2C0,&seq);
}

void State_Read(void)
{
	data |= (i2c_read_data[0]<<8)|(i2c_read_data[1]) ;
	temp=(((175.72*data)/65535)-46.85)*1000;
	LOG_INFO("Temperature in Celsius:%f\n",(float)(temp/1000));
	GPIO_PinModeSet(gpioPortD, 15, gpioModePushPull, 0);	//disEnable the I2C module
	LETIMER_IntEnable(LETIMER0,_LETIMER_IF_COMP0_MASK);

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
				event = I2CComplete;
				Write_Read^=1;
			}
		else event=Error;
	}
	else
	{
		event = Incomplete;
	}

}

void gecko_custom_update(struct gecko_cmd_packet* evt)
{
	gecko_update(evt);
	switch(BGLIB_MSG_ID(evt->header))
	{
    // This boot event is generated when the system boots up after reset
		case gecko_evt_system_boot_id:
		LOG_INFO("\r\nBLE Central started\r\n");
		gecko_cmd_system_set_tx_power(0); //Initial power set to 0
		gecko_cmd_le_gap_set_advertise_timing(0, 400, 400, 0, 0);
		gecko_cmd_le_gap_start_advertising(0, le_gap_general_discoverable, le_gap_connectable_scannable);
		break;

        // This event is generated when a new connection is established
		case gecko_evt_le_connection_opened_id:
		// Add connection to the connection_properties array
		Open_Close_Connection = evt->data.evt_le_connection_opened.connection;
		gecko_cmd_le_connection_set_parameters(Open_Close_Connection, 60, 60,3,30);
		LOG_INFO("\nSuccessful Opened\n");
		break;

		case gecko_evt_le_connection_closed_id:
		Connection_Established = 0;
		gecko_cmd_le_gap_start_advertising(0, le_gap_general_discoverable, le_gap_connectable_scannable);
		LOG_INFO("\nSuccessful Closed\n");
		break;

		case gecko_evt_gatt_server_characteristic_status_id:
		//If changed by temperature value change the value continuously
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
			    if(Event_Status_Retun_Back == Para_Passed )
			    	{
			    	  LOG_INFO("I2C initiated\n");
			    	  I2C_Begin();
			    	  //Softtime is on
			    	  gecko_cmd_hardware_set_soft_timer(2621, 0, 0);
			    	}
			    }
	    break;

		case gecko_evt_hardware_soft_timer_id:

				I2C_Write();
				State_Write();
				State_Read();
			    Conversion_BLE();
			    //Soft timer is off
			    gecko_cmd_hardware_set_soft_timer(0, 0, 0);
			    gecko_cmd_le_connection_get_rssi(Open_Close_Connection);
			    LETIMER_Enable(LETIMER0, true);
			    LOG_INFO("\nTemperature is sent successfully\n");
			    break;

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


