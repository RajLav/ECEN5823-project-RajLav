#include <stdbool.h>
#include "native_gecko.h"
#include "log.h"
#include "LETIMER.h"
#include "adc.h"
//#include "em_int.h"

extern void gecko_main_init();
bool mesh_bgapi_listener(struct gecko_cmd_packet *evt);
extern void handle_gecko_event(uint32_t evt_id, struct gecko_cmd_packet *evt);

int main(void)
{

  // Initialize stack

	TimerInitialize();

	gecko_main_init();

	logInit();

	gpioInit();

	displayInit();

//	Button_Init();

	initADC();

	//gecko_init(&config);

//	SLEEP_InitEx(NULL);

  /* Infinite loop */

//	GPIO_PinOutSet(gpioPortD,12);
//	while(1){
//	ADCSample();
//	WaitForFlag();
//	LOG_INFO("Millivolts:%d",millivolts);
//	}

  while (1) {
	  struct gecko_cmd_packet *evt = gecko_wait_event();
	bool pass = mesh_bgapi_listener(evt);
	if (pass) {
		handle_gecko_event(BGLIB_MSG_ID(evt->header), evt);
	}
	//SLEEP_Sleep();
	//gecko_cmd_system_halt(1);
  };
}
