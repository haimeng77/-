/**
 ****************************************************************************************************
 * @file        main.c
 * @author      正点原子团队(ALIENTEK)
 * @version     V1.0
 * @date        2023-03-21
 * @brief       SDRAM 实验
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


#if !(__ARMCC_VERSION >= 6010050)                             /* 不是AC6编译器，即使用AC5编译器时 */

uint16_t testsdram[250000] __attribute__((at(0XC0000000)));   /* 测试用数组 */

#else      /* 使用AC6编译器时 */

uint16_t testsdram[250000] __attribute__((section(".bss.ARM.__at_0xC0000000")));

#endif

/**
 * @brief       SDRAM内存测试
 * @param       x,y     : 起点坐标
 * @retval      无
*/
void sdram_test(uint16_t x, uint16_t y)
{  
    uint32_t i = 0;
    uint32_t temp = 0;
    uint32_t sval = 0;       /* 在地址0读到的数据 */
    lcd_show_string(x, y, 180, y + 16, 16, "Ex Memory Test:    0KB ", RED);

    /* 每隔16K字节,写入一个数据,总共写入2048个数据,刚好是32M字节 */
    for (i = 0; i < 32 * 1024 * 1024; i += 16 * 1024)
    {
        *(__IO uint32_t *)(BANK5_SDRAM_ADDR + i) = temp; 
        temp++;
    }

    /* 依次读出之前写入的数据,进行校验 */
    for (i = 0; i < 32 * 1024 * 1024; i += 16 * 1024) 
    {
        temp =*(__IO uint32_t*)(BANK5_SDRAM_ADDR + i);

        if (i == 0)
        {
            sval = temp;
        }
        else if (temp <= sval)
        {
            break;  /* 后面读出的数据一定要比第一次读到的数据大 */
        }

        lcd_show_num(x + 15 * 8, y, (uint16_t)(temp - sval + 1) * 16, 5, 16, BLUE);     /* 显示内存容量 */
        printf("SDRAM Capacity:%dKB\r\n", (uint16_t)(temp - sval + 1) * 16);            /* 打印SDRAM容量 */
    }
}

int main(void)
{
    uint8_t key;
    uint8_t i = 0;
    uint32_t ts = 0;
    
    sys_stm32_clock_init(160, 5, 2, 4);     /* 设置时钟, 400Mhz */
    delay_init(400);                        /* 延时初始化 */
    usart_init(100, 115200);                /* 初始化USART */
    led_init();                             /* 初始化LED */ 
    mpu_memory_protection();                /* 保护相关存储区域 */
    lcd_init();                             /* 初始化LCD */
    key_init();                             /* 初始化KEY */
    sdram_init();                           /* 初始化SDRAM */

    lcd_show_string(30, 50, 200, 16, 16, "STM32", RED);
    lcd_show_string(30, 70, 200, 16, 16, "SDRAM TEST", RED);
    lcd_show_string(30, 90, 200, 16, 16, "ATOM@ALIENTEK", RED);
    lcd_show_string(30, 110, 200, 16, 16, "KEY0:Test SDRAM", RED);
    lcd_show_string(30, 130, 200, 16, 16, "KEY1:Test DATA", RED);

    g_point_color = BLUE;                   /* 蓝色画笔 */
    
    for (ts = 0; ts < 250000; ts++)
    {
        testsdram[ ts ] = ts;               /* 预存测试数据 */
    }

    while (1)
    {
        key = key_scan(0);                  /* 不支持连按 */

        if (key == KEY0_PRES)
        {
            sdram_test(30, 170);            /* 测试SRAM容量 */
        }
        else if (key == KEY1_PRES)          /* 打印预存测试数据 */
        {
            for (ts = 0; ts < 250000; ts++)
            {
                lcd_show_num(30, 190, testsdram[ts], 6, 16, BLUE);  /* 显示测试数据 */
                printf("testsdram[%d]:%d\r\n", ts, testsdram[ts]);
            }
        }
        else 
        {
            delay_ms(10);
        }

        i++;

        if (i == 20)
        {
            i = 0;
            LED0_TOGGLE();       /* DS0闪烁 */
        }
    }
}

