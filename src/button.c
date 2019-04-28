/*
 * button.c
 *
 *  Created on: Mar 13, 2019
 *      Author: Raj Lavingia
 */
#include "button.h"
#include "gpio.h"
#include <em_gpio.h>

uint32_t GPIO_Get_info;

void Button_Start_Define(void)
{
	//Setting the button pin
	GPIO_PinModeSet(Button_Port_Define, Button_Port_Pin, gpioModeInput, false);
	GPIO_Get_info = GPIO_IntGet();
	//all interupts clear
	GPIO_IntClear(GPIO_Get_info);
	//rising and falling edge  = true set
	GPIO_IntConfig(Button_Port_Define, Button_Port_Pin, true, true, true);

}


