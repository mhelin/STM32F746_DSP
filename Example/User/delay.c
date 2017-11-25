#include "main.h"
#include "delay.h"
#include "math.h"

// Private functions definitions
float calcDelayFrames(int delayLen, int writePos, float maxTime, int samplingFreq, float dTime);
q15_t calcFracDelay(q15_t* delayBuf, float frac, int bufLen, int readPos);
void calcHeadL(DelayLine* dl);

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
){
	dl->samplingFreq = samplingFreq;
	dl->delayLine = delayLine;
	dl->delayLineLen = delayLineLen;
	dl->audioBufIn = audioBufIn;
	dl->audioBufOut = audioBufOut;

	dl->dMaxTime = (1000 * dl->delayLineLen) / (dl->samplingFreq);
	dl->dTimeL = dTimeL;
	calcHeadL(dl);
	dl->dWritePos  = 0;

	dl->dFeedback = (dFeedback > 0.9) ? 0.9 : dFeedback; // dFeedback must be < 1
	DelayLineSetMix(dl, 50);
	dl->state = 0;

	// init to 0
	for(int i=0; i < dl->delayLineLen; ++i) {
		dl->delayLine[i] = 0;
	}
	dl->sampleL = 0;
}

/// UTILS ------------------------------

float calcDelayFrames(int delayLen, int writePos, float maxTime, int samplingFreq, float dTime){
	// delay time * delay length in seconds
	return delayLen + writePos - (.001 * dTime) * samplingFreq;
}

q15_t calcFracDelay(q15_t* delayBuf, float frac, int bufLen, int readPos){
	return (q15_t)((1- frac) * (delayBuf[readPos]) + frac * (delayBuf[(bufLen + readPos -1) % bufLen]));
}

void calcHeadL(DelayLine* dl){
	 float frames = calcDelayFrames(dl->delayLineLen, dl->dWritePos, dl->dMaxTime, dl->samplingFreq, dl->dTimeL);
	 int framesInt = (int)(floor(frames));
	 dl->dReadPosL = framesInt % dl->delayLineLen;
	 dl->dFracL = frames - framesInt;
}

/// SETTERS ------------------------------

void DelayLineSwitch(DelayLine* dl, int state){
	dl->state = state;
}

void DelayLineToggle(DelayLine* dl){
	dl->state = (dl->state + 1) % 2;
}

void DelayLineSetTimeL(DelayLine* dl, int milliseconds){
	dl->dTimeL = milliseconds;
}

void DelayLineSetTimeLPerc(DelayLine* dl, int val){
	dl->dTimeL = (float)(dl->dMaxTime * val  * .01); // max_ms * perc/100
	calcHeadL(dl);
}

void DelayLineSetMix(DelayLine* dl, int dMix){
	float mix = .01 * 2 * dMix -1;
	//	    Mix Behaviour:
	//	    0   < mix < 0.5 ==>  delay  level attenuation
	//	    0.5 < mix < 1   ==>  direct level attenuation

	if(mix <= 0){
		dl->delayLevel  = 1 + mix;
		dl->directLevel = 1;
	} else {
		dl->delayLevel  = 1;
		dl->directLevel = 1 - mix;
	}
}

void DelayLineSetFeedback(DelayLine* dl, int val){
	dl->dFeedback = (float)(.01 * val * .999 ); // prevent feedback from reaching 1
}


// GETTERS ------------------------------

int DelayLineGetState(DelayLine* dl){
	return dl->state;
}
float DelayLineGetTimeLPerc(DelayLine* dl){
	return (float)(dl->dTimeL/(1.0f * dl->dMaxTime));
}
float DelayLineGetFeedback(DelayLine* dl){
	return (float)(dl->dFeedback);
}
float DelayLineGetMix(DelayLine* dl){
	return (float)(.0f); // TODO
}
// RUN ----------------------------------
void DelayLineRun(DelayLine* dl){

	for(int i = 0; i < AUDIO_BLOCK_LENGTH * .5; i+=2){

		// write to delay line
		dl->delayLine[dl->dWritePos] = dl->delayLevel * dl->dFeedback * dl->sampleL + dl->audioBufIn[i];

		// read from delay line
//		dl->sampleL = calcFracDelay(dl->delayLine, dl->dFracL, dl->delayLineLen, dl->dReadPosL);
		dl->sampleL = dl->delayLine[dl->dReadPosL];

		// write output
		dl->audioBufOut[i] = dl->delayLevel * dl->sampleL + dl->directLevel * dl->audioBufIn[i];

		// increase circular buffer pointers
		dl->dReadPosL  = (dl->dReadPosL+1) % dl->delayLineLen;
		dl->dWritePos  = (dl->dWritePos+1) % dl->delayLineLen;
	}

}
