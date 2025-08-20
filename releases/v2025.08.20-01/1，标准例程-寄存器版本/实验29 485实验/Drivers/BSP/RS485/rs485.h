/**
 ****************************************************************************************************
 * @file        rs485.h
 * @author      ����ԭ���Ŷ�(ALIENTEK)
 * @version     V1.0
 * @date        2023-03-22
 * @brief       RS485 ��������
 * @license     Copyright (c) 2022-2032, ������������ӿƼ����޹�˾
 ****************************************************************************************************
 * @attention
 *
 * ʵ��ƽ̨:����ԭ�� STM32H743������
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

#ifndef __RS485_H
#define __RS485_H

#include "./SYSTEM/sys/sys.h"


/******************************************************************************************/
/* RS485 ���� �� ���� ���� 
 * Ĭ�������RS4852��.
 * ע��: ͨ���޸���10���궨��, ����֧��RS4851~UART7����һ������.
 */

#define RS485_TX_GPIO_PORT                  GPIOA
#define RS485_TX_GPIO_PIN                   SYS_GPIO_PIN2
#define RS485_TX_GPIO_AF                    7                                       /* AF����ѡ�� */
#define RS485_TX_GPIO_CLK_ENABLE()          do{ RCC->AHB4ENR |= 1 << 0; }while(0)   /* PA��ʱ��ʹ�� */

#define RS485_RX_GPIO_PORT                  GPIOA
#define RS485_RX_GPIO_PIN                   SYS_GPIO_PIN3
#define RS485_RX_GPIO_AF                    7                                       /* AF����ѡ�� */
#define RS485_RX_GPIO_CLK_ENABLE()          do{ RCC->AHB4ENR |= 1 << 0; }while(0)   /* PA��ʱ��ʹ�� */

#define RS485_UX                            USART2
#define RS485_UX_IRQn                       USART2_IRQn
#define RS485_UX_IRQHandler                 USART2_IRQHandler
#define RS485_UX_CLK_ENABLE()               do{ RCC->APB1LENR |= 1 << 17; }while(0) /* USART2 ʱ��ʹ�� */

/******************************************************************************************/

/* ����RS485_RE��, ����RS485����/����״̬
 * RS485_RE = 0, �������ģʽ
 * RS485_RE = 1, ���뷢��ģʽ
 */
#define RS485_RE(x)     sys_gpio_pin_set(RS485_RE_GPIO_PORT, RS485_RE_GPIO_PIN, x)


#define RS485_REC_LEN               64          /* �����������ֽ��� 64 */
#define RS485_EN_RX                 1           /* ʹ�ܣ�1��/��ֹ��0��RS485���� */


extern uint8_t g_RS485_rx_buf[RS485_REC_LEN];   /* ���ջ���,���RS485_REC_LEN���ֽ� */
extern uint8_t g_RS485_rx_cnt;                  /* �������ݳ��� */


void rs485_init(uint32_t sclk, uint32_t baudrate);  /* RS485��ʼ�� */
void rs485_send_data(uint8_t *buf, uint8_t len);    /* RS485�������� */
void rs485_receive_data(uint8_t *buf, uint8_t *len);/* RS485�������� */
void rs485_tx_set(uint8_t en);                      /* RS485 TXģʽ */

#endif
















