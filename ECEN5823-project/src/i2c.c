/*
 * i2c.c
 *
 *  Created on: Mar 13, 2019
 *      Author: Raj Lavingia
 */

#include "LETIMER.h"
#include "i2c.h"

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


//
//
//void check_status()
//{
//		LOG_INFO("\nEntered in check status function \n");
//		temperory_variable = GPIO_PinInGet(gpioPortF,6);
//		temperory_variable^=1;
//		//check for temp variable value it will toggle between 0 and 1 , if 0 then button released , if 1 then button pressed
//		//Here take care of debounce issue, press button for a little bit longer time in order to see the value change
//}
//void Key_Status_IRQ_Handler(void)
//{
//	CORE_AtomicDisableIrq();
//	GPIO_Return_Key = GPIO_IntGet();
//	GPIO_IntClear(GPIO_Return_Key);
//	check_status();
//	if(GPIO_Return_Key == Button_Port_Mask)
//			{
//				button_return_value = true;
//			}
//
//	mask |= Button_Press_Pin;
//	gecko_external_signal(mask);
//	CORE_AtomicEnableIrq();
//}
