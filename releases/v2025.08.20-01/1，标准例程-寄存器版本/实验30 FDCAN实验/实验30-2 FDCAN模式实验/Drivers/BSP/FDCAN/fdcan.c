/**
 ****************************************************************************************************
 * @file        fdcan.c
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

#include "./BSP/FDCAN/fdcan.h"
#include "./SYSTEM/usart/usart.h"
#include "./SYSTEM/delay/delay.h"
#include "./BSP/LED/led.h"

FDCAN_HandleTypeDef       g_fdcanx_handle;             /* FDCAN1��� */
FDCAN_TxHeaderTypeDef     g_fdcanx_txheade;            /* ������Ϣ */
FDCAN_RxHeaderTypeDef     g_fdcanx_rxheade;            /* ������Ϣ */

/**
 * @brief       FDCAN��ʼ����������Ϊ500KBit/s
 * @note        ����FDCAN��ʱ��ԴΪPLL1Q=200Mhz
 * @param       presc:��Ƶֵ��ȡֵ��Χ1~512
 * @param       tsjw:����ͬ����Ծʱ�䵥Ԫ.��Χ:1~128
 * @param       ntsg1:ȡֵ��Χ2~256
 * @param       ntsg2:ȡֵ��Χ2~128
 * @param       mode��FDCAN_MODE_NORMAL,��ͨģʽ;FDCAN_MODE_EXTERNAL_LOOPBACK,�ػ�ģʽ;
 *              mode:CAN_MODE_NORMAL,��ͨģʽ;CAN_MODE_LOOPBACK,�ػ�ģʽ;
 * @retval      ����ֵ:0,��ʼ��OK;
 *              ����,��ʼ��ʧ��;
 */
uint8_t fdcan_init(uint16_t presc, uint8_t tsjw, uint16_t ntsg1, uint8_t ntsg2, uint32_t mode)
{
    FDCAN_FilterTypeDef fdcan_filterconfig;

    HAL_FDCAN_DeInit(&g_fdcanx_handle);                              /* �������ǰ������ */
    g_fdcanx_handle.Instance = FDCAN1;
    g_fdcanx_handle.Init.FrameFormat = FDCAN_FRAME_FD_BRS;           /* λ���ʱ任FDCANģʽ */
    g_fdcanx_handle.Init.Mode = mode;                                /* ģʽ����  */
    g_fdcanx_handle.Init.AutoRetransmission = ENABLE;                /* ʹ���Զ��ش�����ͳģʽ��һ��Ҫ�رգ����� */
    g_fdcanx_handle.Init.TransmitPause = ENABLE;                     /* ʹ�ܴ�����ͣ */
    g_fdcanx_handle.Init.ProtocolException = DISABLE;                /* �ر�Э���쳣���� */
    
    g_fdcanx_handle.Init.NominalPrescaler = presc;                   /* ��Ƶϵ�� */
    g_fdcanx_handle.Init.NominalSyncJumpWidth = tsjw;                /* ����ͬ����Ծ��� */
    g_fdcanx_handle.Init.NominalTimeSeg1 = ntsg1;                    /* tsg1��Χ��2~256 */
    g_fdcanx_handle.Init.NominalTimeSeg2 = ntsg2;                    /* tsg2��Χ��2~128 */
    
    g_fdcanx_handle.Init.DataPrescaler = 10;                         /* ���ݶη�Ƶϵ����Χ:1~32  */
    g_fdcanx_handle.Init.DataSyncJumpWidth = 16;                     /* ���ݶ�����ͬ����Ծ���1~16 */
    g_fdcanx_handle.Init.DataTimeSeg1 = 2;                           /* ���ݶ�dsg1��Χ:1~32  5 */
    g_fdcanx_handle.Init.DataTimeSeg2 = 1;                           /* ���ݶ�dsg2��Χ:1~16  1 */
    
    g_fdcanx_handle.Init.MessageRAMOffset = 0;                       /* ��ϢRAMƫ�� */
    g_fdcanx_handle.Init.StdFiltersNbr = 28;                         /* ��׼��ϢID�˲������ */
    g_fdcanx_handle.Init.ExtFiltersNbr = 8;                          /* ��չ��ϢID�˲������ */
    g_fdcanx_handle.Init.RxFifo0ElmtsNbr = 1;                        /* ����FIFO0Ԫ�ر�� */
    g_fdcanx_handle.Init.RxFifo0ElmtSize = FDCAN_DATA_BYTES_8;       /* ����FIFO0Ԫ�ش�С��8�ֽ� */
    g_fdcanx_handle.Init.RxBuffersNbr = 0;                           /* ����FIFO0Ԫ�ر�� */
    g_fdcanx_handle.Init.TxEventsNbr = 0;                            /* �����¼���� */
    g_fdcanx_handle.Init.TxBuffersNbr = 0;                           /* ���ͻ����� */
    g_fdcanx_handle.Init.TxFifoQueueElmtsNbr = 1;                    /* ����FIFO����Ԫ�ر�� */
    g_fdcanx_handle.Init.TxFifoQueueMode = FDCAN_TX_FIFO_OPERATION;  /* ����FIFO����ģʽ */
    g_fdcanx_handle.Init.TxElmtSize = FDCAN_DATA_BYTES_8;            /* ���ʹ�С:8�ֽ� */

    if (HAL_FDCAN_Init(&g_fdcanx_handle) != HAL_OK)
    {
        return 1;   /* ��ʼ�� */
    }

    fdcan_filterconfig.IdType = FDCAN_STANDARD_ID;             /* ��׼ID */
    fdcan_filterconfig.FilterIndex = 0;                        /* �˲������� */
    fdcan_filterconfig.FilterType = FDCAN_FILTER_MASK;         /* �˲������� */
    fdcan_filterconfig.FilterConfig = FDCAN_FILTER_TO_RXFIFO0; /* ������0������FIFO0 */
    fdcan_filterconfig.FilterID1 = 0x0000;                     /* 32λID */
    fdcan_filterconfig.FilterID2 = 0x0000;                     /* ���FDCAN����Ϊ��ͳģʽ�Ļ���������32λ���� */

    /* ���������� */
    if (HAL_FDCAN_ConfigFilter(&g_fdcanx_handle, &fdcan_filterconfig) != HAL_OK)
    {
        return 2;   /* �˲�����ʼ�� */
    }
    /* ����ȫ�ֹ�����,�������в�ƥ��ı�׼֡����չ֡ */
    if (HAL_FDCAN_ConfigGlobalFilter(&g_fdcanx_handle, FDCAN_REJECT, FDCAN_REJECT, FDCAN_FILTER_REMOTE, FDCAN_FILTER_REMOTE) != HAL_OK)
    {
        return 3;
    }
    /* ����CAN��Χ�豸 */
    if (HAL_FDCAN_Start(&g_fdcanx_handle) != HAL_OK)
    {
        return 4;
    }

    HAL_FDCAN_ActivateNotification(&g_fdcanx_handle, FDCAN_IT_RX_FIFO0_NEW_MESSAGE, 0);

    return 0;
}

