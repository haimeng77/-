/**
 ****************************************************************************************************
 * @file        main.c
 * @author      ����ԭ���Ŷ�(ALIENTEK)
 * @version     V1.0
 * @date        2023-03-21
 * @brief       �ڴ汣��(MPU) ʵ��
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


#if !(__ARMCC_VERSION >= 6010050)   /* ����AC6����������ʹ��AC5������ʱ */
uint8_t mpudata[128] __attribute__((at(0X20002000)));  /* ����һ������ */
#else
uint8_t mpudata[128] __attribute__((section(".bss.ARM.__at_0X20002000"))); /* ����һ������ */
#endif

int main(void)
{
    uint8_t key = 0;
    uint8_t t = 0; 
    
    sys_stm32_clock_init(160, 5, 2, 4);     /* ����ʱ��, 400Mhz */
    delay_init(400);                        /* ��ʱ��ʼ�� */
    usart_init(100, 115200);                /* ��ʼ��USART */
    led_init();                             /* ��ʼ��LED */ 
    key_init();                             /* ��ʼ������ */
    printf("\r\n\r\nMPU closed!\r\n");      /* ��ʾMPU�ر� */

    while (1)
    {
        key = key_scan(0);

        if (key == WKUP_PRES)               /* ʹ��MPU�������� mpudata */
        {
            mpu_set_protection(0X20002000, 128, 0, 0, MPU_REGION_PRIV_RO_URO, 0, 0, 1); /* ֻ��,��ֹ����,��ֹcatch,������ */
            printf("MPU open!\r\n");        /* ��ʾMPU�� */
        }
        else if (key == KEY0_PRES)          /* ��������д�����ݣ����������MPU�����Ļ�������ڴ���ʴ��� */
        {
            printf("Start Writing data...\r\n");
            sprintf((char *)mpudata, "MPU test array %d", t);
            printf("Data Write finshed!\r\n");
        }
        else if (key == KEY1_PRES)          /* �������ж�ȡ���ݣ�������û�п���MPU��������������ڴ���ʴ��� */
        {
            printf("Array data is:%s\r\n", mpudata);
        }
        else 
        {
            delay_ms(10);
        }
        
        t++;

        if ((t % 50) == 0) LED0_TOGGLE();   /* LED0ȡ�� */
    }
}


























































