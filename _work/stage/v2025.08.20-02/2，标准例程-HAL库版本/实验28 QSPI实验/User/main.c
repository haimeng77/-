/**
 ****************************************************************************************************
 * @file        main.c
 * @author      ����ԭ���Ŷ�(ALIENTEK)
 * @version     V1.0
 * @date        2022-09-06
 * @brief       QSPI ʵ��
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
#include "./BSP/KEY/key.h"
#include "./BSP/MPU/mpu.h"
#include "./BSP/QSPI/qspi.h"
#include "./BSP/NORFLASH/norflash.h"


/* Ҫд�뵽FLASH���ַ������� */
const uint8_t g_text_buf[] = {"STM32H7 QSPI TEST"};

#define TEXT_SIZE       sizeof(g_text_buf)  /* TEXT�ַ������� */
extern uint16_t g_norflash_type;            /* Ĭ����W25Q256 */

int main(void)
{
    uint8_t key;
    uint16_t i = 0;
    uint8_t datatemp[TEXT_SIZE + 2];
    uint8_t rectemp[TEXT_SIZE + 2];
    uint32_t flashsize;

    sys_cache_enable();                     /* ��L1-Cache */
    HAL_Init();                             /* ��ʼ��HAL�� */
    sys_stm32_clock_init(160, 5, 2, 4);     /* ����ʱ��, 400Mhz */
    delay_init(400);                        /* ��ʱ��ʼ�� */
    usart_init(115200);                     /* ���ڳ�ʼ�� */
    usmart_init(200);                       /* ��ʼ��USMART */
    mpu_memory_protection();                /* ������ش洢���� */
    led_init();                             /* ��ʼ��LED */
    key_init();                             /* ��ʼ��KEY */
    sdram_init();                           /* ��ʼ��SDRAM */
    lcd_init();                             /* ��ʼ��LCD */
    norflash_init();                        /* ��ʼ��norflash */

    lcd_show_string(30, 50, 200, 16, 16, "STM32", RED);
    lcd_show_string(30, 70, 200, 16, 16, "QSPI TEST", RED);
    lcd_show_string(30, 90, 200, 16, 16, "ATOM@ALIENTEK", RED);
    lcd_show_string(30, 110, 200, 16, 16, "KEY1:Write  KEY0:Read", RED);    /* ��ʾ��ʾ��Ϣ */

    while (norflash_read_id() != g_norflash_type)              /* ��ⲻ��FLASHоƬ */
    {
        lcd_show_string(30, 130, 200, 16, 16, "FLASH Check Failed!", RED);
        delay_ms(500);
        lcd_show_string(30, 130, 200, 16, 16, "Please Check!      ", RED);
        delay_ms(500);
        LED0_TOGGLE();                                        /* LED0��˸ */
    }

    lcd_show_string(30, 130, 200, 16, 16, "QSPI FLASH Ready!", BLUE);
    flashsize = 32 * 1024 * 1024;                             /* FLASH ��СΪ32M�ֽ� */

    while (1)
    {
        key = key_scan(0);

        if (key == KEY1_PRES)           /* KEY1����,д�� */
        {
            lcd_fill(0, 150, 239, 319, WHITE);  /* ������� */
            lcd_show_string(30, 150, 200, 16, 16, "Start Write FLASH....", BLUE);
            sprintf((char *)datatemp, "%s%d", (char *)g_text_buf, i);
            norflash_write((uint8_t *)datatemp, flashsize - 200, TEXT_SIZE + 2);   /* �ӵ�����200����ַ����ʼ,д��SIZE���ȵ����� */
            lcd_show_string(30, 150, 200, 16, 16, "FLASH Write Finished!", BLUE);  /* ��ʾ������� */
            printf("datatemp:%s  \r\n", datatemp);
        }

        if (key == KEY0_PRES)           /* KEY0����,��ȡ�ַ�������ʾ */
        {
            lcd_show_string(30, 150, 200, 16, 16, "Start Read FLASH... . ", BLUE);
            norflash_read((uint8_t *)rectemp, flashsize - 200, TEXT_SIZE + 2);     /* �ӵ�����200����ַ����ʼ,����SIZE���ֽ� */
            lcd_show_string(30, 150, 200, 16, 16, "The Data Readed Is:   ", BLUE); /* ��ʾ������� */
            lcd_show_string(30, 170, 200, 16, 16, (char *)rectemp, BLUE);          /* ��ʾ�������ַ��� */
            printf("rectemp:%s  \r\n", rectemp);
        }

        i++;

        if (i == 20)
        {
            LED0_TOGGLE();              /* LED0��˸ */
            i = 0;
        }

        delay_ms(10);
    }
}
