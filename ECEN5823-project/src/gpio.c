/*
 * gpio.c
 *
 *  Created on: Dec 12, 2018
 *      Author: Dan Walkes
 */
#include "gpio.h"
#include "em_gpio.h"
#include <string.h>
#include "LETIMER.h"

#define	LED0_port gpioPortF
#define LED0_pin	4
#define LED1_port gpioPortF
#define LED1_pin 5

void gpioInit()
{
	GPIO_DriveStrengthSet(LED0_port, gpioDriveStrengthWeakAlternateStrong);
	//GPIO_DriveStrengthSet(LED0_port, gpioDriveStrengthWeakAlternateWeak);
	GPIO_PinModeSet(LED0_port, LED0_pin, gpioModePushPull, false);
	GPIO_DriveStrengthSet(LED1_port, gpioDriveStrengthWeakAlternateStrong);
	//GPIO_DriveStrengthSet(LED1_port, gpioDriveStrengthWeakAlternateWeak);
	GPIO_PinModeSet(LED1_port, LED1_pin, gpioModePushPull, false);

  	GPIO_PinModeSet(PortB0, PinB0, gpioModeInput, false);
  	GPIO_PinModeSet(PortB1, PinB1, gpioModeInput, false);

}

void Button_Init(void)
{
	uint32_t int_clear;
	int_clear = GPIO_IntGet();
	GPIO_IntClear(int_clear);
	GPIO_IntConfig(PortB0, PinB0, true, true, true);

	int_clear = GPIO_IntGet();
	GPIO_IntClear(int_clear);
	GPIO_IntConfig(PortB1,PinB1, true, true, true);
}

void gpioLed0SetOn()
{
	GPIO_PinOutSet(LED0_port,LED0_pin);
}
void gpioLed0SetOff()
{
	GPIO_PinOutClear(LED0_port,LED0_pin);
}
void gpioLed1SetOn()
{
	GPIO_PinOutSet(LED1_port,LED1_pin);
}
void gpioLed1SetOff()
{
	GPIO_PinOutClear(LED1_port,LED1_pin);
}

void gpioEnableDisplay()
{
	GPIO_PinOutSet(gpioPortD, 15);//Check
}
void gpioSetDisplayExtcomin(bool high)
{
	if(high)
		GPIO_PinOutSet(gpioPortD,13);
	else
		GPIO_PinOutClear(gpioPortD,13);
}

void GPIO_ODD_IRQHandler(void)
{
	uint32_t iflags;

	iflags = GPIO_IntGetEnabled() & 0x0000AAAA;

	CORE_DECLARE_IRQ_STATE;
	CORE_ENTER_CRITICAL();
		ButtonToggle=GPIO_PinInGet(gpioPortF,7);
		mask |= button_event;
	CORE_EXIT_CRITICAL();
	GPIO_IntClear(iflags);

	gecko_external_signal(mask);
}

void GPIO_EVEN_IRQHandler(void)
{
  uint32_t iflags;
	iflags = GPIO_IntGetEnabled() & 0x00005555;
	CORE_DECLARE_IRQ_STATE;
	CORE_ENTER_CRITICAL();
		ButtonToggle=GPIO_PinInGet(gpioPortF,6);
		mask |= button_event;
	CORE_EXIT_CRITICAL();
	GPIO_IntClear(iflags);

	gecko_external_signal(mask);
}

