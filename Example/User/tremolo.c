/*
 * tremolo.c
 *
 *  Created on: 6 Aug 2017
 *      Author: Freakk
 */

#include "main.h"
#include "tremolo.h"

void TremoloInit(Tremolo* trem, float32_t depth, float32_t freq, q15_t* inputBuf, q15_t* outputBuf, uint32_t bufSize, uint32_t samplingF){
	trem->samplingFreq = samplingF;
	trem->mDepth = depth;
	trem->in = inputBuf;
	trem->out = outputBuf;
	trem->bufSize = bufSize;
	trem->state = 0;
	LfoInit(&trem->lfo, samplingF, freq, .5f);
}

// SETTERS ------------------------------

void TremoloSwitch(Tremolo* trem, int state){
	trem->state = state;
}

void TremoloToggle(Tremolo* trem){
	trem->state = (trem->state + 1) % 2;
}

void TremoloSetModDepth(Tremolo* trem, float32_t depth){
	trem->mDepth = depth;
}

void TremoloSetModDepthPerc(Tremolo* trem, int perc){
	trem->mDepth = (float32_t)(.01 * perc);
}

void TremoloSetModFreq(Tremolo* trem, float32_t freq){
	LfoSetFreq(&trem->lfo, freq);
}

void TremoloSetModFreqPerc(Tremolo* trem, int perc){
	LfoSetFreq( &trem->lfo, (float32_t)(MIN_TREM_FREQ + .01 * perc * (MAX_TREM_FREQ - MIN_TREM_FREQ)));
}

// GETTERS ------------------------------
int TremoloGetState(Tremolo* trem){
	return trem->state;
}

// RUN ----------------------------------
void TremoloRun(Tremolo* trem){

	for(uint32_t i=0; i < trem->bufSize; i+=2 ){
		LfoRun(&trem->lfo);
		trem->sineMod = (float32_t)((1.0 - trem->mDepth) + trem->mDepth * trem->lfo.val);
		trem->out[i]   = trem->sineMod * trem->in[i]; // Left
		trem->out[i+1] = trem->sineMod * trem->in[i+1]; // Right

	}
}
