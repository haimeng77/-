/**
 ****************************************************************************************************
 * @file        wdg.c
 * @author      ����ԭ���Ŷ�(ALIENTEK)
 * @version     V1.0
 * @date        2023-03-21
 * @brief       ���Ź� ��������
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
 *
 ****************************************************************************************************
 */

#include "./BSP/WDG/wdg.h"

/**
 * @brief       ��ʼ���������Ź� 
 * @param       prer: ��Ƶ��:0~7(ֻ�е�3λ��Ч!)
 *   @arg       ��Ƶ���� = 4 * 2^prer. �����ֵֻ����256!
 * @param       rlr: �Զ���װ��ֵ,0~0XFFF. 
 * @note        ʱ�����(���):Tout=((4 * 2^prer) * rlr) / 32 (ms). 
 * @retval      ��
 */
void iwdg_init(uint8_t prer, uint16_t rlr)
{
    IWDG1->KR = 0X5555; /* ʹ�ܶ�IWDG->PR��IWDG->RLR��д */
    IWDG1->PR = prer;   /* ���÷�Ƶϵ�� */
    IWDG1->RLR = rlr;   /* �Ӽ��ؼĴ��� IWDG->RLR */
    IWDG1->KR = 0XAAAA; /* reload */
    IWDG1->KR = 0XCCCC; /* ʹ�ܿ��Ź� */
}

/**
 * @brief       ι�������Ź�
 * @param       ��
 * @retval      ��
 */
void iwdg_feed(void)
{
    IWDG1->KR = 0XAAAA; /* reload */
}








