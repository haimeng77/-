/**
 ****************************************************************************************************
 * @file        main.c
 * @author      ����ԭ���Ŷ�(ALIENTEK)
 * @version     V1.0
 * @date        2022-09-06
 * @brief       USMART ʵ��
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
#include "./BSP/LED/led.h"
#include "./BSP/LCD/lcd.h"
#include "./BSP/SDRAM/sdram.h"
#include "./USMART/usmart.h"
#include "./BSP/MPU/mpu.h"


/* LED״̬���ú��� */
void led_set(uint8_t sta)
{
    LED1(sta);
}

/* �����������ò��Ժ��� */
void test_fun(void(*ledset)(uint8_t), uint8_t sta)
{
    ledset(sta);
}

int main(void)
{
    sys_cache_enable();                 /* ��L1-Cache */
    HAL_Init();                         /* ��ʼ��HAL�� */
    sys_stm32_clock_init(160, 5, 2, 4); /* ����ʱ��, 400Mhz */
    delay_init(400);                    /* ��ʱ��ʼ�� */
    usart_init(115200);                 /* ���ڳ�ʼ�� */
    mpu_memory_protection();            /* ������ش洢���� */
    usmart_dev.init(200);               /* USMART��ʼ�� */
    led_init();                         /* ��ʼ��LED */
    sdram_init();                       /* ��ʼ��SDRAM */
    lcd_init();                         /* ��ʼ��LCD */

    lcd_show_string(30, 50, 200, 16, 16, "STM32", RED);
    lcd_show_string(30, 70, 200, 16, 16, "USMART TEST", RED);
    lcd_show_string(30, 90, 200, 16, 16, "ATOM@ALIENTEK", RED);

    while (1)
    {
        LED0_TOGGLE();                  /* �����˸ */
        delay_ms(1000);
    }
}
