/**
 ****************************************************************************************************
 * @file        main.c
 * @author      正点原子团队(ALIENTEK)
 * @version     V1.0
 * @date        2022-09-06
 * @brief       USB U盘（HOST） 实验
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
#include "./BSP/LED/led.h"
#include "./BSP/LCD/lcd.h"
#include "./BSP/MPU/mpu.h"
#include "./BSP/LCD/ltdc.h"
#include "./BSP/NAND/ftl.h"
#include "./BSP/NAND/nand.h"
#include "./BSP/SDMMC/sdmmc_sdcard.h"
#include "./BSP/IIC/myiic.h"
#include "./PICTURE/piclib.h"
#include "./BSP/NORFLASH/norflash.h"
#include "./MALLOC/malloc.h"
#include "./FATFS/exfuns/exfuns.h"
#include "./BSP/PCF8574/pcf8574.h"
#include "./TEXT/text.h"
#include "./USMART/usmart.h"
#include "usbh_core.h"
#include "usbh_msc.h"



USBH_HandleTypeDef g_hUSBHost;              /* USB Host处理结构体 */

static void USBH_UserProcess(USBH_HandleTypeDef *phost, uint8_t id)
{
    uint32_t total, free;
    uint8_t res = 0;
    printf("id:%d\r\n", id);

    switch (id)
    {
        case HOST_USER_SELECT_CONFIGURATION:
            break;

        case HOST_USER_DISCONNECTION:
            f_mount(0, "3:", 1);                                    /* 卸载U盘 */
            text_show_string(30, 120, 200, 16, "设备连接中...", 16, 0, RED);
            LED1(1);
            lcd_fill(30, 160, 239, 220, WHITE);
            break;

        case HOST_USER_CLASS_ACTIVE:
            text_show_string(30, 120, 200, 16, "设备连接成功!", 16, 0, RED);
            f_mount(fs[3], "3:", 1);                                /* 重新挂载U盘 */
            LED1(0);
            res = exfuns_get_free((uint8_t *)"3:", &total, &free);
            if (res == 0)
            {
                lcd_show_string(30, 160, 200, 16, 16, "FATFS OK!", BLUE);
                lcd_show_string(30, 180, 200, 16, 16, "U Disk Total Size:     MB", BLUE);
                lcd_show_string(30, 200, 200, 16, 16, "U Disk  Free Size:     MB", BLUE);
                lcd_show_num(174, 180, total >> 10, 5, 16, BLUE);   /* 显示U盘总容量 MB */
                lcd_show_num(174, 200, free >> 10, 5, 16, BLUE);
            }
            else
            {
                printf("U盘存储空间获取失败\r\n");
            }
            break;

        case HOST_USER_CONNECTION:
            break;

        default:
            break;
    }
}

int main(void)
{
    uint8_t t = 0;

    sys_cache_enable();                         /* 打开L1-Cache */
    HAL_Init();                                 /* 初始化HAL库 */
    sys_stm32_clock_init(160, 5, 2, 4);         /* 设置时钟, 400Mhz */
    delay_init(400);                            /* 延时初始化 */
    usart_init(115200);                         /* 串口初始化 */
    usmart_init(200);                           /* 初始化USMART */
    mpu_memory_protection();                    /* 保护相关存储区域 */
    led_init();                                 /* 初始化LED */
    sdram_init();                               /* 初始化SDRAM */
    lcd_init();                                 /* 初始化LCD */
    norflash_init();                            /* 初始化norflash */
    piclib_init();                              /* 初始化画图 */
    pcf8574_init();                             /* 初始化PCF8574 */

    my_mem_init(SRAMIN);                        /* 初始化内部内存池(AXI) */
    my_mem_init(SRAMEX);                        /* 初始化外部内存池(SDRAM) */
    my_mem_init(SRAM12);                        /* 初始化SRAM12内存池(SRAM1+SRAM2) */
    my_mem_init(SRAM4);                         /* 初始化SRAM4内存池(SRAM4) */
    my_mem_init(SRAMDTCM);                      /* 初始化DTCM内存池(DTCM) */
    my_mem_init(SRAMITCM);                      /* 初始化ITCM内存池(ITCM) */

    exfuns_init();                              /* 为fatfs相关变量申请内存 */
    f_mount(fs[0], "0:", 1);                    /* 挂载SD卡 */
    f_mount(fs[1], "1:", 1);                    /* 挂载FLASH */
    f_mount(fs[2], "2:", 1);                    /* 挂载NAND FLASH */

    while (fonts_init())                        /* 检查字库 */
    {
        lcd_show_string(30, 50, 200, 16, 16, "Font Error!", RED);
        delay_ms(200);
        lcd_fill(30, 50, 240, 66, WHITE);       /* 清除显示 */
        delay_ms(200);
    }
    
    text_show_string(30, 50, 200, 16, "STM32", 16, 0, RED);
    text_show_string(30, 70, 200, 16, "USB U盘 实验", 16, 0, RED);
    text_show_string(30, 90, 200, 16, "ATOM@ALIENTEK", 16, 0, RED);
    text_show_string(30, 120, 200, 16, "设备连接中...", 16, 0, RED);

    USBH_Init(&g_hUSBHost, USBH_UserProcess, 0);
    USBH_RegisterClass(&g_hUSBHost, USBH_MSC_CLASS);
    USBH_Start(&g_hUSBHost);
    HAL_PWREx_EnableUSBVoltageDetector();

    while (1)
    {
        USBH_Process(&g_hUSBHost);
        delay_ms(10);
        t++;

        if (t == 50)
        {
            t = 0;
            LED0_TOGGLE();
        }
    }
}

