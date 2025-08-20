/**
 ****************************************************************************************************
 * @file        adc.h
 * @author      ����ԭ���Ŷ�(ALIENTEK)
 * @version     V1.0
 * @date        2022-09-06
 * @brief       ADC ��������
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

#ifndef __ADC_H
#define __ADC_H

#include "./SYSTEM/sys/sys.h"

/******************************************************************************************/

/* ADC������ ���� */
#define ADC_ADCX_CHY_GPIO_PORT              GPIOA
#define ADC_ADCX_CHY_GPIO_PIN               GPIO_PIN_5
#define ADC_ADCX_CHY_GPIO_CLK_ENABLE()      do{ __HAL_RCC_GPIOA_CLK_ENABLE(); }while(0)         /* PA��ʱ��ʹ�� */

#define ADC_ADCX                            ADC1
#define ADC_ADCX_CHY                        ADC_CHANNEL_19                                      /* ͨ��Y,  0 <= Y <= 19 */ 
#define ADC_ADCX_CHY_CLK_ENABLE()           do{ __HAL_RCC_ADC12_CLK_ENABLE(); }while(0)         /* ADC1 ʱ��ʹ�� */

/* ADC��ͨ��/��ͨ�� DMA�ɼ� DMA��������� ����
 * ע��: �������ǵ�ͨ������ʹ������Ķ���.
 */
#define ADC_ADCX_DMASx                      DMA1_Stream7 
#define ADC_ADCX_DMASx_REQ                  DMA_REQUEST_ADC1             /* ADC1_DMA����Դ */
#define ADC_ADCX_DMASx_IRQn                 DMA1_Stream7_IRQn 
#define ADC_ADCX_DMASx_IRQHandler           DMA1_Stream7_IRQHandler 

#define ADC_ADCX_DMASx_IS_TC()              ( DMA1->HISR & (1 << 27) )   /* ��ȡDMA1 Stream7������ɱ�־λ��
                                                                          * ����һ���ٺ�����ʽ�����ܵ�����ʹ��
                                                                          */

#define ADC_ADCX_DMASx_CLR_TC()             do{ DMA1->HIFCR |= 1 << 27; }while(0)  /* ���DMA1 Stream7������ɱ�־λ */

/******************************************************************************************/

void adc_init(void);                                                     /* ADCͨ����ʼ�� */
uint32_t adc_get_result(uint32_t ch);                                    /* ���ĳ��ͨ��ֵ  */
uint32_t adc_get_result_average(uint32_t ch, uint8_t times);             /* �õ�ĳ��ͨ����������������ƽ��ֵ */

void adc_dma_init(uint32_t par, uint32_t mar);                           /* ADC DMA�ɼ���ʼ�� */
void adc_dma_enable(uint16_t ndtr);                                      /* ʹ��һ��ADC DMA�ɼ����� */

void adc_nch_dma_init(uint32_t par, uint32_t mar);                       /* ADC��ͨ�� DMA�ɼ���ʼ�� */
void adc_oversample_init(uint32_t osr, uint32_t ovss);                   /* ADC��������ʼ�� */

#endif 
