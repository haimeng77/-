/**
 ****************************************************************************************************
 * @file        wdg.c
 * @author      ����ԭ���Ŷ�(ALIENTEK)
 * @version     V1.1
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
 * V1.1 20230321
 * ����wwdg_init��wwdg_set_counter����
 *
 ****************************************************************************************************
 */

#include "./BSP/WDG/wdg.h"
#include "./BSP/LED/led.h"


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

/* ����WWDG������������ֵ,Ĭ��Ϊ���. */
uint8_t g_wwdg_cnt = 0x7f;

/**
 * @brief       ��ʼ�����ڿ��Ź�
 * @param       tr: T[6:0],������ֵ
 * @param       tw: W[6:0],����ֵ
 * @note        fprer:��Ƶϵ����WDGTB��,��Χ:0~7,��ʾ2^WDGTB��Ƶ
 *              Fwwdg=PCLK3/(4096*2^fprer). һ��PCLK3=100Mhz
 *              ���ʱ��=(4096*2^fprer)*(tr-0X3F)/PCLK3
 *              ����fprer=4,tr=7f,PCLK3=100Mhz
 *              �����ʱ��=4096*16*64/100Mhz=41.94ms
 * @retval      ��
 */
void wwdg_init(uint8_t tr, uint8_t wr, uint8_t fprer)
{
    RCC->APB3ENR |= 1 << 6;     /* ʹ��wwdgʱ�� */
    g_wwdg_cnt = tr & g_wwdg_cnt;/* ��ʼ��WWDG_CNT. */
    RCC->GCR |= 1 << 0;         /* WW1RSC=1,��λWWDG1,Ӳ������ */
    WWDG1->CFR |= fprer << 11;  /* PCLK1/4096�ٳ�2^fprer */
    WWDG1->CFR &= 0XFF80;
    WWDG1->CFR |= wr;           /* �趨����ֵ */
    WWDG1->CR |= g_wwdg_cnt;    /* �趨������ֵ */
    WWDG1->CR |= 1 << 7;        /* �������Ź� */
    sys_nvic_init(2, 3, WWDG_IRQn, 2); /* ��ռ2�������ȼ�3����2 */
    WWDG1->SR = 0X00;           /* �����ǰ�����жϱ�־λ */
    WWDG1->CFR |= 1 << 9;       /* ʹ����ǰ�����ж� */
}

/**
 * @brief       ������WWDG��������ֵ
 * @param       ��
 * @retval      ��
 */
void wwdg_set_counter(uint8_t cnt)
{
    WWDG1->CR = (cnt & 0x7F);   /* ������7λ������ */
}

/**
 * @brief       ���ڿ��Ź��жϷ������
 * @param       ��
 * @retval      ��
 */
void WWDG_IRQHandler(void)
{  
    if (WWDG1->SR & 0X01)               /* ���ж��Ƿ�����WWDG��ǰ�����ж� */
    {
        WWDG1->SR = 0X00;               /* �����ǰ�����жϱ�־λ */
        wwdg_set_counter(g_wwdg_cnt);   /* ���贰�ڿ��Ź���ֵ! */
        LED1_TOGGLE();                  /* �̵���˸ */
    }
}
















