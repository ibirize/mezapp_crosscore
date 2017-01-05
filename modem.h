/*****************************************************************************
 * modem.h
 *****************************************************************************/

#ifndef __MODEM_H__
#define __MODEM_H__

/* Add your custom header content here */
#define SAMPLING_FREQ    48000    //Hz
#define CARRIER_FREQ    6000    //Hz
#define SYMBOL_FREQ        6000    //Hz

#define OVERSAMPLING                    SAMPLING_FREQ/SYMBOL_FREQ
#define FRAME                           263
#define NUMBER_OF_SYMBOLS               FRAME*2
#define NUMBER_OF_SYMBOLS_OVERSAMPLED   NUMBER_OF_SYMBOLS*8
#define NUM_COEFFS                      49
#define NUM_SAMPLES_TX                  NUMBER_OF_SYMBOLS_OVERSAMPLED + NUM_COEFFS
#define NUM_SAMPLES_RX                  NUMBER_OF_SYMBOLS_OVERSAMPLED + NUM_COEFFS*2
#define FILTER_DELAY                    (NUM_COEFFS-1)/2
#define SYMBOLS_16QAM					16
#define BITS_PER_SYMBOL					4

#include <filter.h>
#include "modulator.h"
#include "demodulator.h"

extern char frame[];
extern fract32 constelation_imag[];
extern fract32 constelation_real[];

extern float sin_modulator_6KHz[];
extern float cos_modulator_6KHz[];

extern float sin_modulator_12KHz[];
extern float cos_modulator_12KHz[];

extern segment ("sdram0") fract32 delay_real[NUM_COEFFS];
extern segment ("sdram0") fract32 delay_imag[NUM_COEFFS];

extern fir_state_fr32 state_real;
extern fir_state_fr32 state_imag;

extern segment ("sdram0") fract32 filter_coefficients[];
extern fract32 modulated_signal[NUM_SAMPLES_TX];
extern fract32 modulated_synchronization[NUM_SAMPLES_TX];






#endif /* __MODEM_H__ */
