/*
 * memory
 *
 *  Created on: 2 Nov 2017
 *      Author: freakk
 */

#ifndef EXAMPLE_USER_MEMORY_
#define EXAMPLE_USER_MEMORY_

#define RGB565_BYTE_PER_PIXEL     2
#define ARBG8888_BYTE_PER_PIXEL   4

/**
  * @brief  LCD FB_StartAddress
  * LCD Frame buffer start address : starts at beginning of SDRAM
  */
#define LCD_FRAME_BUFFER          SDRAM_DEVICE_ADDR
#define SDRAM_WRITE_READ_ADDR       ((uint32_t)(LCD_FRAME_BUFFER + (RK043FN48H_WIDTH * RK043FN48H_HEIGHT * ARBG8888_BYTE_PER_PIXEL)))

#define SDRAM_WRITE_READ_ADDR_OFFSET ((uint32_t)0x0800)
#define SRAM_WRITE_READ_ADDR_OFFSET  SDRAM_WRITE_READ_ADDR_OFFSET

#define AUDIO_BLOCK_LENGTH   ((uint32_t)256)
#define AUDIO_REC_START_ADDR         (SDRAM_WRITE_READ_ADDR + SRAM_WRITE_READ_ADDR_OFFSET)
#define AUDIO_BUFFER_IN    AUDIO_REC_START_ADDR
#define AUDIO_BUFFER_OUT   (AUDIO_BUFFER_IN  + (AUDIO_BLOCK_LENGTH * 2 * sizeof(q15_t)))

#define DELAY_MAX_SECONDS 1
#define DELAY_BUFFER_LENGTH  ((uint32_t)(DEFAULT_AUDIO_IN_FREQ * DELAY_MAX_SECONDS)) // stereo
#define DELAY_BUFFER_START_L (AUDIO_BUFFER_OUT + (AUDIO_BLOCK_LENGTH * sizeof(q15_t)))
#define DELAY_BUFFER_START_R (DELAY_BUFFER_START_L + (DELAY_BUFFER_LENGTH * sizeof(q15_t)))
/* The Audio file is flashed with ST-Link Utility @ flash address =  AUDIO_SRC_FILE_ADDRESS */
#define AUDIO_SRC_FILE_ADDRESS       0x08080000   /* Audio file address in flash */


#endif /* EXAMPLE_USER_MEMORY_ */
