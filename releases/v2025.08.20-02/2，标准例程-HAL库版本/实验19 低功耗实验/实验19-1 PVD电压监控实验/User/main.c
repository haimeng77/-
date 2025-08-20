/**
 ****************************************************************************************************
 * @file        main.c
 * @author      正点原子团队(ALIENTEK)
 * @version     V1.0
 * @date        2022-09-06
 * @brief       PVD电压监控 实验
 * @license     Copyright (c) 2020-2032, 广州市星翼电子科技有限公司
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
#include "./BSP/LCD/lcd.h"
#include "./BSP/SDRAM/sdram.h"
#include "./USMART/usmart.h"
#include "./BSP/PWR/pwr.h"
#include "./BSP/MPU/mpu.h"


int main(void)
{
    uint8_t t = 0;

    sys_cache_enable();                      /* 打开L1-Cache */
    HAL_Init();                              /* 初始化HAL库 */
    sys_stm32_clock_init(160, 5, 2, 4);      /* 设置时钟, 400Mhz */
    delay_init(400);                         /* 延时初始化 */
    usart_init(115200);                      /* 串口初始化 */
    mpu_memory_protection();                 /* 保护相关存储区域 */
    led_init();                              /* 初始化LED */
    sdram_init();                            /* 初始化SDRAM */
    lcd_init();                              /* 初始化LCD */

    pwr_pvd_init(PWR_PVDLEVEL_6);            /* PVD 2.85V检测 */

    lcd_show_string(30, 50, 200, 16, 16, "STM32", RED);
    lcd_show_string(30, 70, 200, 16, 16, "PVD TEST", RED);
    lcd_show_string(30, 90, 200, 16, 16, "ATOM@ALIENTEK", RED);
    /* 默认LCD显示电压正常 */
    lcd_show_string(30, 110, 200, 16, 16, "PVD Voltage OK", BLUE);

    while (1)
    {
        if ((t % 20) == 0)
        {
            LED0_TOGGLE();                   /* 每200ms,翻转一次LED0 */
        }
   
        delay_ms(10);
        t++;
    }
}
