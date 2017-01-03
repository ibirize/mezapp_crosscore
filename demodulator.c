#include <filter.h>
#include "modem.h"
#include "demodulator.h"
#include <fract2float_conv.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

segment ("sdram0") fract32 received_signal[NUM_SAMPLES_TX];
segment ("sdram0") fract32 received_real[NUM_SAMPLES_RX];
segment ("sdram0") fract32 received_imag[NUM_SAMPLES_RX];

segment ("sdram0") fract32 received_fr_real[NUM_SAMPLES_TX];
segment ("sdram0") fract32 received_fr_imag[NUM_SAMPLES_TX];

segment ("sdram0") fract32 filtered_fr_real[NUM_SAMPLES_TX];
segment ("sdram0") fract32 filtered_fr_imag[NUM_SAMPLES_TX];

segment ("sdram0") fract32 received_filtered_real[NUM_SAMPLES_RX];
segment ("sdram0") fract32 received_filtered_imag[NUM_SAMPLES_RX];


segment ("sdram0") fract32 received_symbol_real[NUMBER_OF_SYMBOLS];
segment ("sdram0") fract32 received_symbol_imag[NUMBER_OF_SYMBOLS];

segment ("sdram0") char symbols[NUMBER_OF_SYMBOLS];

segment ("sdram0") float range1_f=0;
segment ("sdram0") float range2_f=0.6325;


segment ("sdram0") fract32 range1;
segment ("sdram0") fract32 range2;



segment ("sdram0") char prueba;


void init_demodulator(){
	for (int i = 0;  i < NUM_SAMPLES_TX; i++) {
		received_signal[i] = modulated_signal[i];
	}

}
/*
 * This function is used to i
 */
void demodulator(){
	demodulate();
	filter_demodulator();
	dowmsample();
	demapper();
}

/*
 * Demodulates the received signal
 */
void demodulate(){

	for (int i = 0; i < NUM_SAMPLES_TX; i++) {
		received_real[i] =(received_signal[i]*cos_modulator_6KHz[i%8])*1.4142;
		received_imag[i] =(-received_signal[i]*sin_modulator_6KHz[i%8])*1.4142;
	}
}

/*
 * Filters the demodulated signal (square raised cosine filter)
 */
void filter_demodulator(){

	for (int i = 0; i < NUM_COEFFS; i++) {
		delay_real[i] = 0;
		delay_imag[i] = 0;
		received_real[NUM_SAMPLES_TX+i]=0;
		received_imag[NUM_SAMPLES_TX+i]=0;
	}

	fir_init(state_real,filter_coefficients,delay_real,NUM_COEFFS,0);
	fir_init(state_imag,filter_coefficients,delay_imag,NUM_COEFFS,0);

	fir_fr32(received_real,filtered_fr_real,NUM_SAMPLES_RX,&state_real);
	fir_fr32(received_imag,filtered_fr_imag,NUM_SAMPLES_RX,&state_imag);

}

/*
 * downsamples the signal
 */

//coge los symbolos del real y del imaginario
void dowmsample(){
	for (int i = 0; i < NUMBER_OF_SYMBOLS; i++) {
		received_symbol_imag[i]=filtered_fr_imag[2*FILTER_DELAY+i*8];
		received_symbol_real[i]=filtered_fr_real[2*FILTER_DELAY+i*8];
	}
}


void init_ranges(){
	range1=float_to_fr32(range1_f);
	range2=float_to_fr32(range2_f);
}

/*
 * Detects the received symbols
 */
void demapper(){

	int bit_1, bit_2, bit_3, bit_4;
	char symbol_bits;

	init_ranges();

	for ( int i = 0;  i < NUMBER_OF_SYMBOLS; i++) {
		symbol_bits=0;
		if (received_symbol_real[i]<range1) {
			bit_1=0;

			if (received_symbol_real[i]<-range2) {
				bit_2= 0;
			}else {
				bit_2=1;
			}
		}else {
			bit_1=1;

			if (received_symbol_real[i]<range2) {
				bit_2=0;
			}else{
				bit_2=1;
			}
		}


		if (received_symbol_imag[i]<range1) {
			bit_3=1;

			if (received_symbol_imag[i]<-range2) {
				bit_4=0;
			}else {
				bit_4=1;
			}

		}
		else {
			bit_3=0;

			if (received_symbol_imag[i]<range2) {
				bit_4=1;
			}else{
				bit_4=0;
			}
		}


		symbol_bits=(bit_1<<3)+(bit_2<<2)+(bit_3<<1)+bit_4;

	/*	prueba=symbol_bits;
		printf("%d", (prueba));*/
		symbols[i]=symbol_bits;

	}

}
