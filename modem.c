/*****************************************************************************
 * modem.c
 *****************************************************************************/

#include <sys/platform.h>
#include "adi_initialize.h"
#include "modem.h"
#include "peripherals.h"


/**
 * If you want to use command program arguments, then place them in the following string.
 */
char __argv_string[] = "";
#include <stdio.h>

int main(int argc, char *argv[])
{

	/**
	 * Initialize managed drivers and/or services that have been added to
	 * the project.
	 * @return zero on success
	 */
	//adi_initComponents();

	initializate_peripherals();
	init_modulator();
	demodulator();
	//checkDACinput();
	//sendThroughDAC();


	while(1) {
		receiveFromUART();

		if(packetReceivedUART){
			packetReceivedUART = false;
			modulator();
		}

		checkDACinput();
		sendThroughDAC();

		checkADCinput();
		getADCinput();

		if(packetReceivedADC){
			packetReceivedADC = false;
			demodulator();
			sendThroughUART();
		}

	}


//	char* fff = "ib";
//	memcpy(send_through_uart,fff,strlen(fff));
//	initializate_peripherals();
//	while(1){
//	sendThroughUART();
//	printf("%s", receive_from_uart);
//	}

//	salirPorUART();

	modulator();

	init_demodulator();
	demodulator();


	/* Begin adding your custom code here */

	return 0;
}

