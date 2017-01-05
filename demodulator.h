/*
 * demodulator.h
 *
 *  Created on: 2016 abe. 10
 *      Author: ibair
 */

#ifndef DEMODULATOR_H_
#define DEMODULATOR_H_

void init_demodulator(void);
void demodulator(void);
void demodulate(void);
void filter_demodulator(void);
void dowmsample(void);
void demapper(void);
void init_ranges(void);

extern bool packetReceivedADC;

#endif /* DEMODULATOR_H_ */
