/**
 ****************************************************************************************************
 * @file        main.c
 * @author      正点原子团队(ALIENTEK)
 * @version     V1.0
 * @date        2023-03-21
 * @brief       高级定时器互补输出带死区控制 实验
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
#include "./BSP/TIMER/atim.h"


int main(void)
{
    uint8_t t = 0;
    
    sys_stm32_clock_init(160, 5, 2, 4);         /* 设置时钟, 400Mhz */
    delay_init(400);                            /* 延时初始化 */
    usart_init(100, 115200);                    /* 初始化USART */
    led_init();                                 /* 初始化LED */ 
    
    atim_timx_cplm_pwm_init(1000 - 1, 200 - 1); /* 1Mhz的计数频率 1Khz的周期. */
    atim_timx_cplm_pwm_set(300, 100);           /* 占空比:7:3, 死区时间 100 * tDTS */

    while (1)
    {
        t++;
        delay_ms(10);

        if (t > 50)                             /* 控制LED0闪烁, 提示程序运行状态 */
        {
            t = 0;
            LED0_TOGGLE();
        }
    }
}




















