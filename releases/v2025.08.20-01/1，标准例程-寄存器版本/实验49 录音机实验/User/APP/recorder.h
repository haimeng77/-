/**
 ****************************************************************************************************
 * @file        recorder.h
 * @author      ����ԭ���Ŷ�(ALIENTEK)
 * @version     V1.0
 * @date        2023-01-06
 * @brief       ¼���� Ӧ�ô���
 * @license     Copyright (c) 2022-2032, ������������ӿƼ����޹�˾
 ****************************************************************************************************
 * @attention
 *
 * ʵ��ƽ̨:����ԭ�� STM32������
 * ������Ƶ:www.yuanzige.com
 * ������̳:www.openedv.com
 * ��˾��ַ:www.alientek.com
 * �����ַ:openedv.taobao.com
 *
 * �޸�˵��
 * V1.0 20230106
 * ��һ�η���
 *
 ****************************************************************************************************
 */

#ifndef __RECORDER_H
#define __RECORDER_H

#include "./SYSTEM/SYS/sys.h"
#include "./FATFS/source/ff.h"
#include "./AUDIOCODEC/wav/wavplay.h" 


#define REC_SAI_RX_DMA_BUF_SIZE     4096        /* ����RX DMA �����С */
#define REC_SAI_RX_FIFO_SIZE        10          /* �������FIFO��С */

#define REC_SAMPLERATE              44100       /* ������,44.1Khz */

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












