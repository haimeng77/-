/**
 ****************************************************************************************************
 * @file        main.c
 * @author      ����ԭ���Ŷ�(ALIENTEK)
 * @version     V1.0
 * @date        2022-09-6
 * @brief       �������Ź� ʵ��
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
 ****************************************************************************************************
 */

#include "./SYSTEM/sys/sys.h"
#include "./SYSTEM/usart/usart.h"
#include "./SYSTEM/delay/delay.h"
#include "./BSP/LED/led.h"
#include "./BSP/KEY/key.h"
#include "./BSP/WDG/wdg.h"


int main(void)
{
    sys_cache_enable();                         /* ��L1-Cache */
    HAL_Init();                                 /* ��ʼ��HAL�� */
    sys_stm32_clock_init(160, 5, 2, 4);         /* ����ʱ��, 400Mhz */
    delay_init(400);                            /* ��ʱ��ʼ�� */
    usart_init(115200);                         /* ��ʼ��USART */
    led_init();                                 /* ��ʼ��LED */
    key_init();                                 /* ��ʼ������ */
    delay_ms(100);                              /* ��ʱ100ms�ٳ�ʼ�����Ź�,LED0�ı仯"�ɼ�" */
    iwdg_init(IWDG_PRESCALER_64, 500);          /* Ԥ��Ƶ��Ϊ64,����ֵΪ500,���ʱ��ԼΪ1s */
    LED0(0);                                    /* ����LED0(���) */

    while (1)
    {
        if (key_scan(1) == WKUP_PRES)           /* ���WK_UP����,��ι��,֧������ */
        {
            iwdg_feed();                        /* ι�� */
        }

        delay_ms(10);
    }
}
