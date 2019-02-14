/* Name :- Raj Lavingia
Credits : Dan Walkes
*/

/*Include Header File*/

#include "LETIMER.h"

//global variables defined
uint16_t addr;
uint8_t command;
uint8_t i2c_read_data[2];
uint8_t i2c_write_data[1];
uint16_t data;
float temp;
I2C_TransferSeq_TypeDef    seq;
I2C_TransferReturn_TypeDef ret;
uint8_t SleepMode;
uint8_t roll;
WriteRead Write_Read;
Event event;

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


void intialization(void)
{
	  modes();
	  //LED0 made off
	  gpioLed0SetOff();
	  //LED1 made off
	  gpioLed1SetOff();
	  //LETIMER0

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

//Different modes selection and their respective commands
 void modes(void)
 {
 	if(select_sleep == sleepEM0)
 	{
 		 divide =8;
 		 selection = cmuSelect_LFXO;
 		 frequency_1 = cmuOsc_LFXO;
 	}

 	else if(select_sleep == sleepEM1)
 		{
 			divide =8;
 			selection = cmuSelect_LFXO;
 			frequency_1 = cmuOsc_LFXO;
 		}
 	else if(select_sleep == sleepEM2)
 		{
 			divide =8;
 			selection = cmuSelect_LFXO;
 			frequency_1 = cmuOsc_LFXO;
 		}
 	else if(select_sleep == sleepEM3)
 		{
			//divide =8;
			//selection = cmuSelect_LFXO;
			//frequency_1 = cmuOsc_LFXO;//divide =1;
 		    divide =1;
 		    selection = cmuSelect_ULFRCO;
 			frequency_1 = cmuOsc_ULFRCO;
 		}
 }



void tick_delay(uint32_t us_wait)
{
	uint32_t temp1=LETIMER_CounterGet(LETIMER0);
	uint32_t temp2 = CounterGet(us_wait);
			if(us_wait==0)temp2=1;
			NVIC_DisableIRQ(LETIMER0_IRQn);
			LETIMER_CompareSet(LETIMER0,1,temp2);
			LETIMER_IntSet(LETIMER0,LETIMER_IFC_COMP0|LETIMER_IFC_COMP1);
			LETIMER_IntEnable(LETIMER0,LETIMER_IFC_COMP0|LETIMER_IFC_COMP1);
			NVIC_EnableIRQ(LETIMER0_IRQn);
			event=SleepDeep;

}

void I2C_Startup(void)
{
    LETIMER_IntDisable(LETIMER0,(_LETIMER_IF_COMP0_MASK|_LETIMER_IF_COMP1_MASK)); //Disable Interrupts

   // For turning I2C_LpmOn();
	  	  I2CSPM_Init_TypeDef* I2CSPM_ptr;
	  	  I2CSPM_ptr->sclPin=10;
	  	  I2CSPM_ptr->sdaPin=11;
	  	  I2CSPM_ptr->portLocationScl=14;
	  	  I2CSPM_ptr->portLocationScl=16;
	  	  I2CSPM_Init(I2CSPM_ptr);

	  	  GPIO_PinModeSet(gpioPortD, 15, gpioModePushPull, 1);	//Enable the I2C module
	  	  tick_delay(80000); //this 80ms vaue from datasheet
}


void LETIMER0_IRQHandler(void)
{
	//	LETIMER_IntEnable(LETIMER0,(_LETIMER_IF_COMP0_MASK|_LETIMER_IF_COMP1_MASK));

	if(LETIMER_IntGet(LETIMER0)&LETIMER_IFC_COMP0)
	{
		LETIMER_IntClear(LETIMER0,_LETIMER_IF_COMP0_MASK);
		event=On;
		LETIMER_CompareSet(LETIMER0,0,(((CMU_ClockFreqGet(cmuClock_LFA)/(divide*1))*(TOTAL_P))));
		SLEEP_SleepBlockBegin(sleepEM2);
		++roll;
	}

	else
	{
		LETIMER_IntClear(LETIMER0,_LETIMER_IF_COMP1_MASK);
		event=InitWrite;
		SLEEP_SleepBlockBegin(sleepEM2);
	}
	//LETIMER_IntClear(LETIMER0,(_LETIMER_IF_COMP0_MASK|_LETIMER_IF_COMP1_MASK));
	//	LETIMER_IntEnable(LETIMER0,(_LETIMER_IF_COMP0_MASK|_LETIMER_IF_COMP1_MASK));

}



void I2C_Write(void)
{
  	seq.addr  = addr;
  	seq.flags = I2C_FLAG_WRITE;
  	i2c_write_data[0] = command;
  	seq.buf[0].data   = i2c_write_data;
  	seq.buf[0].len    = 1;
  	NVIC_EnableIRQ(I2C0_IRQn);
  	ret = I2C_TransferInit(I2C0, &seq);
}

void State_Write(void)
{
		seq.flags = I2C_FLAG_READ;
		seq.buf[0].data   = i2c_read_data;
		seq.buf[0].len    = 2;
		ret = I2C_TransferInit(I2C0,&seq);
}

void State_Read(void)
{
	           	data |= (i2c_read_data[0]<<8)|(i2c_read_data[1]) ;
		  		temp=((175.72*data)/65535)-46.85;
		  		LOG_INFO("Temp:%f\n",temp);
	    		 GPIO_PinModeSet(gpioPortD, 15, gpioModePushPull, 0);	//disEnable the I2C module
		  		NVIC_DisableIRQ(I2C0_IRQn);
}



void Event_Handler(void)
{
	if(event == On)
		{
		LOG_INFO("Time:%d\t",loggerGetTimestamp(roll));

		I2C_Startup();
		}
	else if(event == InitWrite)
		{
		LOG_INFO("Time:%d\t",loggerGetTimestamp(roll));

			I2C_Write();
			EMU_EnterEM1();
		}
	else if(event == I2CComplete)

		{

		if(Write_Read==Write)

			  	{
			LOG_INFO("Time:%d\t",loggerGetTimestamp(roll));

				State_Write();
			  		EMU_EnterEM1();//EMU_EnterEM1();
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

			EMU_EnterEM1();
		}

		else if(event == SleepDeep)
		{
			LOG_INFO("Time:%d\t",loggerGetTimestamp(roll));

			SLEEP_SleepBlockEnd(sleepEM2);
		  	EMU_EnterEM3(true);
		}
		else if(event == Error)
		{
			LOG_INFO("Time:%d\t",loggerGetTimestamp(roll));

			LOG_INFO("\nEntered in the error state\n");
		  	NVIC_DisableIRQ(I2C0_IRQn);
		  	SLEEP_SleepBlockEnd(sleepEM2);
		  	EMU_EnterEM3(true);
		}

	}





void I2C0_IRQHandler(void)
{
	ret = I2C_Transfer(I2C0);
	if(ret!=i2cTransferInProgress)
	{
		if(ret==i2cTransferDone)
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

  NVIC_ClearPendingIRQ(I2C0_IRQn);

}

