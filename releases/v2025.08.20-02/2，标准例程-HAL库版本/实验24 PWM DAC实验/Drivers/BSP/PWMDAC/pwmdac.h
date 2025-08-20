/**
 ****************************************************************************************************
 * @file        pwmdac.h
 * @author      ����ԭ���Ŷ�(ALIENTEK)
 * @version     V1.0
 * @date        2022-09-06
 * @brief       PWM DAC ��������
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
 
#ifndef __PWMDAC_H
#define __PWMDAC_H

#include "./SYSTEM/sys/sys.h"


/******************************************************************************************/

/* PWMDAC Ĭ����ʹ�� PA3, ��Ӧ�Ķ�ʱ��Ϊ TIM15_CH2, �����Ҫ�޸ĳ�����IO���, ����Ӧ
 * �Ķ�ʱ����ͨ��ҲҪ�����޸�. �����ʵ����������޸�
 */
#define PWMDAC_GPIO_PORT                    GPIOA
#define PWMDAC_GPIO_PIN                     GPIO_PIN_3
#define PWMDAC_GPIO_CLK_ENABLE()            do{ __HAL_RCC_GPIOA_CLK_ENABLE(); }while(0)     /* PA��ʱ��ʹ�� */
#define PWMDAC_GPIO_AFTIMX                  GPIO_AF4_TIM15

#define PWMDAC_TIMX                         TIM15
#define PWMDAC_TIMX_CHY                     TIM_CHANNEL_2                                   /* ͨ��Y,  1<= Y <=4 */
#define PWMDAC_TIMX_CCRX                    PWMDAC_TIMX->CCR2                               /* ͨ��Y������ȽϼĴ��� */
#define PWMDAC_TIMX_CLK_ENABLE()            do{ __HAL_RCC_TIM15_CLK_ENABLE(); }while(0)     /* TIM15 ʱ��ʹ�� */

/******************************************************************************************/

void pwmdac_init(uint16_t arr, uint16_t psc);   /* PWM DAC��ʼ�� */
void pwmdac_set_voltage(uint16_t vol);          /* PWM DAC���������ѹ */

#endif


