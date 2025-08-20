/**
 ****************************************************************************************************
 * @file        btim.c
 * @author      ����ԭ���Ŷ�(ALIENTEK)
 * @version     V1.1
 * @date        2023-03-25
 * @brief       ������ʱ�� ��������
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
 * V1.0 20230321
 * ��һ�η��� 
 * V1.1 20230325
 * 1, ���� btim_tim7_int_init ����
 *
 ****************************************************************************************************
 */


#include "./BSP/LED/led.h"
#include "./BSP/TIMER/btim.h"
#include "./SYSTEM/usart/usart.h"


/* ��Ƶ����֡�ʿ���ȫ�ֱ��� */
uint16_t g_avi_frame;                /* ����֡�� */
volatile uint8_t g_avi_frameup;      /* ��Ƶ����ʱ϶���Ʊ���,������1��ʱ��,���Ը�����һ֡��Ƶ */

/**
 * @brief       ������ʱ��TIMX�жϷ�����
 * @param       ��
 * @retval      ��
 */
void BTIM_TIMX_INT_IRQHandler(void)
{ 
    if (BTIM_TIMX_INT->SR & 0X0001)     /* ����ж� */
    {
        printf("frame:%dfps\r\n", g_avi_frame);  /* ��ӡ֡�� */
        g_avi_frame = 0;
    }

    BTIM_TIMX_INT->SR &= ~(1 << 0);     /* ����жϱ�־λ */
} 

/**
 * @brief       ������ʱ��TIMX��ʱ�жϳ�ʼ������
 * @note
 *              ������ʱ����ʱ������APB1,��PPRE1 �� 2��Ƶ��ʱ��
 *              ������ʱ����ʱ��ΪAPB1ʱ�ӵ�2��, ��APB1Ϊ100M, ���Զ�ʱ��ʱ�� = 200Mhz
 *              ��ʱ�����ʱ����㷽��: Tout = ((arr + 1) * (psc + 1)) / Ft us.
 *              Ft=��ʱ������Ƶ��,��λ:Mhz
 *
 * @param       arr: �Զ���װֵ��
 * @param       psc: ʱ��Ԥ��Ƶ��
 * @retval      ��
 */
void btim_timx_int_init(uint16_t arr, uint16_t psc)
{
    BTIM_TIMX_INT_CLK_ENABLE();

    BTIM_TIMX_INT->ARR = arr;           /* �趨�������Զ���װֵ */
    BTIM_TIMX_INT->PSC = psc;           /* ����Ԥ��Ƶ��  */
    BTIM_TIMX_INT->DIER |= 1 << 0;      /* ��������ж� */
    BTIM_TIMX_INT->CR1 |= 1 << 0;       /* ʹ�ܶ�ʱ��TIMX */
    sys_nvic_init(1, 3, BTIM_TIMX_INT_IRQn, 2); /* ��ռ1�������ȼ�3����2 */
}

/**
 * @brief       ������ʱ��TIM7�жϷ�����
 * @param       ��
 * @retval      ��
 */
void TIM7_IRQHandler(void)
{
    if (TIM7->SR & 0X0001)      /* ����ж� */
    {
        g_avi_frameup = 1;      /* ���ʱ�䵽 */
    }

    TIM7->SR &= ~(1 << 0);      /* ����жϱ�־λ */
}

/**
 * @brief       ������ʱ��TIMX��ʱ�жϳ�ʼ������
 * @note
 *              ������ʱ����ʱ������APB1,��PPRE1 �� 2��Ƶ��ʱ��
 *              ������ʱ����ʱ��ΪAPB1ʱ�ӵ�2��, ��APB1Ϊ100M, ���Զ�ʱ��ʱ�� = 200Mhz
 *              ��ʱ�����ʱ����㷽��: Tout = ((arr + 1) * (psc + 1)) / Ft us.
 *              Ft=��ʱ������Ƶ��,��λ:Mhz
 *
 * @param       arr: �Զ���װֵ��
 * @param       psc: ʱ��Ԥ��Ƶ��
 * @retval      ��
 */
void btim_tim7_int_init(uint16_t arr, uint16_t psc)
{
    RCC->APB1LENR |= 1 << 5;    /* ʹ��TIM7ʱ�� */
    
    while((RCC->APB1LENR&(1 << 5)) == 0);   /* �ȴ�ʱ��ʹ�ܳɹ�����H7XX��Ҫ�� */ 

    TIM7->ARR = arr;            /* �趨�������Զ���װֵ */
    TIM7->PSC = psc;            /* ����Ԥ��Ƶ��  */
    TIM7->DIER |= 1 << 0;       /* ��������ж� */
    TIM7->CR1 |= 1 << 0;        /* ʹ�ܶ�ʱ��TIMX */
    sys_nvic_init(0, 3, TIM7_IRQn, 2);  /* ��ռ0�������ȼ�3����2 */
}













