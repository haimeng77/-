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
#include "./BSP/KEY/key.h"
#include "./BSP/MPU/mpu.h"
#include "./BSP/SDRAM/sdram.h"
#include "./BSP/ADC/adc.h"


#define ADC_DMA_BUF_SIZE        50             /* ADC DMA�ɼ� BUF��С, Ӧ����ADCͨ������������ */

uint16_t g_adc_dma_buf[ADC_DMA_BUF_SIZE];      /* ADC DMA BUF */
extern uint8_t g_adc_dma_sta;                  /* DMA����״̬��־, 0,δ���; 1, ����� */

int main(void)
{
    uint16_t i;
    uint16_t adcx;
    uint32_t sum;
    float temp;

    sys_cache_enable();                        /* ��L1-Cache */
    HAL_Init();                                /* ��ʼ��HAL�� */
    sys_stm32_clock_init(160, 5, 2, 4);        /* ����ʱ��, 400Mhz */
    delay_init(400);                           /* ��ʱ��ʼ�� */
    usart_init(115200);                        /* ���ڳ�ʼ�� */
    mpu_memory_protection();                   /* ������ش洢���� */
    led_init();                                /* ��ʼ��LED */
    key_init();                                /* ��ʼ��KEY */
    sdram_init();                              /* ��ʼ��SDRAM */
    lcd_init();                                /* ��ʼ��LCD */

    adc_dma_init((uint32_t)&ADC1->DR, (uint32_t)&g_adc_dma_buf);    /* ��ʼ��ADC */

    lcd_show_string(30, 50, 200, 16, 16, "STM32", RED);
    lcd_show_string(30, 70, 200, 16, 16, "ADC DMA TEST", RED);
    lcd_show_string(30, 90, 200, 16, 16, "ATOM@ALIENTEK", RED);

    lcd_show_string(30, 130, 200, 16, 16, "ADC1_CH19_VAL:", BLUE);
    lcd_show_string(30, 150, 200, 16, 16, "ADC1_CH19_VOL:0.000V", BLUE);  /* ���ڹ̶�λ����ʾС����  */

    adc_dma_enable(ADC_DMA_BUF_SIZE);       /* ����ADC DMA�ɼ� */

    while (1)
    {
        if (g_adc_dma_sta == 1)
        {
            /* ��D Cache */
            SCB_InvalidateDCache();

            /* ����DMA �ɼ�����ADC���ݵ�ƽ��ֵ */
            sum = 0;

            for (i = 0; i < ADC_DMA_BUF_SIZE; i++)              /* �ۼ� */
            {
                sum += g_adc_dma_buf[i];
            }

            adcx = sum / ADC_DMA_BUF_SIZE;                      /* ȡƽ��ֵ */

            /* ��ʾ��� */
            lcd_show_xnum(142, 130, adcx, 5, 16, 0, BLUE);      /* ��ʾADC�������ԭʼֵ */

            temp = (float)adcx * (3.3 / 65536);                 /* ��ȡ�����Ĵ�С����ʵ�ʵ�ѹֵ������3.1111 */
            adcx = temp;                                        /* ��ֵ�������ָ�adcx��������ΪadcxΪu16���� */
            lcd_show_xnum(142, 150, adcx, 1, 16, 0, BLUE);      /* ��ʾ��ѹֵ���������֣�3.1111�Ļ������������ʾ3 */

            temp -= adcx;                                       /* ���Ѿ���ʾ����������ȥ��������С�����֣�����3.1111-3=0.1111 */
            temp *= 1000;                                       /* С�����ֳ���1000�����磺0.1111��ת��Ϊ111.1���൱�ڱ�����λС���� */
            lcd_show_xnum(158, 150, temp, 3, 16, 0x80, BLUE);   /* ��ʾС�����֣�ǰ��ת��Ϊ��������ʾ����������ʾ�ľ���111. */

            g_adc_dma_sta = 0;                                  /* ���DMA�ɼ����״̬��־ */
            adc_dma_enable(ADC_DMA_BUF_SIZE);                   /* ������һ��ADC DMA�ɼ� */
        }

        LED0_TOGGLE();
        delay_ms(100);
    }
}
