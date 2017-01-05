//#include "filter_coeffs.h"
#include <filter.h>
#include "modulator.h"
#include "modem.h"
#include "peripherals.h"
#include <fract2float_conv.h>


#define SQRT_10 3.17
char frame[FRAME];
segment ("sdram0") fract32 frame_symbols_real[NUMBER_OF_SYMBOLS];
segment ("sdram0") fract32 frame_symbols_imag[NUMBER_OF_SYMBOLS];

segment ("sdram0") fract32 frame_symbols_real_upsample[NUM_SAMPLES_TX];
segment ("sdram0") fract32 frame_symbols_imag_upsample[NUM_SAMPLES_TX];
segment ("sdram0") fract32 modulated_synchronization[NUM_SAMPLES_TX];


/*
 * For filter use
 */


segment ("sdram0") float constelation_real_f[] = {-3, -3, -3, -3, -1, -1 , -1, -1, 3, 3,   3, 3, 1, 1, 1, 1};
segment ("sdram0") float constelation_imag_f[] = { 3,  1, -3, -1,  3,  1,  -3, -1, 3, 1, -3, -1, 3, 1, -3, -1};

segment ("sdram0") fract32 constelation_imag[16];
segment ("sdram0") fract32 constelation_real[16];


//Para Fc = 6 KHz
segment ("sdram0") float sin_modulator_6KHz[] = {0,    0.7071, 1,    0.7071,        0,    -0.7071,    -1,    -0.7071};
segment ("sdram0") float cos_modulator_6KHz[] = {1,    0.7071,    0,    -0.7071,    -1,    -0.7071,    0,    0.7071};


//Para  Fc=12kHz
segment ("sdram0") float sin_modulator_12KHz[] = {0,    1, 0,    -1,        0,    1,    0,    -1};
segment ("sdram0") float cos_modulator_12KHz[] = {1,    0,    -1,    0,    1,    0,    -1,    0};


segment ("sdram0") fract32 filtered_real_symbols[NUM_SAMPLES_TX];
segment ("sdram0") fract32 filtered_imag_symbols[NUM_SAMPLES_TX];

fir_state_fr32 state_real;
fir_state_fr32 state_imag;

segment ("sdram0") fract32 delay_real[NUM_COEFFS];
segment ("sdram0") fract32 delay_imag[NUM_COEFFS];


/*
 * For the modulator
 */

segment ("sdram0") fract32 modulated_signal[NUM_SAMPLES_TX];

/*
 * This function is used in order to prepare the signal to send from the dac
 */

void init_modulator(){
	for (int i = 0;  i < 16; i++) {
		constelation_real[i]=	float_to_fr32(constelation_real_f[i]/SQRT_10);
		constelation_imag[i]=	float_to_fr32(constelation_imag_f[i]/SQRT_10);//3.17 es sqrt(10)
	}


}
void modulator(){
	mapper();
	upsample();
	filter();
	modulate();
}

/*
 * The function used to parse the signal from binary to symbols
 */
void mapper(){
	int i=0;
	int numDecimal = 0;
	for (i = 0; i < NUMBER_OF_SYMBOLS; i++) {
		if(i%2 != 0) numDecimal = receive_from_uart[i/2] & 0xF; //pares numeros de la derecha
		else numDecimal = ((receive_from_uart[i/2] & 0xF0)>>4); //impares numeros de la izquierda

		frame_symbols_real[i] = constelation_real[numDecimal];
		frame_symbols_imag[i] = constelation_imag[numDecimal];
	}
}

/*
 * The function used to upsample the signal
 */
void upsample(){
	int i=0;

	for (i = 0; i < NUM_SAMPLES_TX; i++) {

		if(i%8 == 0 && i < NUMBER_OF_SYMBOLS_OVERSAMPLED){

			if(i==0){
				modulated_synchronization[i] = 0x80000000;
			}else{
				modulated_synchronization[i] = 0x7FFFFFFF;
			}

			frame_symbols_imag_upsample[i] = frame_symbols_imag[i/8];
			frame_symbols_real_upsample[i] = frame_symbols_real[i/8];
		}else{
			frame_symbols_imag_upsample[i] = 0;
			frame_symbols_real_upsample[i] = 0;
			modulated_synchronization[i] = 0;
		}
	}
}

/*
 * The function used to filter the real and imaginary symbols out
 */
void filter(){

	//Filters the signal
	fir_fr32(frame_symbols_real_upsample, filtered_real_symbols, NUM_SAMPLES_TX, &state_real);
	fir_fr32(frame_symbols_imag_upsample, filtered_imag_symbols, NUM_SAMPLES_TX, &state_imag);

}



/*
 * The function used to modulate the symbols
 */
void modulate(){
	int i = 0;

	for (int i = 0; i < NUM_SAMPLES_TX; ++i) {
		modulated_signal[i] = (filtered_real_symbols[i]*cos_modulator_6KHz[i%8]- filtered_imag_symbols[i]*sin_modulator_6KHz[i%8])*1.4142;
	}

}
