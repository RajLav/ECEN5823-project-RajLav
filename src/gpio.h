/*
 * gpio.h
 *
 *  Created on: Dec 12, 2018
 *      Author: Dan Walkes
 */

#ifndef SRC_GPIO_H_
#define SRC_GPIO_H_
#include <stdbool.h>
#define SCHEDULER_SUPPORTS_DISPLAY_UPDATE_EVENT 1	//done
#define TIMER_SUPPORTS_1HZ_TIMER_EVENT	1			//done
#define GPIO_SET_DISPLAY_EXT_COMIN_IMPLEMENTED 	1	//done
#define GPIO_DISPLAY_SUPPORT_IMPLEMENTED		1	//done

void gpioInit();
void gpioLed0SetOn();
void gpioLed0SetOff();
void gpioLed1SetOn();
void gpioLed1SetOff();
void gpioEnableDisplay();
void gpioSetDisplayExtcomin(bool high);
void timerEnable1HzSchedulerEvent(uint32_t);
#endif /* SRC_GPIO_H_ */
