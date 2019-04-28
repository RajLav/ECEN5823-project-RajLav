/*
 * adc.c
 *
 *  Created on: Apr 21, 2019
 *      Author: mentor
 */


/******************************************************************************
 * COMMON : adc.c,adc.h and PWM_Init() are common files between Yash Gupte and
 * 			Raj Lavingia.
 ******************************************************************************/
#include <src/IR.h>
#include <src/PWM.h>

/**************************************************************************//**
 * @brief  Initialize ADC function
 *****************************************************************************/
void initADC (void)
{
  adcFinished=false;
  // Enable ADC0 clock
  CMU_ClockEnable(cmuClock_ADC0, true);

  // Declare init structs
  ADC_Init_TypeDef init = ADC_INIT_DEFAULT;
  ADC_InitSingle_TypeDef initSingle = ADC_INITSINGLE_DEFAULT;

  // Modify init structs and initialize
  init.prescale = ADC_PrescaleCalc(adcFreq, 0); // Init to max ADC clock for Series 1

  initSingle.diff       = false;        // single ended
  initSingle.reference  = adcRef2V5;    // internal 2.5V reference
  initSingle.resolution = adcRes12Bit;  // 12-bit resolution
  initSingle.acqTime    = adcAcqTime4;  // set acquisition time to meet minimum requirement

  // Select ADC input.
  initSingle.posSel = adcPosSelAPORT2XCH9;

  ADC_Init(ADC0, &init);
  ADC_InitSingle(ADC0, &initSingle);
  ADC_IntClear(ADC0,ADC_IF_SINGLE);
  NVIC_EnableIRQ(ADC0_IRQn);
  ADC_IntEnable(ADC0,ADC_IF_SINGLE);
}

void ADCSample(void)
{

  NVIC_EnableIRQ(ADC0_IRQn);
  ADC_IntEnable(ADC0,ADC_IF_SINGLE);
  //millivolts=500;
  adcFinished=false;
  ADC_Start(ADC0, adcStartSingle);
}

#pragma GCC push_options
#pragma GCC optimize("O0")
	void WaitForFlag(void)
	{
		while(adcFinished==false);
	}
#pragma GCC pop_options

void ADC0_IRQHandler(void)
{
	NVIC_DisableIRQ(LETIMER0_IRQn);
	/* Clear ADC0 interrupt flag */
	uint32_t flags = ADC_IntGet(ADC0);
	ADC_IntClear(ADC0, flags);
	/* Read conversion result to clear Single Data Valid flag */
	sample = ADC_DataSingleGet(ADC0);
	millivolts = (sample * 2500) / 4096;
	LOG_INFO("millivolts:%d",millivolts);
	if(millivolts<200)
		adc_convert=true;
	adcFinished=true;
    NVIC_DisableIRQ(ADC0_IRQn);
    ADC_IntDisable(ADC0,ADC_IF_SINGLE);
    NVIC_EnableIRQ(LETIMER0_IRQn);
}

