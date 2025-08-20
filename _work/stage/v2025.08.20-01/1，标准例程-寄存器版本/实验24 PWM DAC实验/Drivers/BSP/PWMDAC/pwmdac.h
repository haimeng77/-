/**
 ****************************************************************************************************
 * @file        pwmdac.h
 * @author      ����ԭ���Ŷ�(ALIENTEK)
 * @version     V1.0
 * @date        2023-03-22
 * @brief       PWM DAC��� ��������
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
 *
 ****************************************************************************************************
 */

#ifndef __PWMDAC_H
#define __PWMDAC_H

#include "./SYSTEM/sys/sys.h"


/******************************************************************************************/
/* PWM DAC ���� �� ��ʱ�� ���� */

/* PWMDAC Ĭ����ʹ�� PA3, ��Ӧ�Ķ�ʱ��Ϊ TIM2_CH4, �����Ҫ�޸ĳ�����IO���, ����Ӧ
 * �Ķ�ʱ����ͨ��ҲҪ�����޸�. �����ʵ����������޸�.
 */
#define PWMDAC_GPIO_PORT                    GPIOA
#define PWMDAC_GPIO_PIN                     SYS_GPIO_PIN3
#define PWMDAC_GPIO_AF                      4                                       /* AF����ѡ�� */
#define PWMDAC_GPIO_CLK_ENABLE()            do{ RCC->AHB4ENR |= 1 << 0; }while(0)   /* PA��ʱ��ʹ�� */

#define PWMDAC_TIMX                         TIM15
#define PWMDAC_TIMX_CHY                     2                           /* ͨ��Y,  1<= Y <=4 */
#define PWMDAC_TIMX_CCRX                    PWMDAC_TIMX->CCR2           /* ͨ��Y������ȽϼĴ��� */
#define PWMDAC_TIMX_CLK_ENABLE()            do{ RCC->APB2ENR |= 1 << 16; }while(0)  /* TIM ʱ��ʹ�� */

/******************************************************************************************/

void pwmdac_init(uint16_t arr, uint16_t psc);   /* PWM DAC��ʼ�� */
void pwmdac_set_voltage(uint16_t vol);          /* PWM DAC���������ѹ */

#endif

















