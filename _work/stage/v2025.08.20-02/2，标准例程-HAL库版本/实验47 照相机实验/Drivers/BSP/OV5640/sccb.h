/**
 ****************************************************************************************************
 * @file        sccb.h
 * @author      ����ԭ���Ŷ�(ALIENTEK)
 * @version     V1.0
 * @date        2022-09-06
 * @brief       OVϵ������ͷ SCCB ��������
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
 
#ifndef __SCCB_H
#define __SCCB_H

#include "./SYSTEM/sys/sys.h"

/******************************************************************************************/

/* ���� ���� */
#define SCCB_SCL_GPIO_PORT               GPIOB
#define SCCB_SCL_GPIO_PIN                GPIO_PIN_4
#define SCCB_SCL_GPIO_CLK_ENABLE()       do{ __HAL_RCC_GPIOB_CLK_ENABLE(); }while(0)        /* PB��ʱ��ʹ�� */

#define SCCB_SDA_GPIO_PORT               GPIOB
#define SCCB_SDA_GPIO_PIN                GPIO_PIN_3
#define SCCB_SDA_GPIO_CLK_ENABLE()       do{ __HAL_RCC_GPIOB_CLK_ENABLE(); }while(0)        /* PB��ʱ��ʹ�� */

#define OV_SCCB_TYPE_NOD     0          /* ������ÿ�©ģʽ��SCCB�����������裬������Ҫ����������л��ķ�ʽ */
#if OV_SCCB_TYPE_NOD
#define SCCB_SDA_IN()  { GPIOB->MODER &= ~(3 << (3 * 2)); GPIOB->MODER |= 0 << (3 * 2); }   /* PB3 ���� */
#define SCCB_SDA_OUT() { GPIOB->MODER &= ~(3 << (3 * 2)); GPIOB->MODER |= 1 << (3 * 2); }   /* PB3 ��� */
#endif

/******************************************************************************************/

/* IO�������� */
#define SCCB_SCL(x)   do{ x ? \
                          HAL_GPIO_WritePin(SCCB_SCL_GPIO_PORT, SCCB_SCL_GPIO_PIN, GPIO_PIN_SET) : \
                          HAL_GPIO_WritePin(SCCB_SCL_GPIO_PORT, SCCB_SCL_GPIO_PIN, GPIO_PIN_RESET); \
                      }while(0)  /* SCL */

#define SCCB_SDA(x)   do{ x ? \
                          HAL_GPIO_WritePin(SCCB_SDA_GPIO_PORT, SCCB_SDA_GPIO_PIN, GPIO_PIN_SET) : \
                          HAL_GPIO_WritePin(SCCB_SDA_GPIO_PORT, SCCB_SDA_GPIO_PIN, GPIO_PIN_RESET); \
                      }while(0)  /* SDA */

                      
#define SCCB_READ_SDA       HAL_GPIO_ReadPin(SCCB_SDA_GPIO_PORT, SCCB_SDA_GPIO_PIN)         /* ��ȡSDA */

/******************************************************************************************/

/* ����ӿں��� */
void sccb_init(void);
void sccb_start(void);
void sccb_stop(void);
void sccb_nack(void);
uint8_t sccb_send_byte(uint8_t data);
uint8_t sccb_read_byte(void);

#endif

