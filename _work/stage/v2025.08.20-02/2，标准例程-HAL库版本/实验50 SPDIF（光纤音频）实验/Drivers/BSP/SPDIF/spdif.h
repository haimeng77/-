/**
 ****************************************************************************************************
 * @file        spdif.h
 * @author      ����ԭ���Ŷ�(ALIENTEK)
 * @version     V1.0
 * @date        2022-09-06
 * @brief       SPDIF ��������
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


#ifndef __SPDIF_H
#define __SPDIF_H

#include "./SYSTEM/sys/sys.h"


/* SPDIF���ƽṹ�� */
typedef struct
{
    uint8_t consta;          /* ����״̬��0 δ����;1 ������ */
    uint8_t saisync;         /* ��SAIʱ���Ƿ�ͬ��,0,δͬ��;1,��ͬ��. */
    uint32_t samplerate;     /* SPDIF������ */
    uint32_t clock;          /* SPDIFʱ��Ƶ�� */
}spdif_struct;

extern spdif_struct spdif_dev;
extern void (*spdif_rx_stop_callback)(void);    /* SPDIF RXֹͣʱ�Ļص����� */

/* SPDIF RX������ģʽ���� */
#define SPDIF_RX_IDLE   0   /* IDLEģʽ */
#define SPDIF_RX_SYNC   1   /* ͬ��ģʽ */
#define SPDIF_RX_RCV    3   /* ��������ģʽ */

/******************************************************************************************/

void spdif_rx_init(void); 
void spdif_rx_mode(uint8_t mode);
void spdif_rx_dma_init(uint32_t *buf0, uint32_t *buf1, uint16_t num, uint8_t width);
uint8_t spdif_rx_waitsync(void);
uint32_t spdif_rx_getsamplerate(void);
void spdif_play_start(void);
void spdif_play_stop(void); 

#endif

