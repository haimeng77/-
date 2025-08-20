/**
 ****************************************************************************************************
 * @file        gtim.h
 * @author      ����ԭ���Ŷ�(ALIENTEK)
 * @version     V1.1
 * @date        2022-09-06
 * @brief       ͨ�ö�ʱ�� ��������
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
 * V1.1 20220906
 * 1,����gtim_timx_pwm_chy_init����
 * 
 ****************************************************************************************************
 */

#ifndef __GTIM_H
#define __GTIM_H

#include "./SYSTEM/sys/sys.h"


/******************************* ������ͨ�ö�ʱ���ж�ʵ����غ궨�� *************************************/

/* ͨ�ö�ʱ�� ���� */
#define GTIM_TIMX_INT                       TIM3
#define GTIM_TIMX_INT_IRQn                  TIM3_IRQn
#define GTIM_TIMX_INT_IRQHandler            TIM3_IRQHandler
#define GTIM_TIMX_INT_CLK_ENABLE()          do{ __HAL_RCC_TIM3_CLK_ENABLE(); }while(0)  /* TIM3 ʱ��ʹ�� */

/********************************* ������ͨ�ö�ʱ��PWM���ʵ����غ궨�� ********************************/

/* TIMX PWM������� */
#define GTIM_TIMX_PWM_CHY_GPIO_PORT         GPIOB
#define GTIM_TIMX_PWM_CHY_GPIO_PIN          GPIO_PIN_1
#define GTIM_TIMX_PWM_CHY_GPIO_CLK_ENABLE() do{ __HAL_RCC_GPIOB_CLK_ENABLE(); }while(0)  /* PB��ʱ��ʹ�� */
#define GTIM_TIMX_PWM_CHY_GPIO_AF           GPIO_AF2_TIM3                                /* �˿ڸ��õ�TIM3 */

/* TIMX REMAP���� */
#define GTIM_TIMX_PWM                       TIM3                                         /* TIM3 */
#define GTIM_TIMX_PWM_CHY                   TIM_CHANNEL_4                                /* ͨ��Y,  1<= Y <=4 */
#define GTIM_TIMX_PWM_CHY_CCRX              TIM3->CCR4                                   /* ͨ��Y������ȽϼĴ��� */
#define GTIM_TIMX_PWM_CHY_CLK_ENABLE()      do{ __HAL_RCC_TIM3_CLK_ENABLE(); }while(0)   /* TIM3 ʱ��ʹ�� */

/****************************************************************************************************/

void gtim_timx_int_init(uint16_t arr, uint16_t psc);            /* ͨ�ö�ʱ�� ��ʱ�жϳ�ʼ������ */
void gtim_timx_pwm_chy_init(uint16_t arr, uint16_t psc);        /* ͨ�ö�ʱ�� PWM��ʼ������ */

#endif




