/**
 ****************************************************************************************************
 * @file        wdg.c
 * @author      ����ԭ���Ŷ�(ALIENTEK)
 * @version     V1.0
 * @date        2020-09-06
 * @brief       ���Ź� ��������
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

#include "./BSP/WDG/wdg.h"


IWDG_HandleTypeDef g_iwdg_handle;                       /* �������Ź���� */

/**
 * @brief       ��ʼ���������Ź� 
 * @param       prer  : IWDG_PRESCALER_4~IWDG_PRESCALER_256,��Ӧ4~256��Ƶ
 *   @arg       ��Ƶ���� = 4 * 2^prer. �����ֵֻ����256!
 * @param       rlr   : �Զ���װ��ֵ,0~0XFFF. 
 * @note        ʱ�����(���):Tout=((4 * 2^prer) * rlr) / 32 (ms) 
 * @retval      ��
 */
void iwdg_init(uint32_t prer, uint16_t rlr)
{
    g_iwdg_handle.Instance = IWDG1;
    g_iwdg_handle.Init.Prescaler = prer;                /* ����IWDG��Ƶϵ�� */
    g_iwdg_handle.Init.Reload = rlr;                    /* �Ӽ��ؼĴ��� IWDG->RLR ��װ��ֵ */
    g_iwdg_handle.Init.Window = IWDG_WINDOW_DISABLE;    /* �رմ��ڹ��� */
    HAL_IWDG_Init(&g_iwdg_handle);                      /* ��ʼ��IWDG��ʹ�� */
}

/**
 * @brief       ι�������Ź�
 * @param       ��
 * @retval      ��
 */
void iwdg_feed(void)
{
    HAL_IWDG_Refresh(&g_iwdg_handle);                   /* ι�� */
}
