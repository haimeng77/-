/**
 ****************************************************************************************************
 * @file        main.c
 * @author      ����ԭ���Ŷ�(ALIENTEK)
 * @version     V1.0
 * @date        2022-09-06
 * @brief       UCOSII-��Ϣ���С��ź������������ʱ�� ʵ��
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
#include "./BSP/KEY/key.h"
#include "./BSP/SDRAM/sdram.h"
#include "./BSP/MPU/mpu.h"
#include "./BSP/LCD/ltdc.h"
#include "./MALLOC/malloc.h"
#include "./BSP/TOUCH/touch.h"
#include "./BSP/PCF8574/pcf8574.h"
#include "uc-os2_demo.h"



int main(void)
{
    sys_cache_enable();                      /* ��L1-Cache */
    HAL_Init();                              /* ��ʼ��HAL�� */
    sys_stm32_clock_init(160, 5, 2, 4);      /* ����ʱ��, 400Mhz */
    usart_init(115200);                      /* ���ڳ�ʼ�� */
    delay_init(400);                         /* ��ʱ��ʼ�� */
    mpu_memory_protection();                 /* ������ش洢���� */
    sdram_init();                            /* ��ʼ��SDRAM */
    led_init();                              /* ��ʼ��LED */
    lcd_init();                              /* ��ʼ��LCD */
    key_init();                              /* ��ʼ������ */
    pcf8574_init();                          /* ��ʼ��PCF8574 */
    tp_dev.init();                           /* ��������ʼ�� */

    my_mem_init(SRAMIN);                     /* ��ʼ���ڲ��ڴ��(AXI) */
    my_mem_init(SRAMEX);                     /* ��ʼ���ⲿ�ڴ��(SDRAM) */
    my_mem_init(SRAM12);                     /* ��ʼ��SRAM12�ڴ��(SRAM1+SRAM2) */
    my_mem_init(SRAM4);                      /* ��ʼ��SRAM4�ڴ��(SRAM4) */
    my_mem_init(SRAMDTCM);                   /* ��ʼ��DTCM�ڴ��(DTCM) */
    my_mem_init(SRAMITCM);                   /* ��ʼ��ITCM�ڴ��(ITCM) */

    uc_os2_demo();                           /* ����uC/OS-II���� */
}


