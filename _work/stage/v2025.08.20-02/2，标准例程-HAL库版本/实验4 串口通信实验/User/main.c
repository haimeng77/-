/**
 ****************************************************************************************************
 * @file        main.c
 * @author      ����ԭ���Ŷ�(ALIENTEK)
 * @version     V1.0
 * @date        2022-09-6
 * @brief       ����ͨ�� ʵ��
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


int main(void)
{
    uint8_t len;
    uint16_t times = 0;

    sys_cache_enable();                         /* ��L1-Cache */
    HAL_Init();                                 /* ��ʼ��HAL�� */
    sys_stm32_clock_init(160, 5, 2, 4);         /* ����ʱ��, 400Mhz */
    delay_init(400);                            /* ��ʱ��ʼ�� */
    usart_init(115200);                         /* ��ʼ��USART */
    led_init();                                 /* ��ʼ��LED */

    while (1)
    {
        if (g_usart_rx_sta & 0x8000)                                                    /* ���յ�������? */
        {
            len = g_usart_rx_sta & 0x3fff;                                              /* �õ��˴ν��յ������ݳ��� */
            printf("\r\n�����͵���ϢΪ:\r\n");

            HAL_UART_Transmit(&g_uart1_handle, (uint8_t *)g_usart_rx_buf, len, 1000);   /* ���ͽ��յ������� */
            while(__HAL_UART_GET_FLAG(&g_uart1_handle, UART_FLAG_TC) != SET);           /* �ȴ����ͽ��� */
            printf("\r\n\r\n");                                                         /* ���뻻�� */
            g_usart_rx_sta = 0;
        }
        else
        {
            times++;

            if (times % 5000 == 0)
            {
                printf("\r\n����ԭ�� STM32������ ����ʵ��\r\n");
                printf("����ԭ��@ALIENTEK\r\n\r\n\r\n");
            }

            if (times % 200 == 0)
            {
                printf("����������,�Իس�������\r\n");
            }

            if (times % 30  == 0) 
            {
                LED0_TOGGLE();                   /* ��˸LED,��ʾϵͳ�������� */
            }

            delay_ms(10);
        }
    }
}