/**
 * @brief       FDCAN�ײ��������������ã�ʱ�����ã��ж�����
 * @note        �˺����ᱻHAL_FDCAN_Init()����
 * @param       hcan:FDCAN���
 * @retval      ��;
 */
void HAL_FDCAN_MspInit(FDCAN_HandleTypeDef *hcan)
{
    if (FDCAN1 == hcan->Instance)
    {
        RCC->APB1HENR |= 1 << 8;                        /* ʹ��FDCAN1ʱ�� */
        FDCAN_RX_GPIO_CLK_ENABLE();                     /* CAN_RX��ʱ��ʹ�� */
        FDCAN_TX_GPIO_CLK_ENABLE();                     /* CAN_TX��ʱ��ʹ�� */

        sys_gpio_set(FDCAN_RX_GPIO_PORT, FDCAN_RX_GPIO_PIN,
                     SYS_GPIO_MODE_AF, SYS_GPIO_OTYPE_PP, SYS_GPIO_SPEED_HIGH, SYS_GPIO_PUPD_PU);   /* FDCAN_TX����ģʽ���� */
        sys_gpio_set(FDCAN_TX_GPIO_PORT, FDCAN_TX_GPIO_PIN,
                     SYS_GPIO_MODE_AF, SYS_GPIO_OTYPE_PP, SYS_GPIO_SPEED_HIGH, SYS_GPIO_PUPD_PU);   /* FDCAN_TX����ģʽ���� */

        sys_gpio_af_set(FDCAN_RX_GPIO_PORT, FDCAN_RX_GPIO_PIN, 9);       /* FDCAN_RX��, AF9 */
        sys_gpio_af_set(FDCAN_TX_GPIO_PORT, FDCAN_TX_GPIO_PIN, 9);       /* FDCAN_TX��, AF9 */


        RCC->D2CCIP1R &= (uint32_t)3 << 28;             /* FDCANSEL[1:0]���� */
        RCC->D2CCIP1R |= (uint32_t)1 << 28;             /* FDCANSEL[1:0]=1,ѡ��pll1_q_ck��ΪFDCANʱ��Դ,һ��Ϊ200Mhz */

#if FDCAN1_RX0_INT_ENABLE
        sys_nvic_init(1, 0, FDCAN1_IT0_IRQn, 2);  /* ��2 */
#endif
    }
}

