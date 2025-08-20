/**
 ****************************************************************************************************
 * @file        main.c
 * @author      ����ԭ���Ŷ�(ALIENTEK)
 * @version     V1.0
 * @date        2022-09-06
 * @brief       ��ͨ��ADC�ɼ�(DMA��ȡ) ʵ��
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
#include "./BSP/MPU/mpu.h"
#include "./BSP/SDRAM/sdram.h"
#include "./BSP/ADC/adc.h"


#define ADC_DMA_BUF_SIZE        50 * 6       /* ADC DMA�ɼ� BUF��С, Ӧ����ADCͨ������������ */
uint16_t g_adc_dma_buf[ADC_DMA_BUF_SIZE];    /* ADC DMA BUF */

extern uint8_t g_adc_dma_sta;                /* DMA����״̬��־, 0,δ���; 1, ����� */

int main(void)
{
    uint16_t i, j;
    uint16_t adcx;
    uint32_t sum;
    float temp;

    sys_cache_enable();                      /* ��L1-Cache */
    HAL_Init();                              /* ��ʼ��HAL�� */
    sys_stm32_clock_init(160, 5, 2, 4);      /* ����ʱ��, 400Mhz */
    delay_init(400);                         /* ��ʱ��ʼ�� */
    usart_init(115200);                      /* ���ڳ�ʼ�� */
    mpu_memory_protection();                 /* ������ش洢���� */
    led_init();                              /* ��ʼ��LED */
    sdram_init();                            /* ��ʼ��SDRAM */
    lcd_init();                              /* ��ʼ��LCD */

    adc_nch_dma_init((uint32_t)&ADC1->DR, (uint32_t)&g_adc_dma_buf);      /* ��ʼ��ADC DMA�ɼ� */

    lcd_show_string(30, 50, 200, 16, 16, "STM32", RED);
    lcd_show_string(30, 70, 200, 16, 16, "ADC DMA TEST", RED);
    lcd_show_string(30, 90, 200, 16, 16, "ATOM@ALIENTEK", RED);

    lcd_show_string(30, 130, 200, 12, 12, "ADC1_CH14_VAL:", BLUE);
    lcd_show_string(30, 142, 200, 12, 12, "ADC1_CH14_VOL:0.000V", BLUE); /* ���ڹ̶�λ����ʾС���� */
    
    lcd_show_string(30, 160, 200, 12, 12, "ADC1_CH15_VAL:", BLUE);
    lcd_show_string(30, 172, 200, 12, 12, "ADC1_CH15_VOL:0.000V", BLUE); /* ���ڹ̶�λ����ʾС���� */

    lcd_show_string(30, 190, 200, 12, 12, "ADC1_CH16_VAL:", BLUE);
    lcd_show_string(30, 202, 200, 12, 12, "ADC1_CH16_VOL:0.000V", BLUE); /* ���ڹ̶�λ����ʾС���� */

    lcd_show_string(30, 220, 200, 12, 12, "ADC1_CH17_VAL:", BLUE);
    lcd_show_string(30, 232, 200, 12, 12, "ADC1_CH17_VOL:0.000V", BLUE); /* ���ڹ̶�λ����ʾС���� */

    lcd_show_string(30, 250, 200, 12, 12, "ADC1_CH18_VAL:", BLUE);
    lcd_show_string(30, 262, 200, 12, 12, "ADC1_CH18_VOL:0.000V", BLUE); /* ���ڹ̶�λ����ʾС���� */

    lcd_show_string(30, 280, 200, 12, 12, "ADC1_CH19_VAL:", BLUE);
    lcd_show_string(30, 292, 200, 12, 12, "ADC1_CH19_VOL:0.000V", BLUE); /* ���ڹ̶�λ����ʾС���� */

    adc_dma_enable(ADC_DMA_BUF_SIZE);                                    /* ����ADC DMA�ɼ� */

    while (1)
    {
        if (g_adc_dma_sta == 1)
        {
            /* ��D Cache */
            SCB_InvalidateDCache();

            /* ѭ����ʾͨ��14~ͨ��19�Ľ�� */
            for (j = 0; j < 6; j++)                                          /* ����6��ͨ�� */
            {
                sum = 0;                                                     /* ���� */
                for (i = 0; i < ADC_DMA_BUF_SIZE / 6; i++)                   /* ÿ��ͨ���ɼ���50������,����50���ۼ� */
                {
                    sum += g_adc_dma_buf[(6 * i) + j];                       /* ��ͬͨ����ת�������ۼ� */
                }
                adcx = sum / (ADC_DMA_BUF_SIZE / 6);                         /* ȡƽ��ֵ */
                
                /* ��ʾ��� */
                lcd_show_xnum(114, 130 + (j * 30), adcx, 5, 12, 0, BLUE);    /* ��ʾADC�������ԭʼֵ */

                temp = (float)adcx * (3.3 / 65536);                          /* ��ȡ�����Ĵ�С����ʵ�ʵ�ѹֵ������3.1111 */
                adcx = temp;                                                 /* ��ֵ�������ָ�adcx��������ΪadcxΪu16���� */
                lcd_show_xnum(114, 142 + (j * 30), adcx, 1, 12, 0, BLUE);    /* ��ʾ��ѹֵ���������֣�3.1111�Ļ������������ʾ3 */

                temp -= adcx;                                                /* ���Ѿ���ʾ����������ȥ��������С�����֣�����3.1111-3=0.1111 */
                temp *= 1000;                                                /* С�����ֳ���1000�����磺0.1111��ת��Ϊ111.1���൱�ڱ�����λС�� */
                lcd_show_xnum(126, 142 + (j * 30), temp, 3, 12, 0X80, BLUE); /* ��ʾС�����֣�ǰ��ת��Ϊ��������ʾ����������ʾ�ľ���111. */
            }

            g_adc_dma_sta = 0;                                               /* ���DMA�ɼ����״̬��־ */
            adc_dma_enable(ADC_DMA_BUF_SIZE);                                /* ������һ��ADC DMA�ɼ� */
        }

        LED0_TOGGLE();
        delay_ms(100);
    }
}
