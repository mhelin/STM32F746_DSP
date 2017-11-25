#include "lfo.h"

void LfoInit(Lfo* lfo, uint32_t samplingFreq, float32_t freq, float32_t period){
	lfo->samplingFreq = samplingFreq;
	lfo->freq = freq;
	lfo->counter = 0;
	lfo->val = 0;
	lfo->period = period;
}

// SETTERS ------------------------------
void LfoSetFreq(Lfo* lfo, float32_t freq){
	lfo->freq = freq;
}

// GETTERS ------------------------------
float32_t LfoGetFreq(Lfo* lfo){
	return lfo->freq;
}

float32_t LfoRun(Lfo *lfo){
	if(lfo->counter < ((lfo->period/lfo->freq) * lfo->samplingFreq)) // counter goes from 0 to T = 1/f
	{
		++(lfo->counter);
	} else {
		lfo->counter = 0;
	}

	lfo->val = arm_sin_f32( ((float32_t)(2 *3.14 * (lfo->freq)) * ((1.0 *lfo->counter)/lfo->samplingFreq)));
	return lfo->val;
}


