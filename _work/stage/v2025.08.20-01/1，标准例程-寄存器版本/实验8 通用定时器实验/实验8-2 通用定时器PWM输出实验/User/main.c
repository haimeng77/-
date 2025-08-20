/**
 ****************************************************************************************************
 * @file        main.c
 * @author      正点原子团队(ALIENTEK)
 * @version     V1.0
 * @date        2023-03-21
 * @brief       通用定时器PWM输出 实验
 * @license     Copyright (c) 2022-2032, 广州市星翼电子科技有限公司
 ****************************************************************************************************
 * @attention
 *
 * 实验平台:正点原子 阿波罗 H743开发板
 * 在线视频:www.yuanzige.com
 * 技术论坛:www.openedv.com
 * 公司网址:www.alientek.com
 * 购买地址:openedv.taobao.com
 *
 ****************************************************************************************************
 */

#include "./SYSTEM/sys/sys.h"
#include "./SYSTEM/usart/usart.h"
#include "./SYSTEM/delay/delay.h"
#include "./BSP/LED/led.h"
#include "./BSP/TIMER/gtim.h"


int main(void)
{
    uint16_t ledrpwmval = 0;
    uint8_t dir = 1;
    
    sys_stm32_clock_init(160, 5, 2, 4);      /* 设置时钟, 400Mhz */
    delay_init(400);                         /* 延时初始化 */
    usart_init(100, 115200);                 /* 初始化USART */
    led_init();                              /* 初始化LED */
    gtim_timx_pwm_chy_init(500 - 1, 200 - 1);/* 1Mhz的计数频率,2Khz的PWM. */

    while (1)
    {
        delay_ms(10);

        if (dir)
        {
            ledrpwmval++;
        }
        else 
        {
            ledrpwmval--;
        }

        if (ledrpwmval > 300)
        {
            dir = 0;
        }
        
        if (ledrpwmval == 0)
        {
            dir = 1;
        }

        GTIM_TIMX_PWM_CHY_CCRX = ledrpwmval;
    }
}


















