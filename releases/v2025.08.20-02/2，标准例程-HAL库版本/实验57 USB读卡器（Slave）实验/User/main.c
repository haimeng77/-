/**
 ****************************************************************************************************
 * @file        main.c
 * @author      正点原子团队(ALIENTEK)
 * @version     V1.0
 * @date        2022-09-06
 * @brief       USB读卡器（SLAVE） 实验
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
#include "./BSP/KEY/key.h"
#include "./BSP/MPU/mpu.h"
#include "./BSP/LCD/ltdc.h"
#include "./BSP/NAND/ftl.h"
#include "./BSP/NAND/nand.h"
#include "./BSP/SDMMC/sdmmc_sdcard.h"
#include "./BSP/IIC/myiic.h"
#include "./BSP/NORFLASH/norflash.h"
#include "./MALLOC/malloc.h"
#include "./TEXT/text.h"
#include "usbd_core.h"
#include "usbd_desc.h"
#include "usbd_msc.h"
#include "usbd_storage.h"
#include "usbd_conf.h"


USBD_HandleTypeDef USBD_Device;                /* USB Device处理结构体 */
extern PCD_HandleTypeDef g_pcd_usb_otg_fs;     /* PCD_USB_OTG_FS 定义*/
extern volatile uint8_t g_usb_state_reg;       /* USB状态 */
extern volatile uint8_t g_device_state;        /* USB连接 情况 */

int main(void)
{
    uint8_t offline_cnt = 0;
    uint8_t tct = 0;
    uint8_t usb_sta;
    uint8_t device_sta;
    uint64_t card_capacity;                    /* SD卡容量 */

    sys_cache_enable();                        /* 打开L1-Cache */
    mpu_memory_protection();                   /* 保护相关存储区域 */
    HAL_Init();                                /* 初始化HAL库 */
    sys_stm32_clock_init(160, 5, 2, 4);        /* 设置时钟, 400Mhz */
    delay_init(400);                           /* 延时初始化 */
    usart_init(115200);                        /* 串口初始化 */
    led_init();                                /* 初始化LED */
    key_init();                                /* 初始化KEY */
    sdram_init();                              /* 初始化SDRAM */
    lcd_init();                                /* 初始化LCD */
    norflash_init();                           /* 初始化norflash */

    my_mem_init(SRAMIN);                       /* 初始化内部内存池(AXI) */
    my_mem_init(SRAMEX);                       /* 初始化外部内存池(SDRAM) */
    my_mem_init(SRAMDTCM);                     /* 初始化DTCM内存池(DTCM) */

    lcd_show_string(30, 50, 200, 16, 16, "STM32", RED);
    lcd_show_string(30, 70, 200, 16, 16, "USB Card Reader TEST", RED);
    lcd_show_string(30, 90, 200, 16, 16, "ATOM@ALIENTEK", RED);

    if (sd_init())                                                                  /* 初始化SD卡 */
    {
        lcd_show_string(30, 130, 200, 16, 16, "SD Card Error!", RED);               /* 检测SD卡错误 */
    }
    else                                                                            /* SD 卡正常 */
    {
        lcd_show_string(30, 130, 200, 16, 16, "SD Card Size:     MB", RED);
        card_capacity = (uint64_t)(g_sd_card_info_handle.LogBlockNbr) * (uint64_t)(g_sd_card_info_handle.LogBlockSize);     /* 计算SD卡容量 */
        lcd_show_num(134, 130, card_capacity >> 20, 5, 16, RED);                    /* 显示SD卡容量 */
    }
    
    if (norflash_read_id() != W25Q256)
    {
        lcd_show_string(30, 130, 200, 16, 16, "NorFlash Error!", RED); 
    }
    else                                                                            /* SPI FLASH 正常 */
    {
        lcd_show_string(30, 150, 200, 16, 16, "SPI FLASH Size:25MB", RED);
    }
    if (ftl_init())                                                                 /* 检测NAND FLASH错误 */
    {
        lcd_show_string(30, 170, 200, 16, 16, "NAND Error!", RED);
    }
    else                                                                            /* NAND FLASH正常 */
    {
        lcd_show_string(30, 170, 200, 16, 16, "NAND Flash Size:    MB", RED);
        lcd_show_num(158, 170, nand_dev.valid_blocknum * nand_dev.block_pagenum * nand_dev.page_mainsize >> 20, 4, 16, RED); /* 显示SD卡容量 */
    }

    USBD_Init(&USBD_Device, &MSC_Desc, DEVICE_FS);                                  /* 初始化USB */
    USBD_RegisterClass(&USBD_Device, &USBD_MSC);                                    /* 添加类 */
    USBD_MSC_RegisterStorage(&USBD_Device, &USBD_Storage_Interface_fops_FS);        /* 为MSC类添加回调函数 */
    USBD_Start(&USBD_Device);                                                       /* 开启USB */
    delay_ms(1800);

    while (1)
    {
        delay_ms(1);

        if (usb_sta != g_usb_state_reg)                                             /* 状态改变了 */
        {
            lcd_fill(30, 210, 240, 210 + 16, WHITE);                                /* 清除显示 */

            if (g_usb_state_reg & 0x01)                                             /* 正在写 */
            {
                LED1(0);
                lcd_show_string(30, 210, 200, 16, 16, "USB Writing...", RED);       /* 提示USB正在写入数据 */
            }

            if (g_usb_state_reg & 0x02)                                             /* 正在读 */
            {
                LED1(0);
                lcd_show_string(30, 210, 200, 16, 16, "USB Reading...", RED);       /* 提示USB正在读出数据 */
            }

            if (g_usb_state_reg & 0x04)
            {
                lcd_show_string(30, 230, 200, 16, 16, "USB Write Err ", RED);       /* 提示写入错误 */
            }
            else
            {
                lcd_fill(30, 230, 240, 230 + 16, WHITE);                            /* 清除显示 */
            }
            
            if (g_usb_state_reg & 0x08)
            {
                lcd_show_string(30, 250, 200, 16, 16, "USB Read  Err ", RED);       /* 提示读出错误 */
            }
            else
            {
                lcd_fill(30, 250, 240, 250 + 16, WHITE);                            /* 清除显示 */
            }
            
            usb_sta = g_usb_state_reg;                                              /* 记录最后的状态 */
        }

        if (device_sta != g_device_state)
        {
            if (g_device_state == 1)
            {
                lcd_show_string(30, 190, 200, 16, 16, "USB Connected    ", RED);    /* 提示USB连接已经建立 */
            }
            else
            {
                lcd_show_string(30, 190, 200, 16, 16, "USB DisConnected ", RED);    /* 提示USB被拔出了 */
            }

            device_sta = g_device_state;
        }

        tct++;

        if (tct == 200)
        {
            tct = 0;
            LED1(1);                        /* 关闭 LED1 */
            LED0_TOGGLE();                  /* LED0 闪烁 */

            if (g_usb_state_reg & 0x10)
            {
                offline_cnt = 0;            /* USB连接了,则清除offline计数器 */
                g_device_state = 1;
            }
            else                            /* 没有得到轮询 */
            {
                offline_cnt++;

                if (offline_cnt > 10)
                {
                    g_device_state = 0;     /* 2s内没收到在线标记,代表USB被拔出了 */
                }
            }

            g_usb_state_reg = 0;
        }
    } 
}

