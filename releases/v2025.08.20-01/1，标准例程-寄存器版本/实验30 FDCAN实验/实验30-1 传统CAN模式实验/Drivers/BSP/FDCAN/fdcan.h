/**
 ****************************************************************************************************
 * @file        fdcan.h
 * @author      ����ԭ���Ŷ�(ALIENTEK)
 * @version     V1.0
 * @date        2023-03-22
 * @brief       FDCAN ��������
 * @license     Copyright (c) 2022-2032, ������������ӿƼ����޹�˾
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
 * V1.0 20230322
 * ��һ�η���
 *
 ****************************************************************************************************
 */

#ifndef __CAN_H
#define __CAN_H

#include "./SYSTEM/sys/sys.h"
#include "./BSP/FDCAN/stm32h7xx_hal_fdcan.h"

/******************************************************************************************/
/* FDCAN ���� ���� */

#define FDCAN_RX_GPIO_PORT                  GPIOA
#define FDCAN_RX_GPIO_PIN                   SYS_GPIO_PIN11
#define FDCAN_RX_GPIO_CLK_ENABLE()          do{ RCC->AHB4ENR |= 1 << 0; }while(0)       /* PA��ʱ��ʹ�� */ 

#define FDCAN_TX_GPIO_PORT                  GPIOA
#define FDCAN_TX_GPIO_PIN                   SYS_GPIO_PIN12
#define FDCAN_TX_GPIO_CLK_ENABLE()          do{ RCC->AHB4ENR |= 1 << 0; }while(0)       /* PA��ʱ��ʹ�� */ 

/******************************************************************************************/

/* FDCAN1����RX0�ж�ʹ�� */
#define FDCAN1_RX0_INT_ENABLE   0               /* 0,��ʹ��;1,ʹ��. */

/* ��ֹ����HAL�� FDCAN������,���뱨��,����ʵ������3���� */
#define HAL_FDCAN_MODULE_ENABLED                /* ʹ��FDCANģ�� */
#define assert_param(expr)      ((void)0)       /* assert_param���� */
#define HAL_GetTick()           10              /* ����SysTick������ֵ,����û�õ�SysTick */



uint8_t fdcan_init(uint16_t presc, uint8_t tsjw, uint16_t ntsg1, uint8_t ntsg2, uint32_t mode);  /* CAN��ʼ�� */
uint8_t fdcan_send_msg(uint8_t *msg,uint32_t len);                                               /* �������� */
uint8_t fdcan_receive_msg(uint8_t *buf);                                                         /* �������� */

#endif

