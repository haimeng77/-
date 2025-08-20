/**
 ****************************************************************************************************
 * @file        pwr.c
 * @author      ����ԭ���Ŷ�(ALIENTEK)
 * @version     V1.0
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
 *
 ****************************************************************************************************
 */

#include "./BSP/PWR/pwr.h"
#include "./BSP/LED/led.h"
#include "./BSP/LCD/lcd.h"


/**
 * @brief       ��ʼ��PVD��ѹ������
 * @param       pls: ��ѹ�ȼ�
 *   @arg       PWR_PVDLEVEL_0,1.95V;  PWR_PVDLEVEL_1,2.1V
 *   @arg       PWR_PVDLEVEL_2,2.25V;  PWR_PVDLEVEL_3,2.4V;
 *   @arg       PWR_PVDLEVEL_4,2.55V;  PWR_PVDLEVEL_5,2.7V;
 *   @arg       PWR_PVDLEVEL_6,2.85V;  PWR_PVDLEVEL_7,ʹ��PVD_IN���ϵĵ�ѹ(��Vrefint�Ƚ�)
 * @retval      ��
 */
void pwr_pvd_init(uint32_t pls)
{
    PWR_PVDTypeDef pvd_handle = {0};

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



