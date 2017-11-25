/*
 * delay.h
 *
 *  Created on: 6 Aug 2017
 *      Author: Freakk
 */
#include "arm_math.h"
#include "main.h" // for AUDIO_BLOCK_LENGTH

#ifndef EXAMPLE_USER_DELAY_H_
#define EXAMPLE_USER_DELAY_H_

typedef struct {
	int samplingFreq;

	q15_t* delayLine;
	int delayLineLen;

	int dReadPosL;
	float dFracL;
	int dWritePos;

	q15_t sampleL;

	q15_t* audioBufIn; // length AUDIO_BLOCK_LENGTH
	q15_t* audioBufOut;

	float dMaxTime;
	float dTimeL;

	float dFeedback;
	float delayLevel;
	float directLevel;

	int counter;
	int state;

} DelayLine;

void DelayLineInit(
	DelayLine* dl,
	int samplingFreq,
	q15_t* delayLine,
	int delayLineLen,
	q15_t* audioBufIn,
	q15_t* audioBufOut,
	float dTimeL,
	float dFeedback,
	float dMix
);

// Setters
void DelayLineSetTimeL(DelayLine* dl, int milliseconds);
void DelayLineSetTimeLPerc(DelayLine* dl, int val);
void DelayLineSetFeedback(DelayLine* dl, int feedback);
void DelayLineSetMix(DelayLine* dl, int dMix);
void DelayLineToggle(DelayLine* dl);

void DelayLineRun(DelayLine* dl);

// Getters
int DelayLineGetState(DelayLine* dl);
float DelayLineGetTimeLPerc(DelayLine* dl);
float DelayLineGetFeedback(DelayLine* dl);
float DelayLineGetMix(DelayLine* dl);


#endif /* EXAMPLE_USER_DELAY_H_ */
