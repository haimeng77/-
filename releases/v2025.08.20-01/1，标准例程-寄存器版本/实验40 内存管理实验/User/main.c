/**
 ****************************************************************************************************
 * @file        main.c
 * @author      ����ԭ���Ŷ�(ALIENTEK)
 * @version     V1.0
 * @date        2023-03-24
 * @brief       �ڴ���� ʵ��
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


const char *SRAM_NAME_BUF[SRAMBANK] = {"SRAMIN  ", "SRAMEX  ", "SRAM12  ", "SRAM4   ", "SRAMDTCM", "SRAMITCM"};

int main(void)
{
    uint8_t paddr[20];                      /* ���P Addr:+p��ַ��ASCIIֵ */
    uint16_t memused = 0;
    uint8_t key;
    uint8_t i = 0;
    uint8_t *p = 0;
    uint8_t *tp = 0;
    uint8_t sramx = 0;                      /* Ĭ��Ϊ�ڲ�sram */
    
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
    lcd_show_string(30,  70, 200, 16, 16, "MALLOC TEST", RED);
    lcd_show_string(30,  90, 200, 16, 16, "ATOM@ALIENTEK", RED);
    lcd_show_string(30, 110, 200, 16, 16, "KEY0:Malloc  KEY2:Free", RED);
    lcd_show_string(30, 130, 200, 16, 16, "KEY_UP:SRAMx KEY1:Read", RED);

    lcd_show_string(60, 160, 200, 16, 16, "SRAMIN ", BLUE);
    lcd_show_string(30, 180, 200, 16, 16, "SRAMIN   USED:", BLUE);
    lcd_show_string(30, 200, 200, 16, 16, "SRAMEX   USED:", BLUE);
    lcd_show_string(30, 220, 200, 16, 16, "SRAM12   USED:", BLUE);
    lcd_show_string(30, 240, 200, 16, 16, "SRAM4    USED:", BLUE);
    lcd_show_string(30, 260, 200, 16, 16, "SRAMDTCM USED:", BLUE);
    lcd_show_string(30, 280, 200, 16, 16, "SRAMITCM USED:", BLUE);

    while (1)
    {
        key = key_scan(0);      /* ��֧������ */

        switch (key)
        {
            case 0:                                   /* û�а������� */
                break;
            
            case KEY0_PRES:                           /* KEY0���� */
                p = mymalloc(sramx, 2048);            /* ����2K�ֽ�,��д������,��ʾ��lcd��Ļ���� */

                if (p != NULL)
                {
                    sprintf((char *)p, "Memory Malloc Test%03d", i);   /* ��pд��һЩ���� */
                }
                break;

            case KEY1_PRES:              /* KEY1���� */
                if (p != NULL)
                {
                    sprintf((char *)p, "Memory Malloc Test%03d", i);       /* ������ʾ���� */
                    lcd_show_string(30, 310, 209, 16, 16, (char *)p, BLUE);/* ��ʾP������ */
                }
                break;

            case KEY2_PRES:              /* KEY2���� */
                myfree(sramx, p);        /* �ͷ��ڴ� */
                p = 0;                   /* ָ��յ�ַ */
                break;

            case WKUP_PRES:              /* KEY UP���� */
                sramx++;

                if (sramx >= SRAMBANK) 
                {
                    sramx = 0;
                }

                lcd_show_string(60, 160, 200, 16, 16, (char *)SRAM_NAME_BUF[sramx], BLUE);
                break;
        }

        if (tp != p && p != NULL)
        {
            tp = p;
            sprintf((char *)paddr, "P Addr:0X%08X", (uint32_t)tp);
            lcd_show_string(30, 310, 209, 16, 16, (char *)paddr, BLUE);        /* ��ʾp�ĵ�ַ */

            if (p)
            {
                lcd_show_string(30, 330, 330, 16, 16, (char *)p, BLUE);        /* ��ʾP������ */
            }
            else 
            {
                lcd_fill(30, 310, 300, 209, WHITE);                            /* p=0,�����ʾ */
            }
        }

        delay_ms(10);
        i++;

        if ((i % 20) == 0)                                                     /* DS0��˸. */
        {
            memused = my_mem_perused(SRAMIN);
            sprintf((char *)paddr, "%d.%01d%%", memused / 10, memused % 10);
            lcd_show_string(30 + 112, 180, 200, 16, 16, (char *)paddr, BLUE);  /* ��ʾ�ڲ��ڴ�ʹ���� */
            
            memused = my_mem_perused(SRAMEX);
            sprintf((char *)paddr, "%d.%01d%%", memused / 10, memused % 10);
            lcd_show_string(30 + 112, 200, 200, 16, 16, (char *)paddr, BLUE);  /* ��ʾ�ⲿ�ڴ�ʹ���� */
            
            memused = my_mem_perused(SRAM12);
            sprintf((char *)paddr, "%d.%01d%%", memused / 10, memused % 10);
            lcd_show_string(30 + 112, 220, 200, 16, 16, (char *)paddr, BLUE);  /* ��ʾSRAM12�ڴ�ʹ���� */
            
            memused = my_mem_perused(SRAM4);
            sprintf((char *)paddr, "%d.%01d%%", memused / 10, memused % 10);
            lcd_show_string(30 + 112, 240, 200, 16, 16, (char *)paddr, BLUE);  /* ��ʾSRAM4�ڴ�ʹ���� */
            
            memused = my_mem_perused(SRAMDTCM);
            sprintf((char *)paddr, "%d.%01d%%", memused / 10, memused % 10);
            lcd_show_string(30 + 112, 260, 200, 16, 16, (char *)paddr, BLUE);  /* ��ʾDCM�ڴ�ʹ���� */
            
            memused = my_mem_perused(SRAMITCM);
            sprintf((char *)paddr, "%d.%01d%%", memused / 10, memused % 10);
            lcd_show_string(30 + 112, 280, 200, 16, 16, (char *)paddr, BLUE);  /* ��ʾTCM�ڴ�ʹ���� */
            
            LED0_TOGGLE();  /* LED0��˸ */
        }
    }
}




















