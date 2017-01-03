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

	frame[0] = 0xAF;
	frame[1] = 0xAB;

	initializate_peripherals();
	//checkDACinput();
	//sendThroughDAC();


	while(1) {
	for(int i = 0; i < NUM_SAMPLES; i++){
		send_through_dac_right[i] = float_to_fr32(0.9*sin_modulator_6KHz[i%6]);
		send_through_dac_left[i] = float_to_fr32(0.9*cos_modulator_6KHz[i%6]);
	}

	checkDACinput();
	sendThroughDAC();
/*
	for(int i = 0; i < NUM_SAMPLES; i++){
		send_through_dac_right[i] = -1;

		if(i == -1)
			send_through_dac_left[i] = -1;
		else if(i % 8 == 0)
			send_through_dac_left[i] = 1;
		else
			send_through_dac_left[i] = 0;
	}

	checkDACinput();
	sendThroughDAC();*/
}


//	char* fff = "ib";
//	memcpy(send_through_uart,fff,strlen(fff));
//	initializate_peripherals();
//	while(1){
//	sendThroughUART();
//	printf("%s", receive_from_uart);
//	}

//	salirPorUART();
	init_modulator();
	modulator();

	init_demodulator();
	demodulator();


	/* Begin adding your custom code here */

	return 0;
}

