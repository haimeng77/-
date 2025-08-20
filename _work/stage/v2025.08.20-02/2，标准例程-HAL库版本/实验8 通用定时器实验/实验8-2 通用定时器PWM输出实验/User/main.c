/**
 ****************************************************************************************************
 * @file        main.c
 * @author      ����ԭ���Ŷ�(ALIENTEK)
 * @version     V1.0
 * @date        2022-09-06
 * @brief       ͨ�ö�ʱ��PWM��� ʵ��
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
#include "./BSP/TIMER/gtim.h"


extern TIM_HandleTypeDef g_timx_pwm_chy_handle; /* ��ʱ��x��� */

int main(void)
{
    uint16_t ledrpwmval = 0;
    uint8_t dir = 1;

    sys_cache_enable();                         /* ��L1-Cache */
    HAL_Init();                                 /* ��ʼ��HAL�� */
    sys_stm32_clock_init(160, 5, 2, 4);         /* ����ʱ��, 400Mhz */
    delay_init(400);                            /* ��ʱ��ʼ�� */
    usart_init(115200);                         /* ��ʼ��USART */
    led_init();                                 /* ��ʼ��LED */
    gtim_timx_pwm_chy_init(500 - 1, 200 - 1);   /* ����Ƶ�� = 200M/200 = 1MHz��PWMƵ�� = 1M/500 = 2kHz */

    while (1)
    {
        delay_ms(10);

        if (dir)
        {
            ledrpwmval++;                       /* dir==1��ledrpwmval���� */
        }
        else 
        {
            ledrpwmval--;                       /* dir==0��ledrpwmval�ݼ� */
        }

        if (ledrpwmval > 300)
        {
            dir = 0;                            /* ledrpwmval����300�󣬷���Ϊ�ݼ� */
        }

        if (ledrpwmval == 0)
        {
            dir = 1;                            /* ledrpwmval�ݼ���0�󣬷����Ϊ���� */
        }

        /* �޸ıȽ�ֵ����ռ�ձ� */
        __HAL_TIM_SET_COMPARE(&g_timx_pwm_chy_handle, GTIM_TIMX_PWM_CHY, ledrpwmval);
    }
}
