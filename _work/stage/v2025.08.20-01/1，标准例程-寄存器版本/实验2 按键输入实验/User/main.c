/**
 ****************************************************************************************************
 * @file        main.c
 * @author      正点原子团队(ALIENTEK)
 * @version     V1.0
 * @date        2023-03-16
 * @brief       按键输入 实验
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
#include "./BSP/KEY/key.h"


int main(void)
{
    uint8_t key;
    
    sys_stm32_clock_init(160, 5, 2, 4);     /* 设置时钟, 400Mhz */
    delay_init(400);                        /* 延时初始化 */
    led_init();                             /* 初始化LED */
    key_init();                             /* 初始化按键 */
    LED0(0);                                /* 先点亮红灯 */
    
    while(1)
    {
        key = key_scan(0);                  /* 得到键值 */

        if (key)
        {
            switch (key)
            {
                case WKUP_PRES:             /* 控制LED0, LED1互斥点亮 */
                    LED1_TOGGLE();          /* LED1状态取反 */
                
                    /* LED0根据LED1的状态, 进行互斥设置 */
                    if (LED1_GPIO_PORT->ODR & LED1_GPIO_PIN)
                    {
                        LED0(0);
                    }
                    else 
                    {
                        LED0(1);
                    }
                    
                    break;

                case KEY0_PRES:             /* 同时控制LED0, LED1翻转 */
                    LED0_TOGGLE();          /* LED0状态取反 */
                    LED1_TOGGLE();          /* LED1状态取反 */
                    break;

                case KEY1_PRES:             /* 控制LED1(GREEN)翻转 */
                    LED1_TOGGLE();          /* LED1状态取反 */
                    break;

                case KEY2_PRES:             /* 控制LED0(RED)翻转 */
                    LED0_TOGGLE();          /* LED0状态取反 */
                    break;
            }
        }
        else
        {
            delay_ms(10);
        }
    }
}












