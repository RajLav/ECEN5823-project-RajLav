/* Name :- Raj Lavingia
Credits : Dan Walkes
Date :- 3/20/19
*/
#include "LETIMER.h"

#include "init_mcu.h"
#include "init_board.h"
#include "init_app.h"
#include "ble-configuration.h"
#include "board_features.h"


#include "bg_types.h"
#include "native_gecko.h"
#include "gatt_db.h"


#include "em_emu.h"
#include "em_cmu.h"


#include "hal-config.h"

#if defined(HAL_CONFIG)
#include "bsphalconfig.h"
#else
#include "bspconfig.h"
#endif

#include "em_device.h"
#include "em_chip.h"
#include "gpio.h"

#include "sleep.h"
#include "em_letimer.h"


#include "em_assert.h"
#include "em_core.h"
#include "em_rmu.h"

#include "log.h"
#include "i2cspm.h"

#ifndef MAX_CONNECTIONS
#define MAX_CONNECTIONS 4
#endif


uint8_t bluetooth_stack_heap[DEFAULT_BLUETOOTH_HEAP(MAX_CONNECTIONS)];

int main(void)
{
  //Variables initialised before running the gecko init function
  flag = 0;
  Report_Check = 0;
  Event_Status_Retun_Back = 0;
  Para_Passed = 1;
  mask_I2C = 2;
  event = On;
  HEADER=true;
  BOOT=0;
  SERVICE[0] =  0x09;
  SERVICE[1] = 0x18;
  CHARACTERS[0] = 0x1c;
  CHARACTERS[1] = 0x2a;
  ServerAddress.addr[0]=0xc0; ServerAddress.addr[1]=0x29; ServerAddress.addr[2]=0xef; ServerAddress.addr[3]=0x57; ServerAddress.addr[4]=0x0b;
  ServerAddress.addr[5]=0x00;
  Connection_Not_Established_Server=0xff; //error state by default
  Connection_Established_Server_Success = Connection_Not_Established_Server;
  //check the status initial variable declared
  Passkey_Status_Check = false;
  Passkey_Bonding_Status_Fail = false;
  Connection_Established=0;
  ButtonState=0;
  Button_Status_Start_Time=0;
  Passkey_Status_Check=false;
  initMcu();
  initBoard();
  initApp();
//For log initialisation
  logInit();
  Button_UUID();
  //Init_Globals();
  intialization();
  // Initialise serial interface
    RETARGET_SerialInit();
    displayInit();
  //Gecko main initialisation
  gecko_init(&config);
  //Button Start function initialisation
  Button_Start_Define();
  //To set the GPIo pin enable interrupts
  NVIC_EnableIRQ(GPIO_EVEN_IRQn);

    while(1)
    {
       // Always flush the UART buffer before letting the device go to sleep
         RETARGET_SerialFlush();
      evt=gecko_wait_event();
      gecko_custom_update(evt);
    }

}
