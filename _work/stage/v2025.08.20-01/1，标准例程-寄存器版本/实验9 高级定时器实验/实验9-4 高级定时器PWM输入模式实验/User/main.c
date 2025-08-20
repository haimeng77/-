/**
 ****************************************************************************************************
 * @file        main.c
 * @author      ����ԭ���Ŷ�(ALIENTEK)
 * @version     V1.0
 * @date        2023-03-21
 * @brief       �߼���ʱ��PWM����ģʽ ʵ��
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
#include "./BSP/TIMER/atim.h"
#include "./BSP/TIMER/gtim.h"


extern uint16_t g_timxchy_pwmin_psc;            /* PWM����״̬ */
extern uint16_t g_timxchy_pwmin_sta;            /* PWM����״̬ */
extern uint32_t g_timxchy_pwmin_hval;           /* PWM�ĸߵ�ƽ���� */
extern uint32_t g_timxchy_pwmin_cval;           /* PWM�����ڿ�� */

int main(void)
{
    uint8_t t = 0;
    double ht, ct, f, tpsc;
        
    sys_stm32_clock_init(160, 5, 2, 4);         /* ����ʱ��, 400Mhz */
    delay_init(400);                            /* ��ʱ��ʼ�� */
    usart_init(100, 115200);                    /* ��ʼ��USART */
    led_init();                                 /* ��ʼ��LED */ 

    gtim_timx_pwm_chy_init(10 - 1, 200 - 1);    /* 1Mhz�ļ���Ƶ��, 100Khz PWM */
    atim_timx_pwmin_chy_init();                 /* ��ʼ��PWM���벶�� */

    GTIM_TIMX_PWM_CHY_CCRX = 2;                 /* �͵�ƽ���2,�ߵ�ƽ���8 */

    while (1)
    {
        delay_ms(10);
        t++;

        if (t >= 20)                                                /* ÿ200ms���һ�ν��,����˸LED1,��ʾ�������� */
        {
            if (g_timxchy_pwmin_sta)                                /* ������һ������ */
            {
                printf("\r\n");                                     /* �����,����һ�� */
                printf("PWM PSC  :%d\r\n", g_timxchy_pwmin_psc);    /* ��ӡ��Ƶϵ�� */
                printf("PWM Hight:%d\r\n", g_timxchy_pwmin_hval);   /* ��ӡ�ߵ�ƽ���� */
                printf("PWM Cycle:%d\r\n", g_timxchy_pwmin_cval);   /* ��ӡ���� */
                tpsc = ((double)g_timxchy_pwmin_psc + 1) / 200;     /* �õ�PWM����ʱ������ʱ�� */
                ht = g_timxchy_pwmin_hval * tpsc;                   /* ����ߵ�ƽʱ�� */
                ct = g_timxchy_pwmin_cval * tpsc;                   /* �������ڳ��� */
                f = (1 / ct) * 1000000;                             /* ����Ƶ�� */
                printf("PWM Hight time:%.3fus\r\n", ht);            /* ��ӡ�ߵ�ƽ������ */
                printf("PWM Cycle time:%.3fus\r\n", ct);            /* ��ӡ����ʱ�䳤�� */
                printf("PWM Frequency :%.3fHz\r\n", f);             /* ��ӡƵ�� */
                atim_timx_pwmin_chy_restart();                      /* ����PWM������ */
            }

            LED1_TOGGLE();                                          /* DS1��˸ */
            t = 0;
         }
    }
}






















