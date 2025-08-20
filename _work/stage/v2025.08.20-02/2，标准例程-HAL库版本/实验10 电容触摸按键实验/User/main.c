/**
 ****************************************************************************************************
 * @file        main.c
 * @author      ����ԭ���Ŷ�(ALIENTEK)
 * @version     V1.0
 * @date        2022-09-06
 * @brief       ���ݴ������� ʵ��
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
#include "./BSP/TPAD/tpad.h"


int main(void)
{
    uint8_t t = 1;

    sys_cache_enable();                   /* ��L1-Cache */
    HAL_Init();                           /* ��ʼ��HAL�� */
    sys_stm32_clock_init(160, 5, 2, 4);   /* ����ʱ��, 400Mhz */
    delay_init(400);                      /* ��ʱ��ʼ�� */
    usart_init(115200);                   /* ��ʼ��USART */
    led_init();                           /* ��ʼ��LED */
    tpad_init(2);                         /* ��ʼ���������� */

    while (1)
    {
        if (tpad_scan(0))                 /* �ɹ�������һ��������(�˺���ִ��ʱ������15ms) */
        {
            LED1_TOGGLE();                /* LED1��ת */
        }

        t++;
        if (t == 15)
        {
            t = 0;
            LED0_TOGGLE();                /* LED0��ת */
        }

        delay_ms(10);
    }
}
