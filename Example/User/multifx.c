
/* Includes ------------------------------------------------------------------*/
#include "memory.h"
#include "multifx.h"
#include <stdio.h>
#include "string.h"
#include <stdlib.h>
#include "arm_math.h"
#include "tremolo.h"
#include "delay.h"

/* Private typedef -----------------------------------------------------------*/

unsigned char logmsgbuff[128];
char buf[BUFSIZE];

const char * logmsg(char* text, int value, int base)
{
    logmsgbuff[0]='\0';
    char locmessage[128];
    strcat(logmsgbuff,text);
    itoa (value,locmessage,base);
    strcat(logmsgbuff,locmessage);
    return logmsgbuff;
}

/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private function declarations ---------------------------------------------*/
void DualMonoInput(void);
void Bypass(void);
void DualMonoBypass(void);
void Volume(float32_t);
void ApplyFX(uint32_t offset);
float32_t smoothValF(float32_t inVal, float32_t* outVal, float32_t smoothCoeff);
void DrawMeter(q15_t meterValue);
void setDelayMix(int);

/* Private variables ---------------------------------------------------------*/
uint32_t  audio_rec_buffer_state;
float masterVolume = 0.5;

static q15_t *audio_buffer_in = AUDIO_BUFFER_IN;
static q15_t *audio_buffer_out = AUDIO_BUFFER_OUT;

static q15_t stereo_buffer_in[AUDIO_BLOCK_LENGTH];
static q15_t stereo_buffer_out[AUDIO_BLOCK_LENGTH];
static q15_t parallel_fx[AUDIO_BLOCK_LENGTH];

static float32_t parallel_fx_mix = 1; // default to 1 for serial mode

static uint32_t LCD_SIZE_X;
static uint32_t LCD_SIZE_Y;

/* Tremolo */
Tremolo tremolo;

/* Delay */
DelayLine delay;

/* Smooth Values */
// used to smooth transistion between values
float32_t smoothCoeff = (float32_t)(exp(log(0.01f)/( ((float32_t)DEFAULT_SMOOTH_MS) * DEFAULT_AUDIO_IN_FREQ * 0.001)));


/* Public functions ---------------------------------------------------------*/

// Delay ---------------------------------------------------------------------
// ------ Setters ------------------------------------------------------------
void toggleDelay(){
	DelayLineToggle(&delay);
}

void setDelayMix(int val){
	DelayLineSetMix(&delay, val);
}

void setDelayTimeL(int val){
	DelayLineSetTimeLPerc(&delay, val);
}

void setDelayTimeR(int val){
	//DelayLineSetTimeRPerc(&delay, val);
}

void setDelayFeedback(int val){
	DelayLineSetFeedback(&delay, val);
}

// ------ Getters ------------------------------------------------------------
float getDelayTimeLPerc(){
	return DelayLineGetTimeLPerc(&delay);
}
float getDelayTimeRPerc(){
	return 0;
//	return DelayLineGetTimeLPerc(&delay); // TODO
}
float getDelayFeedback(){
	return DelayLineGetFeedback(&delay);
}
float getDelayMix(){
	return DelayLineGetMix(&delay);
}

// Tremolo
void toggleTremolo(){
	TremoloToggle(&tremolo);
}

void setTremoloDepth(int val){
	TremoloSetModDepthPerc(&tremolo, val);
}

void setTremoloFreq(int val){
	TremoloSetModFreqPerc(&tremolo, val);
}

