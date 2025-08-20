/**
 ****************************************************************************************************
 * @file        main.c
 * @author      ����ԭ���Ŷ�(ALIENTEK)
 * @version     V1.0
 * @date        2022-09-06
 * @brief       DSP BasicMath ʵ��
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
#include "./BSP/LCD/ltdc.h"
#include "./BSP/TIMER/btim.h"
#include "./CMSIS/DSP/Include/arm_math.h"


#define DELTA   0.00005f         /* ���ֵ */
extern TIM_HandleTypeDef g_timx_handle;

/**
 * @brief       sin cos ����
 * @param       angle : ��ʼ�Ƕ�
 * @param       times : �������
 * @param       mode  : �Ƿ�ʹ��DSP��
 *   @arg       0 , ��ʹ��DSP��;
 *   @arg       1 , ʹ��DSP��;
 *
 * @retval      ��
 */
uint8_t sin_cos_test(float angle, uint32_t times, uint8_t mode)
{
    float sinx, cosx;
    float result;
    uint32_t i = 0;

    if (mode == 0)
    {
        for (i = 0; i < times; i++)
        {
            cosx = cosf(angle);                 /* ��ʹ��DSP�Ż���sin��cos���� */
            sinx = sinf(angle);
            result = sinx * sinx + cosx * cosx; /* ������Ӧ�õ���1 */
            result = fabsf(result - 1.0f);      /* �Ա���1�Ĳ�ֵ */

            if (result > DELTA)
            {
                return 0XFF;                    /* �ж�ʧ�� */
            }

            angle += 0.001f;                    /* �Ƕ����� */
        }
    }
    else
    {
        for (i = 0; i < times; i++)
        {
            cosx = arm_cos_f32(angle);          /* ʹ��DSP�Ż���sin��cos���� */
            sinx = arm_sin_f32(angle);
            result = sinx * sinx + cosx * cosx; /* ������Ӧ�õ���1 */
            result = fabsf(result - 1.0f);      /* �Ա���1�Ĳ�ֵ */

            if (result > DELTA)
            {
                return 0XFF;                    /* �ж�ʧ�� */
            }

            angle += 0.001f;                    /* �Ƕ����� */
        }
    }

    return 0;                                   /* ������� */
}

uint8_t g_timeout;


int main(void)
{
    float time;
    char buf[50];
    uint8_t res;

    sys_cache_enable();                          /* ��L1-Cache */
    HAL_Init();                                  /* ��ʼ��HAL�� */
    sys_stm32_clock_init(160, 5, 2, 4);          /* ����ʱ��, 400Mhz */
    delay_init(400);                             /* ��ʱ��ʼ�� */
    usart_init(115200);                          /* ���ڳ�ʼ�� */
    mpu_memory_protection();                     /* ������ش洢���� */
    led_init();                                  /* ��ʼ��LED */
    key_init();                                  /* ��ʼ��KEY */
    sdram_init();                                /* ��ʼ��SDRAM */
    lcd_init();                                  /* ��ʼ��LCD */
    btim_timx_int_init(65535, 20000 - 1);        /* 10Khz����Ƶ��,����ʱ6.5�볬�� */

    lcd_show_string(30, 50, 200, 16, 16, "STM32", RED);
    lcd_show_string(30, 70, 200, 16, 16, "DSP BasicMath TEST", RED);
    lcd_show_string(30, 90, 200, 16, 16, "ATOM@ALIENTEK", RED);
    lcd_show_string(30, 120, 200, 16, 16, "No DSP runtime:", RED);         /* ��ʾ��ʾ��Ϣ */
    lcd_show_string(30, 150, 200, 16, 16, "Use DSP runtime:", RED);        /* ��ʾ��ʾ��Ϣ */

    while (1)
    {
        /* ��ʹ��DSP�Ż� */
        BTIM_TIMX_INT->CNT = 0;                  /* ���趨ʱ���ļ�����ֵ */
        g_timeout = 0;
        res = sin_cos_test(PI / 6, 200000, 0);
        time = BTIM_TIMX_INT->CNT + (uint32_t)g_timeout * 65536;
        sprintf(buf, "%0.1fms\r\n", time / 10);

        if (res == 0)
        {
            lcd_show_string(30 + 16 * 8, 120, 100, 16, 16, buf, BLUE);           /* ��ʾ����ʱ�� */
        }
        else
        {
            lcd_show_string(30 + 16 * 8, 120, 100, 16, 16, "error��", BLUE);     /* ��ʾ��ǰ������� */
        }

        /* ʹ��DSP�Ż� */
        BTIM_TIMX_INT->CNT = 0;                  /* ���趨ʱ���ļ�����ֵ */
        g_timeout = 0;
        res = sin_cos_test(PI / 6, 200000, 1);
        time = BTIM_TIMX_INT->CNT + (uint32_t)g_timeout * 65536;
        sprintf(buf, "%0.1fms\r\n", time / 10);

        if (res == 0)
        {
            lcd_show_string(30 + 16 * 8, 150, 100, 16, 16, buf, BLUE);           /* ��ʾ����ʱ�� */
        }
        else
        {
            lcd_show_string(30 + 16 * 8, 150, 100, 16, 16, "error��", BLUE);     /* ��ʾ���� */
        }

        LED0_TOGGLE();
    }
}

