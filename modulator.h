/*
 * modulator.h
 *
 *  Created on: 2016 abe. 10
 *      Author: ibair
 */

#ifndef MODULATOR_H_
#define MODULATOR_H_

void init_modulator(void);
void modulator(void);
void filter(void);
void upsample(void);
void modulate(void);
void mapper(void);
//void funcion_filtro(void);

extern bool packetReceivedUART;

#endif /* MODULATOR_H_ */
