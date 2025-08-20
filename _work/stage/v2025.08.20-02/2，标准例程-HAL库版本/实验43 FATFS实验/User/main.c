/**
 ****************************************************************************************************
 * @file        main.c
 * @author      ����ԭ���Ŷ�(ALIENTEK)
 * @version     V1.0
 * @date        2022-09-06
 * @brief       FATFS ʵ��
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
#include "./BSP/SDRAM/sdram.h"
#include "./USMART/usmart.h"
#include "./BSP/LED/led.h"
#include "./BSP/LCD/lcd.h"
#include "./BSP/MPU/mpu.h"
#include "./BSP/LCD/ltdc.h"
#include "./MALLOC/malloc.h"
#include "./BSP/NAND/ftl.h"
#include "./BSP/NAND/nand.h"
#include "./FATFS/source/ff.h"
#include "./FATFS/exfuns/exfuns.h"
#include "./BSP/NORFLASH/norflash.h"
#include "./BSP/SDMMC/sdmmc_sdcard.h"


int main(void)
{
    uint32_t total, free;
    uint8_t t = 0;
    uint8_t res = 0;

    sys_cache_enable();                      /* ��L1-Cache */
    HAL_Init();                              /* ��ʼ��HAL�� */
    sys_stm32_clock_init(160, 5, 2, 4);      /* ����ʱ��, 400Mhz */
    delay_init(400);                         /* ��ʱ��ʼ�� */
    usart_init(115200);                      /* ���ڳ�ʼ�� */
    usmart_init(200);                        /* ��ʼ��USMART */
    mpu_memory_protection();                 /* ������ش洢���� */
    led_init();                              /* ��ʼ��LED */
    sdram_init();                            /* ��ʼ��SDRAM */
    lcd_init();                              /* ��ʼ��LCD */

    my_mem_init(SRAMIN);                     /* ��ʼ���ڲ��ڴ��(AXI) */
    my_mem_init(SRAMEX);                     /* ��ʼ���ⲿ�ڴ��(SDRAM) */
    my_mem_init(SRAM12);                     /* ��ʼ��SRAM12�ڴ��(SRAM1+SRAM2) */
    my_mem_init(SRAM4);                      /* ��ʼ��SRAM4�ڴ��(SRAM4) */
    my_mem_init(SRAMDTCM);                   /* ��ʼ��DTCM�ڴ��(DTCM) */
    my_mem_init(SRAMITCM);                   /* ��ʼ��ITCM�ڴ��(ITCM) */
    
    lcd_show_string(30, 50, 200, 16, 16, "STM32", RED);
    lcd_show_string(30, 70, 200, 16, 16, "FATFS TEST", RED);
    lcd_show_string(30, 90, 200, 16, 16, "ATOM@ALIENTEK", RED);
    lcd_show_string(30, 110, 200, 16, 16, "Use USMART for test", RED);

    while (sd_init())                       /* ��ⲻ��SD�� */
    {
        lcd_show_string(30, 130, 200, 16, 16, "SD Card Error!", RED);
        delay_ms(500);
        lcd_show_string(30, 130, 200, 16, 16, "Please Check! ", RED);
        delay_ms(500);
        LED0_TOGGLE();                      /* �����˸ */
    }

    ftl_init();
    exfuns_init();                          /* Ϊfatfs��ر��������ڴ� */
    f_mount(fs[0], "0:", 1);                /* ����SD�� */

    res = f_mount(fs[1], "1:", 1);          /* ����FLASH. */
    if (res == 0X0D)                        /* FLASH����,FAT�ļ�ϵͳ����,���¸�ʽ��FLASH */
    {
        lcd_show_string(30, 150, 200, 16, 16, "Flash Disk Formatting...", RED);     /* ��ʽ��FLASH */
        res = f_mkfs("1:", 0, 0, FF_MAX_SS);                                        /* ��ʽ��FLASH,1:,�̷�;0,ʹ��Ĭ�ϸ�ʽ������ */

        if (res == 0)
        {
            f_setlabel((const TCHAR *)"1:ALIENTEK");                                /* ����Flash���̵�����Ϊ��ALIENTEK */
            lcd_show_string(30, 150, 200, 16, 16, "Flash Disk Format Finish", RED); /* ��ʽ����� */
        }
        else
        {
            lcd_show_string(30, 150, 200, 16, 16, "Flash Disk Format Error ", RED); /* ��ʽ��ʧ�� */
        }

        delay_ms(1000);
    }

    res = f_mount(fs[2], "2:", 1);                                                  /* ����NAND FLASH. */
    if (res == 0X0D)                                                                /* NAND FLASH����,FAT�ļ�ϵͳ����,���¸�ʽ��NAND FLASH */
    {
        lcd_show_string(30, 150, 200, 16, 16, "NAND Disk Formatting...", RED);      /* ��ʽ��NAND FLASH */
        res = f_mkfs("2:", 0, 0, FF_MAX_SS);                                        /* ��ʽ��NAND FLASH,2:,�̷�;0,ʹ��Ĭ�ϸ�ʽ������ */

        if (res == 0)
        {
            f_setlabel((const TCHAR *)"2:NANDDISK");                                /* ����Flash���̵�����Ϊ��NANDDISK */
            lcd_show_string(30, 150, 200, 16, 16, "NAND Disk Format Finish", RED);  /* ��ʽ����� */
        }
        else
        {
            lcd_show_string(30, 150, 200, 16, 16, "NAND Disk Format Error ", RED);  /* ��ʽ��ʧ�� */
        }

        delay_ms(1000);
    }

    lcd_fill(30, 150, 240, 150 + 16, WHITE);                                        /* �����ʾ */

    while (exfuns_get_free((uint8_t *)"0:", &total, &free))                         /* �õ�SD������������ʣ������ */
    {
        lcd_show_string(30, 150, 200, 16, 16, "SD Card Fatfs Error!", RED);
        delay_ms(200);
        lcd_fill(30, 150, 240, 150 + 16, WHITE);                                    /* �����ʾ */
        delay_ms(200);
        LED0_TOGGLE();                                                              /* �����˸ */
    }

    lcd_show_string(30, 150, 200, 16, 16, "FATFS OK!", BLUE);     
    lcd_show_string(30, 170, 200, 16, 16, "SD Total Size:     MB", BLUE);
    lcd_show_string(30, 190, 200, 16, 16, "SD  Free Size:     MB", BLUE);
    lcd_show_num(30 + 8 * 14, 170, total >> 10, 5, 16, BLUE);                       /* ��ʾSD�������� MB */
    lcd_show_num(30 + 8 * 14, 190, free >> 10, 5, 16, BLUE);                        /* ��ʾSD��ʣ������ MB */

    while(1)
    {
        t++; 
        delay_ms(200);
        LED0_TOGGLE();         /* �����˸ */
    }
}


