/**
 ****************************************************************************************************
 * @file        main.c
 * @author      ����ԭ���Ŷ�(ALIENTEK)
 * @version     V1.0
 * @date        2022-09-06
 * @brief       FLASHģ��EEPROM ʵ��
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
#include "./BSP/STMFLASH/stmflash.h"


/* Ҫд�뵽STM32 FLASH���ַ������� */
const uint8_t g_text_buf[] = {"STM32 FLASH TEST"};

#define TEXT_LENTH sizeof(g_text_buf)   /* ���鳤�� */

/*SIZE��ʾ���ֳ�(4�ֽ�), ��С������4��������, ������ǵĻ�, ǿ�ƶ��뵽4�������� */
#define SIZE TEXT_LENTH / 4 + ((TEXT_LENTH % 4) ? 1 : 0)

#define FLASH_SAVE_ADDR 0X08020000      /* ����FLASH �����ַ(����Ϊ4�ı���������������,Ҫ���ڱ�������ռ�õ�������.
                                           ����,д������ʱ��,���ܻᵼ�²�����������,�Ӷ����𲿷ֳ���ʧ.��������. */

int main(void)
{
    uint8_t key = 0;
    uint16_t i = 0;
    uint8_t datatemp[SIZE];

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

    lcd_show_string(30, 50, 200, 16, 16, "STM32", RED);
    lcd_show_string(30, 70, 200, 16, 16, "FLASH EEPROM TEST", RED);
    lcd_show_string(30, 90, 200, 16, 16, "ATOM@ALIENTEK", RED);
    lcd_show_string(30, 110, 200, 16, 16, "KEY1:Write  KEY0:Read", RED);

    while (1)
    {
        key = key_scan(0);

        if (key == KEY1_PRES)               /* KEY1����,д��STM32 FLASH */
        {
            lcd_fill(0, 150, 239, 319, WHITE);                                   /* ������� */
            lcd_show_string(30, 150, 200, 16, 16, "Start Write FLASH....", RED);
            stmflash_write(FLASH_SAVE_ADDR, (uint32_t *)g_text_buf, SIZE);
            lcd_show_string(30, 150, 200, 16, 16, "FLASH Write Finished!", RED); /* ��ʾ������� */
        }

        if (key == KEY0_PRES)               /* KEY0����,��ȡ�ַ�������ʾ */
        {
            lcd_show_string(30, 150, 200, 16, 16, "Start Read FLASH.... ", RED);
            stmflash_read(FLASH_SAVE_ADDR, (uint32_t *)datatemp, SIZE);
            lcd_show_string(30, 150, 200, 16, 16, "The Data Readed Is:  ", RED); /* ��ʾ������� */
            lcd_show_string(30, 170, 200, 16, 16, (char *)datatemp, BLUE);       /* ��ʾ�������ַ��� */
        }

        i++;
        delay_ms(10);

        if (i == 20)
        {
            LED0_TOGGLE();                  /* ��ʾϵͳ�������� */
            i = 0;
        }
    }
}


