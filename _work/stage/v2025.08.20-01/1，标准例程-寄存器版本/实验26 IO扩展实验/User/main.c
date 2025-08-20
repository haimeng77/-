/**
 ****************************************************************************************************
 * @file        main.c
 * @author      正点原子团队(ALIENTEK)
 * @version     V1.0
 * @date        2023-03-22
 * @brief       IO扩展实验 实验
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
#include "./BSP/MPU/mpu.h"
#include "./BSP/LCD/lcd.h"
#include "./BSP/SDRAM/sdram.h"
#include "./USMART/usmart.h"
#include "./BSP/PCF8574/pcf8574.h"


int main(void)
{
    uint8_t key;
    uint16_t i = 0;
    uint8_t beepsta = 1;
    
    sys_stm32_clock_init(160, 5, 2, 4);     /* 设置时钟, 400Mhz */
    delay_init(400);                        /* 延时初始化 */
    usart_init(100, 115200);                /* 初始化USART */
    usmart_dev.init(100);                   /* USMART初始化 */
    led_init();                             /* 初始化LED */
    mpu_memory_protection();                /* 保护相关存储区域 */
    sdram_init();                           /* 初始化SDRAM */
    lcd_init();                             /* 初始化LCD */
    key_init();                             /* 初始化按键 */
    
    lcd_show_string(30, 50, 200, 16, 16, "STM32", RED);
    lcd_show_string(30, 70, 200, 16, 16, "PCF8574 TEST", RED);
    lcd_show_string(30, 90, 200, 16, 16, "ATOM@ALIENTEK", RED);
    lcd_show_string(30, 110, 200, 16, 16, "KEY0:BEEP ON/OFF", RED);    /* 显示提示信息 */
    lcd_show_string(30, 130, 200, 16, 16, "EXIO:DS1 ON/OFF", RED);     /* 显示提示信息 */

    while (pcf8574_init()) /* 检测不到PCF8574 */
    {
        lcd_show_string(30, 170, 200, 16, 16, "PCF8574 Check Failed!", RED);
        delay_ms(500);
        lcd_show_string(30, 170, 200, 16, 16, "Please Check!      ", RED);
        delay_ms(500);
        LED0_TOGGLE();     /* 红灯闪烁 */
    }

    lcd_show_string(30, 170, 200, 16, 16, "PCF8574 Ready!", RED);

    while (1)
    {
        key = key_scan(0);

        if (key == KEY0_PRES)                      /* KEY0按下,读取字符串并显示 */
        {
            beepsta =! beepsta;                    /* 蜂鸣器状态取反 */
            pcf8574_write_bit(BEEP_IO, beepsta);   /* 控制蜂鸣器 */
        }

        if (PCF8574_INT == 0)                      /* PCF8574的中断低电平有效 */
        {
            key = pcf8574_read_bit(EX_IO);         /* 读取EXIO状态,同时清除PCF8574的中断输出(INT恢复高电平) */

            if (key == 0)
            {
                LED1_TOGGLE();                     /* LED1状态取反 */
            }
        }

        i++;
        delay_ms(10);

        if (i == 20)
        {
            LED0_TOGGLE();                        /* 红灯闪烁 */
            i = 0;
        }
    }
}











