/**
 ****************************************************************************************************
 * @file        audioplay.h
 * @author      ����ԭ���Ŷ�(ALIENTEK)
 * @version     V1.0
 * @date        2022-09-06
 * @brief       ���ֲ����� Ӧ�ô���
 * @license     Copyright (c) 2020-2032, ������������ӿƼ����޹�˾
 ****************************************************************************************************
 * @attention
 *
 * ʵ��ƽ̨:����ԭ�� ������ H743������
 * ������Ƶ:www.yuanzige.com
 * ������̳:www.openedv.com
 * ��˾��ַ:www.alientek.com
 * �����ַ:openedv.taobao.com
 *
 * �޸�˵��
 * V1.0 20220906
 * ��һ�η���
 *
 ****************************************************************************************************
 */

#ifndef __AUDIOPLAY_H
#define __AUDIOPLAY_H

#include "./SYSTEM/SYS/sys.h"
#include "./FATFS/source/ff.h"
#include "./AUDIOCODEC/wav/wavplay.h"


#define SAI_TX_DMA_BUF_SIZE    10240        /* ����TX DMA �����С(����192Kbps@24bit��ʱ��,��Ҫ����10240��Ų��Ῠ) */


/* ���ֲ��ſ����� */
typedef __PACKED_STRUCT
{  
    uint8_t *saibuf1;           /* SAI�����BUF */
    uint8_t *saibuf2;
    uint8_t *tbuf;              /* ��ʱ����,����24bit�����ʱ����Ҫ�õ� */
    FIL *file;                  /* ��Ƶ�ļ�ָ�� */

    uint8_t status;             /* bit0:0,��ͣ����;1,�������� */
                                /* bit1:0,��������;1,�������� */
}__audiodev;

extern __audiodev g_audiodev;                                               /* ���ֲ��ſ����� */

void wav_sai_dma_callback(void);
void audio_start(void);                                                     /* ��ʼ��Ƶ���� */
void audio_stop(void);                                                      /* ֹͣ��Ƶ���� */
uint16_t audio_get_tnum(uint8_t *path);                                     /* �õ�path·����,Ŀ���ļ����ܸ��� */
void audio_index_show(uint16_t index, uint16_t total);                      /* ��ʾ��Ŀ���� */
void audio_msg_show(uint32_t totsec, uint32_t cursec, uint32_t bitrate);    /* ��ʾ����ʱ��,������ ��Ϣ */
void audio_play(void);                                                      /* �������� */
uint8_t audio_play_song(uint8_t *fname);                                    /* ����ĳ����Ƶ�ļ� */

#endif