void AudioInit(){
	BSP_LCD_SetFont(&Font12);
	LCD_SIZE_X = BSP_LCD_GetXSize();
	LCD_SIZE_Y = BSP_LCD_GetYSize();

	if (BSP_AUDIO_IN_OUT_Init(INPUT_DEVICE_INPUT_LINE_1, OUTPUT_DEVICE_HEADPHONE, DEFAULT_AUDIO_IN_FREQ, DEFAULT_AUDIO_IN_BIT_RESOLUTION, DEFAULT_AUDIO_IN_CHANNEL_NBR, DEFAULT_AUDIO_IN_VOLUME, DEFAULT_AUDIO_OUT_VOLUME) != AUDIO_OK)
	{
		BSP_LCD_SetTextColor(LCD_COLOR_RED);
		BSP_LCD_DisplayStringAt(0, LCD_SIZE_Y - 95, (uint8_t *)" AUDIO RECORD INIT FAILED", CENTER_MODE);
		BSP_LCD_DisplayStringAt(0, LCD_SIZE_Y - 65, (uint8_t *)" Please reset the board ", CENTER_MODE);
	}

	/* Initialize SDRAM buffers */
	audio_rec_buffer_state = BUFFER_OFFSET_NONE;
	memset(audio_buffer_in, 0, AUDIO_BLOCK_LENGTH*2);
	memset(audio_buffer_out, 0, AUDIO_BLOCK_LENGTH*2);
	memset(parallel_fx, 0, AUDIO_BLOCK_LENGTH*2);

	parallel_fx_mix = 1;

	DelayLineInit(
		&delay,
		DEFAULT_AUDIO_IN_FREQ,
		DELAY_BUFFER_START_L,
		DELAY_BUFFER_LENGTH,
		stereo_buffer_in,
		stereo_buffer_in,
		350,
//		450.0341,
		0.4,
		0.3
	);

	TremoloInit(
		&tremolo,
		0.8,
		3,
		stereo_buffer_in,
		stereo_buffer_in,
		AUDIO_BLOCK_LENGTH,
		DEFAULT_AUDIO_IN_FREQ
	);


	/* Start Recording */
	// Number of elements not size,
	BSP_AUDIO_IN_Record((q15_t*)audio_buffer_in, AUDIO_BLOCK_LENGTH);

	/* Start Playback */
	// Data in bytes, left and right elements
	BSP_AUDIO_OUT_SetAudioFrameSlot(CODEC_AUDIOFRAME_SLOT_02);
	BSP_AUDIO_OUT_Play((q15_t*)AUDIO_BUFFER_OUT, AUDIO_BLOCK_LENGTH * 2);

}

/*
 * Bypass
 */
void Bypass(){ // TODO: re-design Bypass
//	memcpy((q15_t *)(AUDIO_BUFFER_OUT + OFFSET),(q15_t *)(AUDIO_BUFFER_IN + OFFSET), AUDIO_BLOCK_LENGTH);
	for(uint32_t i=0; i < AUDIO_BLOCK_LENGTH; i++ ){
		stereo_buffer_out[i] = stereo_buffer_in[i];
	}
}

void DualMonoInput(){
	for(uint32_t i=0; i < AUDIO_BLOCK_LENGTH; i+=2 ){
		stereo_buffer_in[i+1] = stereo_buffer_in[i];
	}
}

void DualMonoBypass(){

	for(uint32_t i=0; i < AUDIO_BLOCK_LENGTH; i+=2 ){
		stereo_buffer_out[i] = stereo_buffer_in[i]; // Left
		stereo_buffer_out[i + 1] = stereo_buffer_out[i]; // Right
	}

}

/*
 * VU Meter
 * */
static q15_t meterValue = 0;
//static uint32_t meterMax = 0; // used to calibrate max val re-distortion
static float32_t meterSmoothedVal = 0;
static q15_t meterTopY = 0;
static q15_t meterRedY = 0;
static q15_t meterYelY = 0;
static float32_t meterIn[(uint32_t)(AUDIO_BLOCK_LENGTH/2)];
void Meter(uint32_t offset){
	uint32_t i, j;
	for(i=0, j=0; i < AUDIO_BLOCK_LENGTH; i+=2, j++ ){
		meterIn[j] = audio_buffer_in[i + offset]; // extract left channel
	}
	arm_rms_q15(audio_buffer_in, (uint32_t)(AUDIO_BLOCK_LENGTH/2), &meterValue);

	// DrawMeter(meterValue);

//	// Calibrate Max val pre-distortion
//	if(meterValue > meterMax) {
//		meterMax = meterValue;
//		BSP_LCD_DisplayStringAt(0, LCD_SIZE_Y - 20, (uint8_t *)"METER           ", LEFT_MODE);
//		BSP_LCD_DisplayStringAt(0, LCD_SIZE_Y - 20,  (uint8_t *)logmsg("METER ", meterValue, 10), LEFT_MODE);
//	}

	meterValue = 0;

}

float32_t smoothValF(float32_t inVal, float32_t* outVal, float32_t coeff ){

	*outVal = coeff * (*outVal - inVal) + inVal;

	return *outVal;

}

void DrawMeter(q15_t meterValue){

	/* Blank top */
	float32_t value = (1.0f * meterValue) / METER_MAX_RMS ;
	value = smoothValF(value, &meterSmoothedVal, smoothCoeff );
	BSP_LCD_SetTextColor(LCD_COLOR_BLACK);
	meterTopY = (uint32_t)(LCD_SIZE_Y * (1 - value));
	BSP_LCD_FillRect(LCD_SIZE_X - 20, 0, 20, meterTopY);

	/* Fill meter */
	//BSP_LCD_FillRect(LCD_SIZE_X - 20, meterTopY, 20, (uint32_t)(LCD_SIZE_Y * value));
	meterRedY = (uint32_t)(meterTopY *.25);
	meterYelY = (uint32_t)(meterRedY *.25);
	BSP_LCD_SetTextColor(LCD_COLOR_GREEN);
	BSP_LCD_FillRect(LCD_SIZE_X - 20, meterTopY, 20, (uint32_t)(LCD_SIZE_Y - meterTopY));

//	/* Text */
//	BSP_LCD_DisplayStringAt(0, LCD_SIZE_Y - 20, (uint8_t *)"METER           ", LEFT_MODE);
//	BSP_LCD_DisplayStringAt(0, LCD_SIZE_Y - 20,  (uint8_t *)logmsg("METER ", (int)((100.0f/METER_MAX) * meterValue), 10), LEFT_MODE);

}

