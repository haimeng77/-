/**
 ****************************************************************************************************
 * @file        rs485.c
 * @author      ����ԭ���Ŷ�(ALIENTEK)
 * @version     V1.0
 * @date        2022-09-06
 * @brief       RS485 ��������
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

#include "./BSP/RS485/rs485.h"
#include "./BSP/PCF8574/pcf8574.h"
#include "./SYSTEM/delay/delay.h"


UART_HandleTypeDef g_rs458_handle;   /* USART2���(����RS485) */

#if EN_USART2_RX                     /* ���ʹ���˽��� */

/* ���ջ����� */
uint8_t g_RS485_rx_buf[64];          /* ���ջ���,���64���ֽ�. */

/* ���յ������ݳ��� */
uint8_t g_RS485_rx_cnt = 0;

void USART2_IRQHandler(void)
{
    uint8_t res;

    if ((__HAL_UART_GET_FLAG(&g_rs458_handle, UART_FLAG_RXNE) != RESET))  /* �����ж� */
    {
        HAL_UART_Receive(&g_rs458_handle, &res, 1, 1000);
        if (g_RS485_rx_cnt < 64)
        {
            g_RS485_rx_buf[g_RS485_rx_cnt] = res;                         /* ��¼���յ���ֵ */
            g_RS485_rx_cnt++;                                             /* ������������1  */
        } 
    } 
}
#endif

/**
 * @brief       RS485��ʼ������
 * @note        �ú�����Ҫ�ǳ�ʼ������
 * @param       bound:������
 * @retval      ��
 */
void rs485_init(uint32_t bound)
{
    /* GPIO�˿����� */
    GPIO_InitTypeDef gpio_init_struct;

    pcf8574_init();                                             /* ��ʼ��PCF8574�����ڿ���RE�� */

    RS485_TX_GPIO_CLK_ENABLE();                                 /* ʹ��GPIOAʱ�� */
    RS485_UX_CLK_ENABLE();                                      /* ʹ��USART2ʱ�� */

    gpio_init_struct.Pin = RS485_TX_GPIO_PIN | RS485_RX_GPIO_PIN; 
    gpio_init_struct.Mode = GPIO_MODE_AF_PP;                    /* �������� */
    gpio_init_struct.Pull = GPIO_PULLUP;                        /* ���� */
    gpio_init_struct.Speed = GPIO_SPEED_HIGH;                   /* ���� */
    gpio_init_struct.Alternate = GPIO_AF7_USART2;               /* ����ΪUSART2 */
    HAL_GPIO_Init(RS485_TX_GPIO_PORT, &gpio_init_struct);       /* ��ʼ��PA2,3 */

    /* USART ��ʼ������ */
    g_rs458_handle.Instance = RS485_UX;                         /* USART2 */
    g_rs458_handle.Init.BaudRate = bound;                       /* ������ */
    g_rs458_handle.Init.WordLength = UART_WORDLENGTH_8B;        /* �ֳ�Ϊ8λ���ݸ�ʽ */
    g_rs458_handle.Init.StopBits = UART_STOPBITS_1;             /* һ��ֹͣλ */
    g_rs458_handle.Init.Parity = UART_PARITY_NONE;              /* ����żУ��λ */
    g_rs458_handle.Init.HwFlowCtl = UART_HWCONTROL_NONE;        /* ��Ӳ������ */
    g_rs458_handle.Init.Mode = UART_MODE_TX_RX;                 /* �շ�ģʽ */
    HAL_UART_Init(&g_rs458_handle);                             /* HAL_UART_Init()��ʹ��USART2 */

    __HAL_UART_DISABLE_IT(&g_rs458_handle, UART_IT_TC);

#if EN_USART2_RX
    __HAL_UART_ENABLE_IT(&g_rs458_handle, UART_IT_RXNE);        /* ���������ж� */
    HAL_NVIC_EnableIRQ(RS485_UX_IRQn);                          /* ʹ��USART1�ж� */
    HAL_NVIC_SetPriority(RS485_UX_IRQn, 3, 3);                  /* ��ռ���ȼ�3�������ȼ�3 */
#endif

    rs485_tx_set(0);                                            /* ����Ϊ����ģʽ */
}

/**
 * @brief       RS485����len���ֽ�
 * @param       buf:�������׵�ַ
 * @param       len:���͵��ֽ���(Ϊ�˺ͱ�����Ľ���ƥ��,���ｨ�鲻Ҫ����64���ֽ�)
 * @retval      ��
 */
void rs485_send_data(uint8_t *buf, uint8_t len)
{
    rs485_tx_set(1);                                            /* ����Ϊ����ģʽ */
    HAL_UART_Transmit(&g_rs458_handle, buf, len, 1000);         /* ����2�������� */
    g_RS485_rx_cnt = 0;
    rs485_tx_set(0);                                            /* ����Ϊ����ģʽ */
}

/**
 * @brief       RS485��ѯ���յ�������
 * @param       buf:���ջ����׵�ַ
 * @param       len:���������ݳ���
 * @retval      ��
 */
void rs485_receive_data(uint8_t *buf, uint8_t *len)
{
    uint8_t rxlen = g_RS485_rx_cnt;
    uint8_t i = 0;
    *len = 0;                             /* Ĭ��Ϊ0 */
    delay_ms(10);                         /* �ȴ�10ms,��������10msû�н��յ�һ������,����Ϊ���ս��� */

    if (rxlen == g_RS485_rx_cnt && rxlen) /* ���յ�������,�ҽ�������� */
    {
        for (i = 0; i < rxlen; i++)
        {
            buf[i] = g_RS485_rx_buf[i];
        }
        *len = g_RS485_rx_cnt;           /* ��¼�������ݳ��� */
        g_RS485_rx_cnt = 0;              /* ���� */
    }
}

/**
 * @brief       RS485ģʽ����.
 * @param       en:0,����;1,����.
 * @retval      ��
 */
void rs485_tx_set(uint8_t en)
{
    pcf8574_write_bit(RS485_RE_IO, en);
}
