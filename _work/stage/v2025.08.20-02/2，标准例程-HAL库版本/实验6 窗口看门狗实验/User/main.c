/**
 ****************************************************************************************************
 * @file        main.c
 * @author      ����ԭ���Ŷ�(ALIENTEK)
 * @version     V1.0
 * @date        2022-09-06
 * @brief       ���ڿ��Ź� ʵ��
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
#include "./BSP/WDG/wdg.h"


int main(void)
{
    sys_cache_enable();                         /* ��L1-Cache */
    HAL_Init();                                 /* ��ʼ��HAL�� */
    sys_stm32_clock_init(160, 5, 2, 4);         /* ����ʱ��, 400Mhz */
    delay_init(400);                            /* ��ʱ��ʼ�� */
    usart_init(115200);                         /* ��ʼ��USART */
    led_init();                                 /* ��ʼ��LED */
    LED0(0);                                    /* ����LED0(���) */
    delay_ms(300);                              /* ��ʱ300ms�ٳ�ʼ�����Ź�,LED0�ı仯"�ɼ�" */
    wwdg_init(0X7F, 0X5F, WWDG_PRESCALER_16);   /* ������ֵΪ7f,���ڼĴ���Ϊ5f,��Ƶ��Ϊ16 */

    while (1)
    {
        LED0(1);                                /* �ر�LED0(���) */
    }
}
