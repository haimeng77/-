/**
 ****************************************************************************************************
 * @file        pwr.c
 * @author      ����ԭ���Ŷ�(ALIENTEK)
 * @version     V1.3
 * @date        2022-09-06
 * @brief       �͹���ģʽ ��������
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
 * �޸�˵��
 * V1.0 20220906
 * ��һ�η���
 * V1.1 20220906
 * 1, ֧�ֽ���˯��ģʽ������
 * 2, ����pwr_wkup_key_init��pwr_enter_sleep����
 * V1.2 20220906
 * 1, ֧�ֽ���ֹͣģʽ������
 * 2, ����pwr_enter_stop����
 * V1.3 20220906
 * 1, ֧�ֽ������ģʽ������
 * 2, ����pwr_enter_standby����
 *
 ****************************************************************************************************
 */

#include "./BSP/PWR/pwr.h"
#include "./BSP/LED/led.h"
#include "./BSP/LCD/lcd.h"
#include "./BSP/RTC/rtc.h"
#include "./BSP/KEY/key.h"
#include "./SYSTEM/delay/delay.h"


/**
 * @brief       ��ʼ��PVD��ѹ������
 * @param       pls: ��ѹ�ȼ�(PWR_PVD_detection_level)
 *   @arg       PWR_PVDLEVEL_0,2.2V;
 *   @arg       PWR_PVDLEVEL_1,2.3V;
 *   @arg       PWR_PVDLEVEL_2,2.4V;
 *   @arg       PWR_PVDLEVEL_3,2.5V;
 *   @arg       PWR_PVDLEVEL_4,2.6V;
 *   @arg       PWR_PVDLEVEL_5,2.7V;
 *   @arg       PWR_PVDLEVEL_6,2.8V;
 *   @arg       PWR_PVDLEVEL_7,2.9V;
 * @retval      ��
 */
void pwr_pvd_init(uint32_t pls)
{
    PWR_PVDTypeDef pvd_handle = {0};

    HAL_PWR_EnablePVD();                                /* ʹ��PVDʱ�� */

    pvd_handle.PVDLevel = pls;
    pvd_handle.Mode = PWR_PVD_MODE_IT_RISING_FALLING;
    HAL_PWR_ConfigPVD(&pvd_handle);

    HAL_NVIC_SetPriority(PVD_IRQn, 3, 3);               /* ��ռ���ȼ�3�������ȼ�3 */
    HAL_NVIC_EnableIRQ(PVD_IRQn);
    HAL_PWR_EnablePVD();
}

/**
 * @brief       PVD�жϷ�����
 * @param       ��
 * @retval      ��
 */
void PVD_IRQHandler(void)
{
    HAL_PWR_PVD_IRQHandler();
}

/**
 * @brief       PVD/AVD�жϷ���ص�����
 * @param       ��
 * @retval      ��
 */
void HAL_PWR_PVDCallback(void)
{
    if (__HAL_PWR_GET_FLAG(PWR_FLAG_PVDO))                               /* ��ѹ��PLS��ѡ��ѹ���� */
    {
        lcd_show_string(30, 130, 200, 16, 16, "PVD Low Voltage!", RED);  /* LCD��ʾ��ѹ�� */
        LED1(0);                                                         /* �����̵�, ������ѹ���� */
    }
    else
    {
        lcd_show_string(30, 130, 200, 16, 16, "PVD Voltage OK! ", BLUE); /* LCD��ʾ��ѹ���� */
        LED1(1);                                                         /* ����̵� */
    }
}

/*********************************** ������˯��ģʽʵ�� ���� *******************************************/

/**
 * @brief       �͹���ģʽ�µİ�����ʼ��(���ڻ���˯��ģʽ/ֹͣģʽ/����ģʽ)
 * @param       ��
 * @retval      ��
 */
void pwr_wkup_key_init(void)
{
    GPIO_InitTypeDef gpio_init_struct;
    
    PWR_WKUP_GPIO_CLK_ENABLE();                               /* WKUPʱ��ʹ�� */

    gpio_init_struct.Pin = PWR_WKUP_GPIO_PIN;                 /* WK_UP���� */
    gpio_init_struct.Mode = GPIO_MODE_IT_RISING;              /* �ж�,������ */
    gpio_init_struct.Pull = GPIO_PULLDOWN;                    /* ���� */
    gpio_init_struct.Speed = GPIO_SPEED_FREQ_HIGH;            /* ���� */
    HAL_GPIO_Init(PWR_WKUP_GPIO_PORT, &gpio_init_struct);     /* WK_UP���ų�ʼ�� */

    HAL_NVIC_SetPriority(PWR_WKUP_INT_IRQn, 2, 2);            /* ��ռ���ȼ�2�������ȼ�2 */
    HAL_NVIC_EnableIRQ(PWR_WKUP_INT_IRQn); 
}

