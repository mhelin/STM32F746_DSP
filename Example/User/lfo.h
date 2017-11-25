/*
 * lfo.h
 *
 *  Created on: 7 Nov 2017
 *      Author: freakk
 */

#ifndef EXAMPLE_USER_LFO_H_
#define EXAMPLE_USER_LFO_H_

#include "arm_math.h"

typedef struct {
	float32_t freq;
	float32_t period; // set to 1.0 for full sine lfo, 0.5 for positive half
	uint32_t samplingFreq;
	uint32_t counter;
	float32_t val;
} Lfo;

void LfoInit(Lfo* lfo, uint32_t samplingFreq, float32_t freq, float32_t period);
float32_t LfoRun(Lfo* lfo);

// SETTERS ------------------------------
void LfoSetFreq(Lfo* lfo, float32_t freq);

// GETTERS ------------------------------
float32_t LfoGetFreq(Lfo* lfo);

#endif /* EXAMPLE_USER_LFO_H_ */
