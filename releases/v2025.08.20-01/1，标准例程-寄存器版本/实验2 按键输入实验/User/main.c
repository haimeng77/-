/**
 ****************************************************************************************************
 * @file        main.c
 * @author      ����ԭ���Ŷ�(ALIENTEK)
 * @version     V1.0
 * @date        2023-03-16
 * @brief       �������� ʵ��
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


int main(void)
{
    uint8_t key;
    
    sys_stm32_clock_init(160, 5, 2, 4);     /* ����ʱ��, 400Mhz */
    delay_init(400);                        /* ��ʱ��ʼ�� */
    led_init();                             /* ��ʼ��LED */
    key_init();                             /* ��ʼ������ */
    LED0(0);                                /* �ȵ������ */
    
    while(1)
    {
        key = key_scan(0);                  /* �õ���ֵ */

        if (key)
        {
            switch (key)
            {
                case WKUP_PRES:             /* ����LED0, LED1������� */
                    LED1_TOGGLE();          /* LED1״̬ȡ�� */
                
                    /* LED0����LED1��״̬, ���л������� */
                    if (LED1_GPIO_PORT->ODR & LED1_GPIO_PIN)
                    {
                        LED0(0);
                    }
                    else 
                    {
                        LED0(1);
                    }
                    
                    break;

                case KEY0_PRES:             /* ͬʱ����LED0, LED1��ת */
                    LED0_TOGGLE();          /* LED0״̬ȡ�� */
                    LED1_TOGGLE();          /* LED1״̬ȡ�� */
                    break;

                case KEY1_PRES:             /* ����LED1(GREEN)��ת */
                    LED1_TOGGLE();          /* LED1״̬ȡ�� */
                    break;

                case KEY2_PRES:             /* ����LED0(RED)��ת */
                    LED0_TOGGLE();          /* LED0״̬ȡ�� */
                    break;
            }
        }
        else
        {
            delay_ms(10);
        }
    }
}












