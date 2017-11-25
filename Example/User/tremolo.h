/*
 * tremolo.h
 *
 *  Created on: 6 Aug 2017
 *      Author: Freakk
 */

#ifndef EXAMPLE_USER_TREMOLO_H_
#define EXAMPLE_USER_TREMOLO_H_

#define MAX_TREM_FREQ 4 // Hz
#define MIN_TREM_FREQ .5 // Hz

#include "arm_math.h"
#include "lfo.h"

typedef struct {
	uint32_t samplingFreq;
	float32_t tremCounter;
	float32_t mDepth;
	float32_t sineMod;
	q15_t* in;
	q15_t* out;
	uint32_t bufSize;
	int state;
	Lfo lfo;
} Tremolo;

// SETTERS
void TremoloInit(Tremolo* trem, float32_t depth, float32_t freq, q15_t* inputBuf, q15_t* outputBuf, uint32_t bufSize, uint32_t samplingF);
void TremoloSetModDepth(Tremolo* trem, float32_t depth);
void TremoloSetModDepthPerc(Tremolo* trem, int perc);
void TremoloSetModFreq(Tremolo* trem, float32_t freq);
void TremoloSetModFreqPerc(Tremolo* trem, int perc);
void TremoloSwitch(Tremolo* trem, int state);
void TremoloToggle(Tremolo* trem);

// GETTERS
int TremoloGetState(Tremolo* dl);

// RUN
void TremoloRun(Tremolo* trem);

#endif /* EXAMPLE_USER_TREMOLO_H_ */
