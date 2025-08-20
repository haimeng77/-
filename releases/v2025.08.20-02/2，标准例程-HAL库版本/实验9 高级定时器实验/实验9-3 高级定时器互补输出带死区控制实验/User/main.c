/**
 ****************************************************************************************************
 * @file        main.c
 * @author      ����ԭ���Ŷ�(ALIENTEK)
 * @version     V1.0
 * @date        2022-09-06
 * @brief       �߼���ʱ������������������� ʵ��
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
#include "./BSP/TIMER/atim.h"


int main(void)
{
    uint8_t t = 0;

    sys_cache_enable();                             /* ��L1-Cache */
    HAL_Init();                                     /* ��ʼ��HAL�� */
    sys_stm32_clock_init(160, 5, 2, 4);             /* ����ʱ��, 400Mhz */
    delay_init(400);                                /* ��ʱ��ʼ�� */
    usart_init(115200);                             /* ��ʼ��USART */
    led_init();                                     /* ��ʼ��LED */
    atim_timx_cplm_pwm_init(1000 - 1, 20 - 1);      /* 10Mhz�ļ���Ƶ�� 10Khz������ */
    atim_timx_cplm_pwm_set(300, 100);               /* ռ�ձ�Ϊ7:3������70%������ͨ����30%��, ����ʱ�� 100 * tDTS */

    while (1)
    {
        t++;
        delay_ms(10);

        if (t > 50)                                 /* ����LED0��˸, ��ʾ��������״̬ */
        {
            t = 0;
            LED0_TOGGLE();
        }
    }
}
