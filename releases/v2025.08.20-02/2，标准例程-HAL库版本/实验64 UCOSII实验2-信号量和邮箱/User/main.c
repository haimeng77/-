/**
 ****************************************************************************************************
 * @file        main.c
 * @author      ����ԭ���Ŷ�(ALIENTEK)
 * @version     V1.0
 * @date        2022-09-06
 * @brief       UCOSII-�ź��������� ʵ��
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
#include "./BSP/LED/led.h"
#include "./BSP/LCD/lcd.h"
#include "./BSP/KEY/key.h"
#include "./BSP/MPU/mpu.h"
#include "./BSP/PCF8574/pcf8574.h"
#include "./BSP/TOUCH/touch.h"
#include "uc-os2_demo.h"



int main(void)
{
    sys_cache_enable();                        /* ��L1-Cache */
    HAL_Init();                                /* ��ʼ��HAL�� */
    sys_stm32_clock_init(160, 5, 2, 4);        /* ����ʱ��, 400Mhz */
    delay_init(400);                           /* ��ʱ��ʼ�� */
    usart_init(115200);                        /* ���ڳ�ʼ�� */
    mpu_memory_protection();                   /* ������ش洢���� */
    led_init();                                /* ��ʼ��LED */
    sdram_init();                              /* ��ʼ��SDRAM */
    lcd_init();                                /* ��ʼ��LCD */
    key_init();                                /* ��ʼ��KEY */
    pcf8574_init();                            /* ��ʼ��PCF8574 */
    tp_dev.init();                             /* ��������ʼ�� */

    uc_os2_demo();                             /* ����uC/OS-II���� */
}


