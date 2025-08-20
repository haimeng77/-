/**
 ****************************************************************************************************
 * @file        gtim.h
 * @author      ����ԭ���Ŷ�(ALIENTEK)
 * @version     V1.3
 * @date        2023-03-21
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
 * V1.2 20230321
 * 1,����gtim_timx_cap_chy_init����
 * V1.3 20230321
 * 1,֧���ⲿ�����������
 * 2,����gtim_timx_cnt_chy_init,gtim_timx_cnt_chy_get_count��gtim_timx_cnt_chy_restart�������� 
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

/* TIMX PWM������� 
 * ���������PWM����LED0(RED)������
 */
#define GTIM_TIMX_PWM_CHY_GPIO_PORT         GPIOB
#define GTIM_TIMX_PWM_CHY_GPIO_PIN          GPIO_PIN_1
#define GTIM_TIMX_PWM_CHY_GPIO_CLK_ENABLE() do{ __HAL_RCC_GPIOB_CLK_ENABLE(); }while(0)  /* PB��ʱ��ʹ�� */
#define GTIM_TIMX_PWM_CHY_GPIO_AF           GPIO_AF2_TIM3                                /* �˿ڸ��õ�TIM3 */

/* TIMX REMAP���� */
#define GTIM_TIMX_PWM                       TIM3                                         /* TIM3 */
#define GTIM_TIMX_PWM_CHY                   TIM_CHANNEL_4                                /* ͨ��Y,  1<= Y <=4 */
#define GTIM_TIMX_PWM_CHY_CLK_ENABLE()      do{ __HAL_RCC_TIM3_CLK_ENABLE(); }while(0)   /* TIM3 ʱ��ʹ�� */

/********************************* ������ͨ�ö�ʱ�����벶��ʵ����غ궨�� *************************************/

 /* TIMX ���벶���� 
 * ��������벶��ʹ�ö�ʱ��TIM5_CH1,����WK_UP����������
 * �ر�Ҫע��:Ĭ���õ�PA0,���õ�����������!���������IO,��Ӧ����������ʽҲ�ø�!
 */
#define GTIM_TIMX_CAP_CHY_GPIO_PORT         GPIOA
#define GTIM_TIMX_CAP_CHY_GPIO_PIN          GPIO_PIN_0
#define GTIM_TIMX_CAP_CHY_GPIO_CLK_ENABLE() do{ __HAL_RCC_GPIOA_CLK_ENABLE(); }while(0)   /* PA��ʱ��ʹ�� */
#define GTIM_TIMX_CAP_CHY_GPIO_AF           GPIO_AF2_TIM5                                 /* AF����ѡ�� */

#define GTIM_TIMX_CAP                       TIM5                       
#define GTIM_TIMX_CAP_IRQn                  TIM5_IRQn
#define GTIM_TIMX_CAP_IRQHandler            TIM5_IRQHandler
#define GTIM_TIMX_CAP_CHY                   TIM_CHANNEL_1                                 /* ͨ��Y,  1<= Y <=4 */
#define GTIM_TIMX_CAP_CHY_CLK_ENABLE()      do{ __HAL_RCC_TIM5_CLK_ENABLE(); }while(0)    /* TIM5 ʱ��ʹ�� */

/*********************************������ͨ�ö�ʱ���������ʵ����غ궨��*************************************/

/* TIMX �����������
* ������������ʹ�ö�ʱ��TIM2_CH1,����WK_UP����������
* �ر�Ҫע��:Ĭ���õ�PA0,���õ�����������!���������IO,��Ӧ����������ʽҲ�ø�!
*/
#define GTIM_TIMX_CNT_CHY_GPIO_PORT            GPIOA
#define GTIM_TIMX_CNT_CHY_GPIO_PIN             GPIO_PIN_0
#define GTIM_TIMX_CNT_CHY_GPIO_AF              GPIO_AF1_TIM2                                /* AF����ѡ�� */
#define GTIM_TIMX_CNT_CHY_GPIO_CLK_ENABLE()    do{ __HAL_RCC_GPIOA_CLK_ENABLE(); }while(0)  /* PA��ʱ��ʹ�� */

#define GTIM_TIMX_CNT                          TIM2
#define GTIM_TIMX_CNT_IRQn                     TIM2_IRQn
#define GTIM_TIMX_CNT_IRQHandler               TIM2_IRQHandler
#define GTIM_TIMX_CNT_CHY                      TIM_CHANNEL_1                                /* ͨ��Y,  1<= Y <=2 */
#define GTIM_TIMX_CNT_CHY_CLK_ENABLE()         do{ __HAL_RCC_TIM2_CLK_ENABLE(); }while(0)   /* TIM2 ʱ��ʹ�� */

/****************************************************************************************************/

void gtim_timx_int_init(uint16_t arr, uint16_t psc);                                        /* ͨ�ö�ʱ�� ��ʱ�жϳ�ʼ������ */
void gtim_timx_pwm_chy_init(uint16_t arr, uint16_t psc);                                    /* ͨ�ö�ʱ�� PWM��ʼ������ */
void gtim_timx_cap_chy_init(uint16_t arr, uint16_t psc);                                    /* ͨ�ö�ʱ�� ���벶���ʼ������ */
void gtim_timx_cnt_chy_init(uint16_t psc);                                                  /* ͨ�ö�ʱ�� ���������ʼ������ */
uint32_t gtim_timx_cnt_chy_get_count(void);                                                 /* ͨ�ö�ʱ�� ��ȡ������� */
void gtim_timx_cnt_chy_restart(void);                                                       /* ͨ�ö�ʱ�� ���������� */

#endif


