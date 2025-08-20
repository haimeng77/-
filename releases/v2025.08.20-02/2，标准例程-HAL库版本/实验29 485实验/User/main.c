/**
 ****************************************************************************************************
 * @file        main.c
 * @author      正点原子团队(ALIENTEK)
 * @version     V1.0
 * @date        2022-09-06
 * @brief       RS485 实验
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
#include "./BSP/SDRAM/sdram.h"
#include "./USMART/usmart.h"
#include "./BSP/IIC/myiic.h"
#include "./BSP/LED/led.h"
#include "./BSP/LCD/lcd.h"
#include "./BSP/KEY/key.h"
#include "./BSP/MPU/mpu.h"
#include "./BSP/RS485/rs485.h"


int main(void)
{
    uint8_t key;
    uint8_t i = 0, t = 0;
    uint8_t cnt = 0;
    uint8_t rs485buf[5]; 

    sys_cache_enable();                       /* 打开L1-Cache */
    HAL_Init();                               /* 初始化HAL库 */
    sys_stm32_clock_init(160, 5, 2, 4);       /* 设置时钟, 400Mhz */
    delay_init(400);                          /* 延时初始化 */
    usart_init(115200);                       /* 串口初始化 */
    usmart_init(200);                         /* 初始化USMART */
    mpu_memory_protection();                  /* 保护相关存储区域 */
    led_init();                               /* 初始化LED */
    sdram_init();                             /* 初始化SDRAM */
    lcd_init();                               /* 初始化LCD */
    key_init();                               /* 初始化KEY */
    rs485_init(9600);                         /* 初始化RS485 */

    lcd_show_string(30, 50, 200, 16, 16, "STM32", RED);
    lcd_show_string(30, 70, 200, 16, 16, "RS485 TEST", RED);
    lcd_show_string(30, 90, 200, 16, 16, "ATOM@ALIENTEK", RED);
    lcd_show_string(30, 110, 200, 16, 16, "KEY0:Send", RED);                /* 显示提示信息 */

    lcd_show_string(30, 150, 200, 16, 16, "Count:", BLUE);
    lcd_show_string(30, 170, 200, 16, 16, "Send Data:", BLUE);
    lcd_show_string(30, 210, 200, 16, 16, "Receive Data:", BLUE);

    while (1)
    {
        key = key_scan(0);

        if (key == KEY0_PRES)                                               /* KEY0按下,发送一次数据 */
        {
            for (i = 0; i < 5; i++)
            {
                rs485buf[i] = cnt + i;                                      /* 填充发送缓冲区 */
                lcd_show_num(30 + i * 32, 190, rs485buf[i], 3, 16, 0X80);   /* 显示数据 */
            }
            rs485_send_data(rs485buf, 5);       /* 发送5个字节 */
        }

        rs485_receive_data(rs485buf, &key);

        if (key)                                /* 接收到有数据 */
        {
            if (key > 5)
            {
                key = 5;                        /* 最大是5个数据. */
            }
            for (i = 0; i < key; i++)
            {
                lcd_show_num(30 + i * 32, 230, rs485buf[i], 3, 16, 0X80);   /* 显示数据 */
            }
        }

        t++; 
        delay_ms(10);

        if (t == 20)
        {
            LED0_TOGGLE();                      /* 提示系统正在运行 */
            t = 0; 
            cnt++;
            lcd_show_num(30 + 48, 150, cnt, 3, 16, 0X80);                   /* 显示数据 */
        }
    }
}
