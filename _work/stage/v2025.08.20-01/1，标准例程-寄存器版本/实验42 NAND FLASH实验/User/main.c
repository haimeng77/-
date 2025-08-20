/**
 ****************************************************************************************************
 * @file        main.c
 * @author      ����ԭ���Ŷ�(ALIENTEK)
 * @version     V1.0
 * @date        2023-03-24
 * @brief       NAND FLASH ʵ��
 * @license     Copyright (c) 2022-2032, ������������ӿƼ����޹�˾
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
#include "./BSP/LED/led.h"
#include "./BSP/KEY/key.h"
#include "./BSP/MPU/mpu.h"
#include "./BSP/LCD/lcd.h"
#include "./BSP/SDRAM/sdram.h"
#include "./USMART/usmart.h"
#include "./MALLOC/malloc.h"
#include "./BSP/NAND/ftl.h"
#include "./BSP/NAND/nand.h"



int main(void)
{
    uint8_t key, t = 0;
    uint16_t i;
    uint8_t *buf;
    uint8_t *backbuf;
    
    sys_stm32_clock_init(160, 5, 2, 4);     /* ����ʱ��, 400Mhz */
    delay_init(400);                        /* ��ʱ��ʼ�� */
    usart_init(100, 115200);                /* ��ʼ��USART */
    usmart_dev.init(100);                   /* ��ʼ��USMART */
    led_init();                             /* ��ʼ��LED */
    mpu_memory_protection();                /* ������ش洢���� */
    sdram_init();                           /* ��ʼ��SDRAM */
    lcd_init();                             /* ��ʼ��LCD */
    key_init();                             /* ��ʼ������ */
    
    my_mem_init(SRAMIN);                    /* ��ʼ���ڲ��ڴ��(AXI) */
    my_mem_init(SRAMEX);                    /* ��ʼ���ⲿ�ڴ��(SDRAM) */
    my_mem_init(SRAM12);                    /* ��ʼ��SRAM12�ڴ��(SRAM1+SRAM2) */
    my_mem_init(SRAM4);                     /* ��ʼ��SRAM4�ڴ��(SRAM4) */
    my_mem_init(SRAMDTCM);                  /* ��ʼ��DTCM�ڴ��(DTCM) */
    my_mem_init(SRAMITCM);                  /* ��ʼ��ITCM�ڴ��(ITCM) */

    lcd_show_string(30,  50, 200, 16, 16, "STM32", RED);
    lcd_show_string(30,  70, 200, 16, 16, "NAND TEST", RED);
    lcd_show_string(30,  90, 200, 16, 16, "ATOM@ALIENTEK", RED);
    lcd_show_string(30, 110, 200, 16, 16, "KEY0:Read Sector 2", RED);
    lcd_show_string(30, 130, 200, 16, 16, "KEY1:Write Sector 2", RED);
    lcd_show_string(30, 150, 200, 16, 16, "KEY2:Recover Sector 2", RED);

    while (ftl_init())                                                   /* ���NAND FLASH,����ʼ��FTL */
    {
        lcd_show_string(30, 170, 200, 16, 16, "NAND Error!", RED);
        delay_ms(500);
        lcd_show_string(30, 170, 200, 16, 16, "Please Check! ", RED);
        delay_ms(500);
        LED0_TOGGLE();                                                   /* �����˸ */
    }

    backbuf =   mymalloc(SRAMIN, NAND_ECC_SECTOR_SIZE);                  /* ����һ�������Ļ��� */
    buf = mymalloc(SRAMIN, NAND_ECC_SECTOR_SIZE);                        /* ����һ�������Ļ��� */
    sprintf((char *)buf, "NAND Size:%dMB", (nand_dev.block_totalnum / 1024) * (nand_dev.page_mainsize / 1024) * nand_dev.block_pagenum);
    lcd_show_string(30, 190, 200, 16, 16, (char *)buf, BLUE);            /* ��ʾNAND���� */
    ftl_read_sectors(backbuf, 2, NAND_ECC_SECTOR_SIZE, 1);               /* Ԥ�ȶ�ȡ����0����������,��ֹ��д�����ļ�ϵͳ��. */

    while(1)
    {
        key = key_scan(0);

        switch (key)
        {
            case KEY0_PRES:                                              /* KEY0����,��ȡsector */
                key = ftl_read_sectors(buf, 2, NAND_ECC_SECTOR_SIZE, 1); /* ��ȡ���� */

                if (key == 0)                                            /* ��ȡ�ɹ� */
                {
                    lcd_show_string(30, 210, 200, 16, 16, "USART1 Sending Data...  ", BLUE);
                    printf("Sector 2 data is:\r\n");

                    for (i = 0; i < NAND_ECC_SECTOR_SIZE; i++)
                    {
                        printf("%x ",buf[i]);                            /* ������� */
                    }

                    printf("\r\ndata end.\r\n");
                    lcd_show_string(30, 210, 200, 16, 16, "USART1 Send Data Over!  ", BLUE); 
                }
                break;

            case KEY1_PRES:                                             /* KEY1����,д��sector */
                for (i = 0; i < NAND_ECC_SECTOR_SIZE; i++)
                 {
                     buf[i] = i + t;                                    /* �������(�����,����t��ֵ��ȷ��) */
                 }

                lcd_show_string(30, 210, 210, 16, 16, "Writing data to sector..", BLUE);
                key = ftl_write_sectors(buf, 2, NAND_ECC_SECTOR_SIZE, 1);                    /* д������ */

                if (key == 0)
                {
                    lcd_show_string(30, 210, 200, 16, 16, "Write data successed    ", BLUE); /* д��ɹ� */
                }
                else 
                {
                    lcd_show_string(30, 210, 200, 16, 16, "Write data failed       ", BLUE); /* д��ʧ�� */
                }
                break;

            case KEY2_PRES:                                                                  /* KEY2����,�ָ�sector������ */
                lcd_show_string(30, 210, 210, 16, 16, "Recovering data...      ", BLUE);
                key = ftl_write_sectors(backbuf, 2, NAND_ECC_SECTOR_SIZE, 1);                /* д������ */

                if (key == 0)
                {
                    lcd_show_string(30, 210, 200, 16, 16, "Recovering data OK      ", BLUE); /* �ָ��ɹ� */
                }
                else 
                {
                    lcd_show_string(30, 210, 200, 16, 16, "Recovering data failed  ", BLUE); /* �ָ�ʧ�� */
                }
                break;
        }

        t++;
        delay_ms(10);

        if (t == 20)
        {
            LED0_TOGGLE();               /* �����˸ */
            t = 0;
        }
    } 
}












