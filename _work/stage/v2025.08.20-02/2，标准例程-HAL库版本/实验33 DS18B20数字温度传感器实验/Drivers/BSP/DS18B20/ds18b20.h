/**
 ****************************************************************************************************
 * @file        ds18b20.h
 * @author      ����ԭ���Ŷ�(ALIENTEK)
 * @version     V1.0
 * @date        2022-09-06
 * @brief       DS18B20�����¶ȴ����� ��������
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

#ifndef __DS18B20_H
#define __DS18B20_H

#include "./SYSTEM/sys/sys.h"


/******************************************************************************************/

/* DS18B20���� ���� */
#define DS18B20_DQ_GPIO_PORT                GPIOB
#define DS18B20_DQ_GPIO_PIN                 GPIO_PIN_12
#define DS18B20_DQ_GPIO_CLK_ENABLE()        do{ __HAL_RCC_GPIOB_CLK_ENABLE(); }while(0)     /* PB��ʱ��ʹ�� */

#define DS18B20_DQ_OUT(x)   do{ x ? \
                                HAL_GPIO_WritePin(DS18B20_DQ_GPIO_PORT, DS18B20_DQ_GPIO_PIN, GPIO_PIN_SET) : \
                                HAL_GPIO_WritePin(DS18B20_DQ_GPIO_PORT, DS18B20_DQ_GPIO_PIN, GPIO_PIN_RESET); \
                            }while(0)                                                       /* ���ݶ˿���� */
#define DS18B20_DQ_IN       HAL_GPIO_ReadPin(DS18B20_DQ_GPIO_PORT, DS18B20_DQ_GPIO_PIN)     /* ���ݶ˿����� */

/******************************************************************************************/
                            
uint8_t ds18b20_init(void);             /* ��ʼ��DS18B20 */
uint8_t ds18b20_check(void);            /* ����Ƿ����DS18B20 */
short ds18b20_get_temperature(void);    /* ��ȡ�¶� */

#endif

