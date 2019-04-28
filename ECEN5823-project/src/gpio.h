
#ifndef SRC_GPIO_H_
#define SRC_GPIO_H_
#include <stdbool.h>

#define PortB0	gpioPortF
#define PinB0		6
#define Button0_Interrupt_Mask	0x40

#define PortB1	gpioPortF
#define PinB1		7

#define GPIO_SET_DISPLAY_EXT_COMIN_IMPLEMENTED 	1
#define GPIO_DISPLAY_SUPPORT_IMPLEMENTED		1	//done

//***********************************************************************************
// function prototypes
//***********************************************************************************

void gpioInit();
void gpioLed0SetOn();
void gpioLed0SetOff();
void gpioLed1SetOn();
void gpioLed1SetOff();
void gpioEnableDisplay();
void gpioSetDisplayExtcomin(bool high);
void Button_Init(void);
#endif /* SRC_GPIO_H_ */
