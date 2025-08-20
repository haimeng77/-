/**
 ****************************************************************************************************
 * @file        main.c
 * @author      ����ԭ���Ŷ�(ALIENTEK)
 * @version     V1.0
 * @date        2022-09-06
 * @brief       ��ͨ��ADC������(26λ�ֱ���) ʵ��
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


int main(void)
{
    uint32_t adcx;
    float temp;

    sys_cache_enable();                       /* ��L1-Cache */
    HAL_Init();                               /* ��ʼ��HAL�� */
    sys_stm32_clock_init(160, 5, 2, 4);       /* ����ʱ��, 400Mhz */
    delay_init(400);                          /* ��ʱ��ʼ�� */
    usart_init(115200);                       /* ���ڳ�ʼ�� */
    mpu_memory_protection();                  /* ������ش洢���� */
    led_init();                               /* ��ʼ��LED */
    sdram_init();                             /* ��ʼ��SDRAM */
    lcd_init();                               /* ��ʼ��LCD */

    /* ��ʼ��ADC, 1024x������, ����λ 
     * 26λADC�ֱ������ֵΪ:67108864, ʵ�������ڷֱ���̫�� ,��λֵ�Ѿ���׼ȷ
     * һ�����ǿ������� ovss=4, ��С16��, ��22λ�ֱ���, ��λֵ������ȶ�һЩ.
     * ��������Ϊ����ʾ26λ������ADCת��Ч��, �ѷֱ��ʵ������, 26λ,���Ҳ���λ. 
     */
    adc_oversample_init(1024 - 1, ADC_RIGHTBITSHIFT_NONE);

    lcd_show_string(30, 50, 200, 16, 16, "STM32", RED);
    lcd_show_string(30, 70, 200, 16, 16, "ADC OverSample TEST", RED);
    lcd_show_string(30, 90, 200, 16, 16, "ATOM@ALIENTEK", RED);

    lcd_show_string(30, 110, 200, 16, 16, "ADC1_CH19_VAL:", BLUE);
    lcd_show_string(30, 130, 200, 16, 16, "ADC1_CH19_VOL:0.000V", BLUE); /* ���ڹ̶�λ����ʾС���� */

    while (1)
    {
        adcx = adc_get_result_average(ADC_ADCX_CHY, 10);    /* ��ȡͨ��19��ת��ֵ��10��ȡƽ�� */
        lcd_show_xnum(142, 110, adcx, 8, 16, 0, BLUE);      /* ��ʾADC�������ԭʼֵ */
 
        temp = (float)adcx * (3.3 / 67108864);              /* ��ȡ�����Ĵ�С����ʵ�ʵ�ѹֵ������3.1111 */
        adcx = temp;                                        /* ��ֵ�������ָ�adcx��������ΪadcxΪ���� */
        lcd_show_xnum(142, 130, adcx, 1, 16, 0, BLUE);      /* ��ʾ��ѹֵ���������֣�3.1111�Ļ������������ʾ3 */

        temp -= adcx;                                       /* ���Ѿ���ʾ����������ȥ��������С�����֣�����3.1111-3=0.1111 */
        temp *= 1000;                                       /* С�����ֳ���1000�����磺0.1111��ת��Ϊ111.1���൱�ڱ�����λС���� */
        lcd_show_xnum(158, 130, temp, 3, 16, 0X80, BLUE);   /* ��ʾС�����֣�ǰ��ת��Ϊ��������ʾ����������ʾ�ľ���111. */

        LED0_TOGGLE();
        delay_ms(200);
    }
}