/**
 * @brief       �˺����ᱻHAL_FDCAN_DeInit����
 * @param       hfdcan:fdcan���
 * @retval      ��
 */
void HAL_FDCAN_MspDeInit(FDCAN_HandleTypeDef *hfdcan)
{
    RCC->APB1HRSTR |= 1 << 8;       /* FDCANRST=1,��λFDCAN */
    delay_ms(10);                   /* �ӳ� */
    RCC->APB1HRSTR &= ~(1 << 8);    /* FDCANRST=0,������λ */
}

/**
 * @brief       can����һ������(�̶���ʽ:IDΪ0X12,��׼֡,����֡)
 * @param       len:���ݳ���(���Ϊ8)
 * @param       msg:����ָ��,���Ϊ8���ֽ�.
 * @retval      ����ֵ:0,�ɹ�;
 * @retval      ����,ʧ��;
 */
uint8_t fdcan_send_msg(uint8_t *msg, uint32_t len)
{
    g_fdcanx_txheade.Identifier = 0x12;                              /* 32λID */
    g_fdcanx_txheade.IdType = FDCAN_STANDARD_ID;                     /* ��׼ID */
    g_fdcanx_txheade.TxFrameType = FDCAN_DATA_FRAME;                 /* ʹ�ñ�׼֡ */
    g_fdcanx_txheade.DataLength = len;                               /* ���ݳ��� */
    g_fdcanx_txheade.ErrorStateIndicator = FDCAN_ESI_ACTIVE;
    g_fdcanx_txheade.BitRateSwitch = FDCAN_BRS_ON;                   /* ���������л� */
    g_fdcanx_txheade.FDFormat = FDCAN_FD_CAN;                        /* FDCANģʽ���� */
    g_fdcanx_txheade.TxEventFifoControl = FDCAN_NO_TX_EVENTS;        /* �޷����¼� */
    g_fdcanx_txheade.MessageMarker = 0;

    if (HAL_FDCAN_AddMessageToTxFifoQ(&g_fdcanx_handle, &g_fdcanx_txheade, msg) != HAL_OK) /* ������Ϣ */
    {
        return 1;
    }

    return 0;
}

/**
 * @brief       can�ڽ������ݲ�ѯ
 * @param       buf:���ݻ�����
 * @retval      ����ֵ:0,�����ݱ��յ�;
 * @retval      ����,���յ����ݳ���;
 */
uint8_t fdcan_receive_msg(uint8_t *buf)
{
    if (HAL_FDCAN_GetRxMessage(&g_fdcanx_handle, FDCAN_RX_FIFO0, &g_fdcanx_rxheade, buf) != HAL_OK)   /* �������� */
    {
        return 0;
    }

    return g_fdcanx_rxheade.DataLength >> 16;
}

#if FDCAN1_RX0_INT_ENABLE           /* ʹ��RX0�ж� */
/**
 * @brief       FDCAN�жϷ�����
 * @param       ��
 * @retval      ��;
 */
void FDCAN1_IT0_IRQHandler(void)
{
    HAL_FDCAN_IRQHandler(&g_fdcanx_handle);
}

/**
 * @brief       FIFO0�ص�����
 * @param       hfdcan:FDCAN���
 * @param       RxFifo0ITs:����FIFO
 * @retval      ��;
 */
void HAL_FDCAN_RxFifo0Callback(FDCAN_HandleTypeDef *hfdcan, uint32_t RxFifo0ITs)
{
    uint8_t i = 0;
    uint8_t rxdata[8];

    if ((RxFifo0ITs & FDCAN_IT_RX_FIFO0_NEW_MESSAGE) != RESET)    /* FIFO1�������ж� */
    {
        /* ��ȡFIFO0�н��յ������� */
        HAL_FDCAN_GetRxMessage(hfdcan, FDCAN_RX_FIFO0, &g_fdcanx_rxheade, rxdata);
        printf("id:%#x\r\n", g_fdcanx_rxheade.Identifier);
        printf("len:%d\r\n", g_fdcanx_rxheade.DataLength >> 16);

        for (i = 0; i < 8; i++)
        {
            printf("rxdata[%d]:%d\r\n", i, rxdata[i]);
        }

        HAL_FDCAN_ActivateNotification(hfdcan, FDCAN_IT_RX_FIFO0_NEW_MESSAGE, 0);
    }
}

#endif






















