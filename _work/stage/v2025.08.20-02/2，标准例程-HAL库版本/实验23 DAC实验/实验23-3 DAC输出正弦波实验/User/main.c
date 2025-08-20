/**
 ****************************************************************************************************
 * @file        main.c
 * @author      ����ԭ���Ŷ�(ALIENTEK)
 * @version     V1.0
 * @date        2022-09-06
 * @brief       DAC DMA������Ҳ� ʵ��
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
#include "./BSP/ADC/adc.h"
#include "./BSP/DAC/dac.h"


uint16_t g_dac_sin_buf[4096];    /* �������ݻ����� */

/**
 * @brief       �������Ҳ�����
 * @note        �豣֤: maxval > samples/2
 * @param       maxval : ���ֵ(0 < maxval < 2048)
 * @param       samples: ������ĸ���
 * @retval      ��
 */
void dac_creat_sin_buf(uint16_t maxval, uint16_t samples)
{
    uint8_t i;
    float inc = (2 * 3.1415926) / samples;      /* ����������һ������DAC_SIN_BUF���㣩*/
    float outdata = 0;

    for (i = 0; i < samples; i++)
    {
        outdata = maxval * (1 + sin(inc * i));  /* ������dots����Ϊ���ڵ�ÿ�����ֵ���Ŵ�maxval������ƫ�Ƶ��������� */
        if (outdata > 4095)
        {
            outdata = 4095;                     /* �����޶� */ 
        }
        g_dac_sin_buf[i] = outdata;
    }
}

/**
 * @brief       ͨ��USMART�������Ҳ��������,�����޸����Ƶ��.
 * @param       arr : TIM7���Զ���װ��ֵ
 * @param       psc : TIM7�ķ�Ƶϵ��
 * @retval      ��
 */
void dac_dma_sin_set(uint16_t arr, uint16_t psc)
{
    dac_dma_wave_enable(100, arr, psc);
}

int main(void)
{
    uint16_t adcx;
    float temp;
    uint8_t t = 0;
    uint8_t key;

    sys_cache_enable();                        /* ��L1-Cache */
    HAL_Init();                                /* ��ʼ��HAL�� */
    sys_stm32_clock_init(160, 5, 2, 4);        /* ����ʱ��, 400Mhz */
    delay_init(400);                           /* ��ʱ��ʼ�� */
    usart_init(115200);                        /* ���ڳ�ʼ�� */
    usmart_dev.init(200);                      /* ��ʼ��USMART */
    mpu_memory_protection();                   /* ������ش洢���� */
    led_init();                                /* ��ʼ��LED */
    sdram_init();                              /* ��ʼ��SDRAM */
    lcd_init();                                /* ��ʼ��LCD */
    key_init();                                /* ��ʼ������ */
    adc_init();                                /* ��ʼ��ADC */
    dac_dma_wave_init();                       /* ��ʼ��DACͨ��1 DMA������� */

    lcd_show_string(30, 50, 200, 16, 16, "STM32", RED);
    lcd_show_string(30, 70, 200, 16, 16, "DAC DMA Sine WAVE TEST", RED);
    lcd_show_string(30, 90, 200, 16, 16, "ATOM@ALIENTEK", RED);
    lcd_show_string(30, 110, 200, 16, 16, "KEY0:5Khz  KEY1:50Khz", RED);

    lcd_show_string(30, 130, 200, 16, 16, "DAC VAL:", BLUE);
    lcd_show_string(30, 150, 200, 16, 16, "DAC VOL:0.000V", BLUE);
    lcd_show_string(30, 170, 200, 16, 16, "ADC VOL:0.000V", BLUE);

    dac_creat_sin_buf(2048, 100);
    dac_dma_wave_enable(100, 100 - 1, 20 - 1);  /* 100Khz����Ƶ��, 100����, ���Եõ����1KHz�����Ҳ�. */

    while (1)
    {
        t++;
        key = key_scan(0);                                  /* ����ɨ�� */

        if (key == KEY0_PRES)                               /* �߲����� , Լ5Khz���� */
        {
            dac_creat_sin_buf(2048, 100);                   /* �������Ҳ������� */
            dac_dma_wave_enable(100, 20 - 1, 20 - 1);       /* 500Khz����Ƶ��, 100����, �õ����5KHz�����Ҳ�. */
        }
        else if (key == KEY1_PRES)                          /* �Ͳ����� , Լ50Khz���� */
        {
            dac_creat_sin_buf(2048, 10);                    /* �������Ҳ������� */
            dac_dma_wave_enable(10, 20 - 1, 20 - 1);        /* 500Khz����Ƶ��, 10����, ���Եõ����50KHz�����Ҳ�. */
        }

        adcx = DAC1->DHR12R1;                               /* ��ȡDAC1_OUT1�����״̬ */
        lcd_show_xnum(94, 130, adcx, 4, 16, 0, BLUE);       /* ��ʾDAC�Ĵ���ֵ */

        temp = (float)adcx * (3.3 / 4096);                  /* �õ�DAC��ѹֵ */
        adcx = temp;
        lcd_show_xnum(94, 150, temp, 1, 16, 0, BLUE);       /* ��ʾ��ѹֵ�������� */

        temp -= adcx;
        temp *= 1000;
        lcd_show_xnum(110, 150, temp, 3, 16, 0X80, BLUE);   /* ��ʾ��ѹֵ��С������ */

        adcx = adc_get_result_average(ADC_ADCX_CHY, 20);    /* �õ�ADCͨ��19��ת����� */
        temp = (float)adcx * (3.3 / 65536);                 /* �õ�ADC��ѹֵ(adc��16bit��) */
        adcx = temp;
        lcd_show_xnum(94, 170, temp, 1, 16, 0, BLUE);       /* ��ʾ��ѹֵ�������� */

        temp -= adcx;
        temp *= 1000;
        lcd_show_xnum(110, 170, temp, 3, 16, 0X80, BLUE);   /* ��ʾ��ѹֵ��С������ */

        if (t == 40)                                        /* ��ʱʱ�䵽�� */
        {
            LED0_TOGGLE();                                  /* LED0��˸ */
            t = 0;
        }

        delay_ms(5);
    }
}