void Volume(float vol){
	arm_scale_q15(stereo_buffer_out, (q15_t)(UNIT * vol), 0, stereo_buffer_out, AUDIO_BLOCK_LENGTH);
	//memcpy(stereo_buffer_out, stereo_buffer_tmp, AUDIO_BLOCK_LENGTH);
}

void ApplyFX(uint32_t offset){
	/* Always ON */
	Meter(offset);

	DualMonoInput();

	/* Switchable */

	/* Serial FX */
	Bypass();
	//DualMonoBypass();
	if(tremolo.state){
		TremoloRun(&tremolo);
	}


	/* Parallel FX */
	if(delay.state){
		DelayLineRun(&delay);
	}


	// Send to output
	for(uint32_t i = 0; i < AUDIO_BLOCK_LENGTH; i++){
//		// Mix serial and parallel FX
//		stereo_buffer_out[i] = (q15_t)((1-parallel_fx_mix) * stereo_buffer_in[i]) +
//				(q15_t)(parallel_fx_mix * parallel_fx[i]);
		stereo_buffer_out[i] = stereo_buffer_in[i];
	}

//	arm_scale_q15(stereo_buffer_in, (q15_t)(1-parallel_fx_mix), 0, stereo_buffer_in, AUDIO_BLOCK_LENGTH);
//	arm_scale_q15(parallel_fx, (q15_t)(parallel_fx_mix), 0, parallel_fx, AUDIO_BLOCK_LENGTH);
//	arm_add_q15(stereo_buffer_in, parallel_fx, stereo_buffer_out, AUDIO_BLOCK_LENGTH);

	Volume(masterVolume);
}


/*------------------------------------------------------------------------------
       Callbacks implementation:
           the callbacks API are defined __weak in the stm32746g_discovery_audio.c file
           and their implementation should be done the user code if they are needed.
           Below some examples of callback implementations.
  ----------------------------------------------------------------------------*/
/**
  * @brief Manages the DMA Transfer complete interrupt.
  * @param None
  * @retval None
  */
void BSP_AUDIO_IN_TransferComplete_CallBack(void)
{
  audio_rec_buffer_state = BUFFER_OFFSET_FULL;
  /* Process 2nd half block */
  memcpy(stereo_buffer_in,(q15_t *)(AUDIO_BUFFER_IN + AUDIO_BLOCK_LENGTH), AUDIO_BLOCK_LENGTH);
  ApplyFX(AUDIO_BLOCK_LENGTH);
  memcpy((q15_t *)(AUDIO_BUFFER_OUT + AUDIO_BLOCK_LENGTH), stereo_buffer_out, AUDIO_BLOCK_LENGTH);
  return;
}

/**
  * @brief  Manages the DMA Half Transfer complete interrupt.
  * @param  None
  * @retval None
  */
void BSP_AUDIO_IN_HalfTransfer_CallBack(void)
{
  audio_rec_buffer_state = BUFFER_OFFSET_HALF;
  /* Process 1st half block */
  memcpy(stereo_buffer_in,(q15_t *)(AUDIO_BUFFER_IN), AUDIO_BLOCK_LENGTH);
  ApplyFX(0);
  memcpy((q15_t *)(AUDIO_BUFFER_OUT), stereo_buffer_out, AUDIO_BLOCK_LENGTH);
  return;
}

/**
  * @brief  Audio IN Error callback function.
  * @param  None
  * @retval None
  */
void BSP_AUDIO_IN_Error_CallBack(void)
{
  /* This function is called when an Interrupt due to transfer error on or peripheral
     error occurs. */
  /* Display message on the LCD screen */
  BSP_LCD_SetBackColor(LCD_COLOR_RED);
  BSP_LCD_DisplayStringAt(0, LINE(14), (uint8_t *)"       DMA  ERROR     ", CENTER_MODE);

  /* Stop the program with an infinite loop */
  while (BSP_PB_GetState(BUTTON_KEY) != RESET)
  {
    return;
  }
  /* could also generate a system reset to recover from the error */
  /* .... */
}

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