/**
 * @brief       WK_UP���� �ⲿ�жϷ������
 * @param       ��
 * @retval      ��
 */
void PWR_WKUP_INT_IRQHandler(void)
{
    HAL_GPIO_EXTI_IRQHandler(PWR_WKUP_GPIO_PIN);
}

/**
 * @brief       ����CPU˯��ģʽ
 * @param       ��
 * @retval      ��
 */
void pwr_enter_sleep(void)
{
    HAL_SuspendTick();  /* ��ͣ�δ�ʱ�ӣ���ֹͨ���δ�ʱ���жϻ��� */
    
    HAL_PWR_EnterSLEEPMode(PWR_MAINREGULATOR_ON, PWR_SLEEPENTRY_WFI); /* ִ��WFIָ��, �������ģʽ */
}

/**
 * @brief       �ⲿ�жϻص�����
 * @param       GPIO_Pin:�ж�������
 * @note        �˺����ᱻPWR_WKUP_INT_IRQHandler()����
 * @retval      ��
 */
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
    if (GPIO_Pin == PWR_WKUP_GPIO_PIN)
    {
        /* HAL_GPIO_EXTI_IRQHandler()�����Ѿ�Ϊ����������жϱ�־λ���������ǽ��˻ص��������Բ����κ��� */
    }
}

/*********************************** ������ֹͣģʽʵ�� ���� *******************************************/

/**
 * @brief       ����ֹͣģʽ
 * @param       ��
 * @retval      ��
 */
void pwr_enter_stop(void)
{
    sys_stm32_clock_init(160, 5, 2, 4);/* ����ʱ��,400Mhz����Ƶ */

    /* ��SVOS3����ֹͣģʽʱ��������ѹ��Ϊ�͹���ģʽ���ȴ��жϻ��� */
    HAL_PWR_EnterSTOPMode(PWR_LOWPOWERREGULATOR_ON, PWR_STOPENTRY_WFI);
}

/*********************************** �����Ǵ���ģʽʵ�� ���� *******************************************/

/**
 * @brief       �������ģʽ
 * @param       ��
 * @retval      ��
 */
void pwr_enter_standby(void)
{
    __HAL_RCC_AHB1_FORCE_RESET();                       /* ��λ����IO�� */

    while (WKUP_KD);                                    /* �ȴ�WK_UP�����ɿ�(����RTC�ж�ʱ,�����WK_UP�ɿ��ٽ������) */

    HAL_PWR_DisableWakeUpPin(PWR_WAKEUP_PIN1);          /* ��ֹ���� */

    __HAL_RCC_BACKUPRESET_FORCE();                      /* ��λ�������� */
    HAL_PWR_EnableBkUpAccess();                         /* ���������ʹ�� */  

    __HAL_PWR_CLEAR_FLAG(PWR_FLAG_WKUP1);
    __HAL_RTC_WRITEPROTECTION_DISABLE(&g_rtc_handle);   /* �ر�RTCд���� */

    /* �ر�RTC����жϣ�������RTCʵ����� */
    __HAL_RTC_WAKEUPTIMER_DISABLE_IT(&g_rtc_handle, RTC_IT_WUT);
    __HAL_RTC_TIMESTAMP_DISABLE_IT(&g_rtc_handle, RTC_IT_TS);
    __HAL_RTC_ALARM_DISABLE_IT(&g_rtc_handle, RTC_IT_ALRA | RTC_IT_ALRB);

    /* ���RTC����жϱ�־λ */
    __HAL_RTC_ALARM_CLEAR_FLAG(&g_rtc_handle, RTC_FLAG_ALRAF | RTC_FLAG_ALRBF);
    __HAL_RTC_TIMESTAMP_CLEAR_FLAG(&g_rtc_handle, RTC_FLAG_TSF); 
    __HAL_RTC_WAKEUPTIMER_CLEAR_FLAG(&g_rtc_handle, RTC_FLAG_WUTF);

    __HAL_RCC_BACKUPRESET_RELEASE();                    /* ��������λ���� */
    __HAL_RTC_WRITEPROTECTION_ENABLE(&g_rtc_handle);    /* ʹ��RTCд���� */
    __HAL_PWR_CLEAR_FLAG(PWR_FLAG_WKUP1);               /* ���Wake_UP��־ */

    HAL_PWR_EnableWakeUpPin(PWR_WAKEUP_PIN1);           /* ʹ��KEY_UP���ŵĻ��ѹ��� */
    PWR->WKUPEPR &= ~(3 << 16);                         /* ���WKUPPUPDԭ�������� */
    PWR->WKUPEPR |= 2 << 16;                            /* WKUPPUPD=10,PA0���� */
    PWR->WKUPCR |= 0X3F << 0;                           /* ���WKUP0~5���ѱ�־ */

    HAL_PWR_EnterSTANDBYMode();                         /* �������ģʽ */
}

