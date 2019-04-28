#include <src/PWM.h>
#include <stdbool.h>
#include "native_gecko.h"
#include "log.h"
#include "PWM.h"
#include "adc.h"
//#include "em_int.h"
/******************************************************************************
 * COMMON : adc.c,adc.h and PWM_Init() are common files between Yash Gupte and
 * 			Raj Lavingia.
 ******************************************************************************/
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

	PWMInit();

	Button_Init();



  /* Infinite loop */
  while (1) {
	struct gecko_cmd_packet *evt = gecko_wait_event();
	bool pass = mesh_bgapi_listener(evt);
	if (pass) {
		handle_gecko_event(BGLIB_MSG_ID(evt->header), evt);
	}
  };
}
