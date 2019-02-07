/*Name :- Raj Lavingia
 * Credits : Dan Walkes
 * Date : 2/6/19
 */

//Head file included
#include "I2C.h"

//Globally defined variables

uint16_t addr  = 0X40; //address
uint8_t command = 0XE3; //read mode
uint32_t TimerTicks;
uint32_t ReqTicks;
uint32_t divide=1;
uint32_t selection=1;
uint32_t frequency_1=1;
uint8_t select_sleep=sleepEM1;


const LETIMER_Init_TypeDef letimer=
{
  .enable = true,        /**< Start counting when initialization completes. */
  .debugRun = false ,       /**< Counter shall keep running during debug halt. */
  .comp0Top=true,   /**< Load COMP0 register into CNT when counter underflows. */
  .bufTop=false,      /**< Load COMP1 into COMP0 when REP0 reaches 0. */
  .out0Pol= false ,       /**< Idle value for output 0. */
  .out1Pol = false ,       /**< Idle value for output 1. */
  .ufoa0=false,          /**< Underflow output 0 action. */
  .ufoa1=false,          /**< Underflow output 1 action. */
  .repMode= letimerRepeatFree       /**< Repeat mode. */
 // uint32_t                   topValue;       /**< Top value. Counter wraps when top value matches counter value is reached. */
};


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


 void Sleep(void)
 {

 	if(select_sleep == sleepEM0)
 	{
 		while(1);

 	}

 	else if(select_sleep == sleepEM1)
 		{
 			__WFI();

 		}

 	else if(select_sleep == sleepEM2)
 		{
 		    EMU_EnterEM2(true);

 		}

 	else if(select_sleep == sleepEM3)
 			{
 			    EMU_EnterEM3(false);

 			}

 }

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
 	  LETIMER_CompareSet(LETIMER0,0,(((CMU_ClockFreqGet(cmuClock_LFA)/(divide))*(TOTAL_P))));
 	  LETIMER_IntSet(LETIMER0,COMP0);
 	  NVIC_EnableIRQ(LETIMER0_IRQn);
	  LETIMER_IntEnable(LETIMER0,COMP0);
 	  LETIMER_Enable(LETIMER0,true);
 	  LETIMER_Init(LETIMER0,&letimer);
 }

 //delay function
 void tick_delay(uint32_t temp1)
 {
	 	float temp2 = 0.000001;
	 	TimerTicks=LETIMER_CounterGet(LETIMER0);
 		ReqTicks=(((temp1*temp2*32768)/divide));
 		while(1)
 		{
 				if((TimerTicks-LETIMER_CounterGet(LETIMER0))>ReqTicks)break;
 		}
 }

//interupt handler function
void LETIMER0_IRQHandler(void)
{

	LETIMER_IntDisable(LETIMER0,(_LETIMER_IF_COMP0_MASK|_LETIMER_IF_COMP1_MASK)); //Disable Interrupts

	if(_LETIMER_IF_COMP0_MASK & LETIMER_IntGet(LETIMER0))
	{
		gpioLed1SetOff();
		//gpioLED0SetOff();
		//LETIMER_CompareSet(LETIMER0,0,(((CMU_ClockFreqGet(cmuClock_LFA)/(divide))*(TOTAL_P))));

		LETIMER_IntClear(LETIMER0,_LETIMER_IF_COMP0_MASK);
		SchedulerEvent=true;
		LETIMER_CompareSet(LETIMER0,0,(((CMU_ClockFreqGet(cmuClock_LFA)/(divide))*(TOTAL_P))));
	}

	LETIMER_IntClear(LETIMER0,(_LETIMER_IF_COMP0_MASK|_LETIMER_IF_COMP1_MASK));
	LETIMER_IntEnable(LETIMER0,(_LETIMER_IF_COMP0_MASK|_LETIMER_IF_COMP1_MASK));
}

//initialization of i2c module
void I2C_TempInit(void)
{
	  	  // For turning I2C_LpmOn();
	  	  I2CSPM_Init_TypeDef* I2CSPM_ptr;
	  	  I2CSPM_ptr->sclPin=10;
	  	  I2CSPM_ptr->sdaPin=11;
	  	  I2CSPM_ptr->portLocationScl=14;
	  	  I2CSPM_ptr->portLocationScl=16;
	  	  I2CSPM_Init(I2CSPM_ptr);

	  	  GPIO_PinModeSet(gpioPortD, 15, gpioModePushPull, 1);	//Enable the I2C module
	  	  tick_delay(80000); //this 80ms vaue from datasheet
	  	  I2C_MeasureAndRead(addr,command,seq,status);

	  	  //For turning I2C_LpmOff();
	  	  GPIO_PinModeSet(gpioPortD, 15, gpioModePushPull, 0);	//Disable the I2C module
	  	  tick_delay(80000);
}

//write on the line to sensor
void write()
{
		  seq.addr  = (addr<<1);
		  seq.flags = I2C_FLAG_WRITE;
		  seq.buf[0].data = WriteBuff;
		  seq.buf[0].len = 1;
		  WriteBuff[0] = command;
		  status = I2CSPM_Transfer(I2C0, &seq);
		  if(status!= i2cTransferDone)
			 {
			  LOG_ERROR("Error message status check:%x\n",status);
			 }

}

//read from the sensor
void read()
{
	seq.flags = I2C_FLAG_READ;
	seq.buf[0].data = ReadBuff;
	seq.buf[0].len = 2;
	status = I2CSPM_Transfer(I2C0, &seq); //i2cspm.c taken from
	if(status!=i2cTransferDone)
		  {
			LOG_ERROR("Error message status check:%x\n",status);
		  }

}


//measurements of read and write, finally computed for a formula for converting data to celsius
void I2C_MeasureAndRead(uint16_t addr,uint8_t command,I2C_TransferSeq_TypeDef seq,I2C_TransferReturn_TypeDef status)
{

	  float Celcius=0.0;
	  //logic from I2c forum of simplicity studio and lecture notes

	  write();
	  read();

	  Temp = (ReadBuff[0]<<8);
	  Temp |= (ReadBuff[1]) ;
	  Celcius=((175.72*Temp)/65535)-46.85; //formula from temp sensor datasheet
	  LOG_INFO("%f",Celcius);


}
