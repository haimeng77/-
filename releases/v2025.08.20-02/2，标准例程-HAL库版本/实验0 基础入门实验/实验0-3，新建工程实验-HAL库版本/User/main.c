/**
 ****************************************************************************************************
 * @file        main.c
 * @author      ����ԭ���Ŷ�(ALIENTEK)
 * @version     V1.0
 * @date        2022-09-6
 * @brief       �½�����ʵ��-HAL��汾 ʵ��
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


void led_init(void);                                            /* LED��ʼ���������� */

int main(void)
{
    sys_cache_enable();                                         /* ��L1-Cache */
    HAL_Init();                                                 /* ��ʼ��HAL�� */
    sys_stm32_clock_init(160, 5, 2, 4);                         /* ����ʱ��, 400Mhz */
    delay_init(400);                                            /* ��ʱ��ʼ�� */
    led_init();                                                 /* ��ʼ��LED */

    while (1)
    {
        HAL_GPIO_WritePin(GPIOB,GPIO_PIN_0, GPIO_PIN_SET);      /* PB0��1 */ 
        HAL_GPIO_WritePin(GPIOB,GPIO_PIN_1, GPIO_PIN_RESET);    /* PB1��0 */ 
        delay_ms(500);
        HAL_GPIO_WritePin(GPIOB,GPIO_PIN_0, GPIO_PIN_RESET);    /* PB0��0 */
        HAL_GPIO_WritePin(GPIOB,GPIO_PIN_1, GPIO_PIN_SET);      /* PB1��1 */
        delay_ms(500); 
    }
}

/**
 * @brief       ��ʼ��LED���IO��, ��ʹ��ʱ��
 * @param       ��
 * @retval      ��
 */
void led_init(void)
{
    GPIO_InitTypeDef gpio_init_struct;
    __HAL_RCC_GPIOB_CLK_ENABLE();                               /* PB0ʱ��ʹ�� */
    __HAL_RCC_GPIOB_CLK_ENABLE();                               /* PB1ʱ��ʹ�� */

    gpio_init_struct.Pin = GPIO_PIN_0;                          /* LED1���� */
    gpio_init_struct.Mode = GPIO_MODE_OUTPUT_PP;                /* ������� */
    gpio_init_struct.Pull = GPIO_PULLUP;                        /* ���� */
    gpio_init_struct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;         /* ���� */
    HAL_GPIO_Init(GPIOB, &gpio_init_struct);                    /* ��ʼ��LED1���� */

    gpio_init_struct.Pin = GPIO_PIN_1;                          /* LED0���� */
    HAL_GPIO_Init(GPIOB, &gpio_init_struct);                    /* ��ʼ��LED0���� */
}

