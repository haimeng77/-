/**
 ****************************************************************************************************
 * @file        dac.h
 * @author      ����ԭ���Ŷ�(ALIENTEK)
 * @version     V1.1
 * @date        2023-03-22
 * @brief       DAC ��������
 * @license     Copyright (c) 2022-2032, ������������ӿƼ����޹�˾
 ****************************************************************************************************
 * @attention
 *
 * ʵ��ƽ̨:����ԭ�� STM32H743������
 * ������Ƶ:www.yuanzige.com
 * ������̳:www.openedv.com
 * ��˾��ַ:www.alientek.com
 * �����ַ:openedv.taobao.com
 *
 * �޸�˵��
 * V1.0 20230322
 * ��һ�η���
 * V1.1 20230322
 * ����dac_triangular_wave����
 *
 ****************************************************************************************************
 */

#ifndef __DAC_H
#define __DAC_H

#include "./SYSTEM/sys/sys.h"


void dac_init(uint8_t outx);    /* DACͨ��1��ʼ�� */ 
void dac_set_voltage(uint8_t outx, uint16_t vol);   /* ����ͨ��1/2�����ѹ */ 
void dac_triangular_wave(uint16_t maxval, uint16_t dt, uint16_t samples, uint16_t n);   /* ������ǲ� */

#endif

















