/**
 ****************************************************************************************************
 * @file        main.c
 * @author      ����ԭ���Ŷ�(ALIENTEK)
 * @version     V1.0
 * @date        2023-03-21
 * @brief       ͨ�ö�ʱ��������� ʵ��
 * @license     Copyright (c) 2022-2032, ������������ӿƼ����޹�˾
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
#include "./BSP/TIMER/gtim.h"


int main(void)
{
    uint32_t curcnt = 0;
    uint32_t oldcnt = 0;
    uint8_t key = 0;
    uint8_t t = 0;
    
    sys_stm32_clock_init(160, 5, 2, 4);         /* ����ʱ��, 400Mhz */
    delay_init(400);                            /* ��ʱ��ʼ�� */
    usart_init(100, 115200);                    /* ��ʼ��USART */
    led_init();                                 /* ��ʼ��LED */
    key_init();                                 /* ��ʼ������ */
    gtim_timx_cnt_chy_init(0);                  /* ��ʱ��������ʼ��, ����Ƶ */
    gtim_timx_cnt_chy_restart();                /* �������� */
    
    while (1)
    {
        key = key_scan(0);                      /* ɨ�谴�� */

        if (key == KEY0_PRES)                   /* KEY0��������,�������� */
        {
            printf("key0 press \r\n");
            gtim_timx_cnt_chy_restart();        /* ������������ */
        }

        curcnt = gtim_timx_cnt_chy_get_count(); /* ��ȡ����ֵ */

        if (oldcnt != curcnt)
        {
            oldcnt = curcnt;
            printf("CNT:%d\r\n", oldcnt);       /* ��ӡ������� */
        }

        t++;

        if (t > 40)                             /* 200ms����һ�� */
        {
            t = 0;
            LED0_TOGGLE();                      /* LED0��˸ ,��ʾ�������� */
        }

        delay_ms(10);
    }
}































