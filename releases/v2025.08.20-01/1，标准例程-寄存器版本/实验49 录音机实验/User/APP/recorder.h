/**
 ****************************************************************************************************
 * @file        recorder.h
 * @author      正点原子团队(ALIENTEK)
 * @version     V1.0
 * @date        2023-01-06
 * @brief       录音机 应用代码
 * @license     Copyright (c) 2022-2032, 广州市星翼电子科技有限公司
 ****************************************************************************************************
 * @attention
 *
 * 实验平台:正点原子 STM32开发板
 * 在线视频:www.yuanzige.com
 * 技术论坛:www.openedv.com
 * 公司网址:www.alientek.com
 * 购买地址:openedv.taobao.com
 *
 * 修改说明
 * V1.0 20230106
 * 第一次发布
 *
 ****************************************************************************************************
 */

#ifndef __RECORDER_H
#define __RECORDER_H

#include "./SYSTEM/SYS/sys.h"
#include "./FATFS/source/ff.h"
#include "./AUDIOCODEC/wav/wavplay.h" 


#define REC_SAI_RX_DMA_BUF_SIZE     4096        /* 定义RX DMA 数组大小 */
#define REC_SAI_RX_FIFO_SIZE        10          /* 定义接收FIFO大小 */

#define REC_SAMPLERATE              44100       /* 采样率,44.1Khz */

uint8_t recoder_sai_fifo_read(uint8_t **buf);
uint8_t recoder_sai_fifo_write(uint8_t *buf);

void recoder_sai_dma_rx_callback(void);
void recoder_enter_rec_mode(void);
void recoder_wav_init(__WaveHeader *wavhead);
void recoder_msg_show(uint32_t tsec, uint32_t kbps);
void recoder_remindmsg_show(uint8_t mode);
void recoder_new_pathname(uint8_t *pname);
void wav_recorder(void);

#endif












