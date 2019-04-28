/*
 * button.h
 *
 *  Created on: Mar 13, 2019
 *      Author: Raj Lavingia
 */
#include "stdbool.h"

#ifndef BUTTON_H_
#define BUTTON_H_
#define Button_Port_Define	gpioPortF
#define Button_Port_Pin		6
#define Button_Port_Mask	0x40
#define Button_Press_Pin (32)
bool button_return_value;
bool button_return_temp_status;
bool Passkey_Status_Check;
bool Passkey_Bonding_Status_Fail;
void Button_Start_Define(void);

#endif /* BUTTON_H_ */
