/*
 * peripherals.h
 *
 *  Created on: 15/12/2016
 *      Author: Hector
 */

#ifndef PERIPHERALS_H_
#define PERIPHERALS_H_

#include <fract.h>

#define BUFFER_SIZE 4
#define NUM_SAMPLES 8192

/*Functions*/

void initializate_peripherals(void);
void sendThroughUART(void);
void receiveFromUART(void);
void checkDACinput(void);
void getADCinput(void);
void checkADCinput(void);
void sendThroughDAC(void);

extern unsigned char send_through_uart[];
extern unsigned char receive_from_uart[];
extern fract32 send_through_dac_right[];
extern fract32 received_from_adc_right[];
extern fract32 send_through_dac_left[];
extern fract32 received_from_adc_left[];



#endif /* PERIPHERALs_H_ */
