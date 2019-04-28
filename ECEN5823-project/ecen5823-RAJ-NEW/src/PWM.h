/*
 * PWM.h
 *
 *  Created on: Apr 24, 2019
 *      Author: mentor
 */

#ifndef SRC_PWM_H_
#define SRC_PWM_H_

#include "LETIMER.h"
#include "em_chip.h"
#include "em_cmu.h"
#include "em_emu.h"
#include "em_gpio.h"
#include "em_timer.h"
#define lock	(bool)(false)
#define unlock	(bool)(true)

uint32_t topValue;
uint8_t servo_state;
bool door_open;

void PWMInit(void);;
void ServoPosition(bool state);
#endif /* SRC_PWM_H_ */
