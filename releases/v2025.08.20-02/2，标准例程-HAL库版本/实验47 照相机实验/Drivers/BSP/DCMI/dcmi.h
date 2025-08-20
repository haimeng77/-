/**
 ****************************************************************************************************
 * @file        dcmi.h
 * @author      ����ԭ���Ŷ�(ALIENTEK)
 * @version     V1.0
 * @date        2022-09-06
 * @brief       DCMI ��������
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
 
#ifndef __DCMI_H
#define __DCMI_H

#include "./SYSTEM/sys/sys.h"

/* DCMI DMA���ջص�����,��Ҫ�û�ʵ�ָú��� */
extern void (*dcmi_rx_callback)(void);

extern DCMI_HandleTypeDef g_dcmi_handle;          /* DCMI��� */
extern DMA_HandleTypeDef g_dma_dcmi_handle;       /* DMA��� */

/* �ӿں��� */
void dcmi_init(void);
void dcmi_stop(void);
void dcmi_start(void);
void dcmi_dma_init(uint32_t mem0addr, uint32_t mem1addr, uint16_t memsize, uint32_t memblen, uint32_t meminc);

/* �����ú��� */
void dcmi_cr_set(uint8_t pclk, uint8_t hsync, uint8_t vsync);
void dcmi_set_window(uint16_t sx, uint16_t sy, uint16_t width, uint16_t height);
void dcmi_cr_set(uint8_t pclk, uint8_t hsync, uint8_t vsync);

#endif
