/**
 ****************************************************************************************************
 * @file        fdcan.c
 * @author      正点原子团队(ALIENTEK)
 * @version     V1.0
 * @date        2022-09-06
 * @brief       FDCAN 驱动代码
 * @license     Copyright (c) 2020-2032, 广州市星翼电子科技有限公司
 ****************************************************************************************************
 * @attention
 *
 * 实验平台:正点原子 阿波罗 H743开发板
 * 在线视频:www.yuanzige.com
 * 技术论坛:www.openedv.com
 * 公司网址:www.alientek.com
 * 购买地址:openedv.taobao.com
 *
 * 修改说明
 * V1.0 20220906
 * 第一次发布
 *
 ****************************************************************************************************
 */

#include "./BSP/FDCAN/fdcan.h"
#include "./SYSTEM/usart/usart.h"
#include "./SYSTEM/delay/delay.h"
#include "./BSP/LED/led.h"

FDCAN_HandleTypeDef       g_fdcanx_handle;             /* FDCAN1句柄 */
FDCAN_TxHeaderTypeDef     g_fdcanx_txheade;            /* 发送消息 */
FDCAN_RxHeaderTypeDef     g_fdcanx_rxheade;            /* 接收消息 */

/**
 * @brief       FDCAN初始化，波特率为500KBit/s
 * @note        配置FDCAN的时钟源为PLL1Q=200Mhz
 * @param       presc:分频值，取值范围1~512
 * @param       tsjw:重新同步跳跃时间单元.范围:1~128
 * @param       ntsg1:取值范围2~256
 * @param       ntsg2:取值范围2~128
 * @param       mode：FDCAN_MODE_NORMAL,普通模式;FDCAN_MODE_EXTERNAL_LOOPBACK,回环模式;
 *              mode:CAN_MODE_NORMAL,普通模式;CAN_MODE_LOOPBACK,回环模式;
 * @retval      返回值:0,初始化OK;
 *              其他,初始化失败;
 */
uint8_t fdcan_init(uint16_t presc, uint8_t tsjw, uint16_t ntsg1, uint8_t ntsg2, uint32_t mode) 
{
    FDCAN_FilterTypeDef fdcan_filterconfig;

    HAL_FDCAN_DeInit(&g_fdcanx_handle);                              /* 先清除以前的设置 */
    g_fdcanx_handle.Instance = FDCAN1; 
    g_fdcanx_handle.Init.FrameFormat = FDCAN_FRAME_CLASSIC;          /* 传统模式 */
    g_fdcanx_handle.Init.Mode = mode;                                /* 模式设置  */
    g_fdcanx_handle.Init.AutoRetransmission = DISABLE;               /* 关闭自动重传！传统模式下一定要关闭！！！  */
    g_fdcanx_handle.Init.TransmitPause = DISABLE;                    /* 关闭传输暂停 */
    g_fdcanx_handle.Init.ProtocolException = DISABLE;                /* 关闭协议异常处理 */
    g_fdcanx_handle.Init.NominalPrescaler = presc;                   /* 分频系数 */
    g_fdcanx_handle.Init.NominalSyncJumpWidth = tsjw;                /* 重新同步跳跃宽度 */
    g_fdcanx_handle.Init.NominalTimeSeg1 = ntsg1;                    /* tsg1范围：2~256 */
    g_fdcanx_handle.Init.NominalTimeSeg2 = ntsg2;                    /* tsg2范围：2~128 */
    g_fdcanx_handle.Init.MessageRAMOffset = 0;                       /* 信息RAM偏移 */
    g_fdcanx_handle.Init.StdFiltersNbr = 0;                          /* 标准信息ID滤波器编号 */
    g_fdcanx_handle.Init.ExtFiltersNbr = 0;                          /* 扩展信息ID滤波器编号 */
    g_fdcanx_handle.Init.RxFifo0ElmtsNbr = 1;                        /* 接收FIFO0元素编号 */
    g_fdcanx_handle.Init.RxFifo0ElmtSize = FDCAN_DATA_BYTES_8;       /* 接收FIFO0元素大小：8字节 */
    g_fdcanx_handle.Init.RxBuffersNbr = 0;                           /* 接收FIFO0元素编号 */
    g_fdcanx_handle.Init.TxEventsNbr = 0;                            /* 发送事件编号 */
    g_fdcanx_handle.Init.TxBuffersNbr = 0;                           /* 发送缓冲编号 */
    g_fdcanx_handle.Init.TxFifoQueueElmtsNbr = 1;                    /* 发送FIFO序列元素编号 */
    g_fdcanx_handle.Init.TxFifoQueueMode = FDCAN_TX_FIFO_OPERATION;  /* 发送FIFO序列模式 */
    g_fdcanx_handle.Init.TxElmtSize = FDCAN_DATA_BYTES_8;            /* 发送大小:8字节 */

    if (HAL_FDCAN_Init(&g_fdcanx_handle) != HAL_OK) 
    {
        return 1;   /* 初始化 */
    }

    fdcan_filterconfig.IdType = FDCAN_STANDARD_ID;                   /* 标准ID */
    fdcan_filterconfig.FilterIndex = 0;                              /* 滤波器索引 */
    fdcan_filterconfig.FilterType = FDCAN_FILTER_MASK;               /* 滤波器类型 */
    fdcan_filterconfig.FilterConfig = FDCAN_FILTER_TO_RXFIFO0;       /* 过滤器0关联到FIFO0 */
    fdcan_filterconfig.FilterID1 = 0x0000;                           /* 32位ID */
    fdcan_filterconfig.FilterID2 = 0x0000;                           /* 如果FDCAN配置为传统模式的话，这里是32位掩码 */
    
     /* 过滤器配置 */
    if (HAL_FDCAN_ConfigFilter(&g_fdcanx_handle, &fdcan_filterconfig) != HAL_OK) 
    {
        return 2;                                                    /* 滤波器初始化 */
    }

    HAL_FDCAN_Start(&g_fdcanx_handle);                               /* 开启FDCAN */
    HAL_FDCAN_ActivateNotification(&g_fdcanx_handle, FDCAN_IT_RX_FIFO0_NEW_MESSAGE, 0);

    return 0;
}

/**
 * @brief       FDCAN底层驱动，引脚配置，时钟配置，中断配置
 * @note        此函数会被HAL_FDCAN_Init()调用
 * @param       hcan:FDCAN句柄
 * @retval      无;
 */
void HAL_FDCAN_MspInit(FDCAN_HandleTypeDef *hcan)
{
    if (FDCAN1 == hcan->Instance)
    {
        GPIO_InitTypeDef gpio_init_struct;
        RCC_PeriphCLKInitTypeDef fdcan_periphclk;

        __HAL_RCC_FDCAN_CLK_ENABLE();                         /* 使能FDCAN1时钟 */
        FDCAN_RX_GPIO_CLK_ENABLE();                           /* CAN_RX脚时钟使能 */
        FDCAN_TX_GPIO_CLK_ENABLE();                           /* CAN_TX脚时钟使能 */

        /* FDCAN1时钟源配置为PLL1Q */
        fdcan_periphclk.PeriphClockSelection = RCC_PERIPHCLK_FDCAN;
        fdcan_periphclk.FdcanClockSelection = RCC_FDCANCLKSOURCE_PLL;
        HAL_RCCEx_PeriphCLKConfig(&fdcan_periphclk);
        
        gpio_init_struct.Pin = FDCAN_RX_GPIO_PIN;             /* PA11 */
        gpio_init_struct.Mode = GPIO_MODE_AF_PP;              /* 推挽复用 */
        gpio_init_struct.Pull = GPIO_PULLUP;                  /* 上拉 */
        gpio_init_struct.Speed = GPIO_SPEED_FAST;             /* 快速 */
        gpio_init_struct.Alternate = GPIO_AF9_FDCAN1;         /* 复用为FDCAN1 */
        HAL_GPIO_Init(FDCAN_RX_GPIO_PORT, &gpio_init_struct); /* 初始化 */

        gpio_init_struct.Pin = FDCAN_TX_GPIO_PIN;             /* PA12 */
        HAL_GPIO_Init(FDCAN_TX_GPIO_PORT, &gpio_init_struct); /* CAN_RX脚 必须设置成输入模式 */

#if FDCAN1_RX0_INT_ENABLE     
    HAL_NVIC_SetPriority(FDCAN1_IT0_IRQn, 1, 2);
    HAL_NVIC_EnableIRQ(FDCAN1_IT0_IRQn);
#endif
    }
}

/**
 * @brief       此函数会被HAL_FDCAN_DeInit调用
 * @param       hfdcan:fdcan句柄
 * @retval      无
 */
void HAL_FDCAN_MspDeInit(FDCAN_HandleTypeDef *hfdcan)
{
    __HAL_RCC_FDCAN_FORCE_RESET();       /* 复位FDCAN1 */
    __HAL_RCC_FDCAN_RELEASE_RESET();     /* 停止复位 */
    
#if FDCAN1_RX0_INT_ENABLE   
    HAL_NVIC_DisableIRQ(FDCAN1_IT0_IRQn);
#endif
}

/**
 * @brief       can发送一组数据(固定格式:ID为0X12,标准帧,数据帧)
 * @param       len:数据长度(最大为8)
 * @param       msg:数据指针,最大为8个字节.
 * @retval      返回值:0,成功;
 * @retval      其他,失败;
 */
uint8_t fdcan_send_msg(uint8_t *msg, uint32_t len)
{
    g_fdcanx_txheade.Identifier = 0x12;                              /* 32位ID */
    g_fdcanx_txheade.IdType = FDCAN_STANDARD_ID;                     /* 标准ID */
    g_fdcanx_txheade.TxFrameType = FDCAN_DATA_FRAME;                 /* 使用标准帧 */
    g_fdcanx_txheade.DataLength = len;                               /* 数据长度 */
    g_fdcanx_txheade.ErrorStateIndicator = FDCAN_ESI_ACTIVE;
    g_fdcanx_txheade.BitRateSwitch = FDCAN_BRS_OFF;                  /* 关闭速率切换 */
    g_fdcanx_txheade.FDFormat = FDCAN_CLASSIC_CAN;                   /* 传统的CAN模式 */
    g_fdcanx_txheade.TxEventFifoControl = FDCAN_NO_TX_EVENTS;        /* 无发送事件 */
    g_fdcanx_txheade.MessageMarker = 0;

    if (HAL_FDCAN_AddMessageToTxFifoQ(&g_fdcanx_handle, &g_fdcanx_txheade, msg) != HAL_OK) /* 发送消息 */
    {
        return 1;
    }

    return 0;
}

/**
 * @brief       can口接收数据查询
 * @param       buf:数据缓存区
 * @retval      返回值:0,无数据被收到;
 * @retval      其他,接收的数据长度;
 */
uint8_t fdcan_receive_msg(uint8_t *buf)
{
    if (HAL_FDCAN_GetRxMessage(&g_fdcanx_handle, FDCAN_RX_FIFO0, &g_fdcanx_rxheade, buf) != HAL_OK)   /* 接收数据 */
    {
        return 0;
    }

    return g_fdcanx_rxheade.DataLength>>16;
}

#if FDCAN1_RX0_INT_ENABLE           /* 使能RX0中断 */
/**
 * @brief       FDCAN中断服务函数
 * @param       无
 * @retval      无;
 */
void FDCAN1_IT0_IRQHandler(void)
{
    HAL_FDCAN_IRQHandler(&g_fdcanx_handle);
}

/**
 * @brief       FIFO0回调函数
 * @param       hfdcan:FDCAN句柄
 * @param       RxFifo0ITs:接收FIFO
 * @retval      无;
 */
void HAL_FDCAN_RxFifo0Callback(FDCAN_HandleTypeDef *hfdcan, uint32_t RxFifo0ITs)
{
    uint8_t i = 0;
    uint8_t rxdata[8];

    if ((RxFifo0ITs & FDCAN_IT_RX_FIFO0_NEW_MESSAGE) != RESET)    /* FIFO1新数据中断 */ 
    {
        /* 提取FIFO0中接收到的数据 */
        HAL_FDCAN_GetRxMessage(hfdcan, FDCAN_RX_FIFO0, &g_fdcanx_rxheade, rxdata);
        printf("id:%#x\r\n", g_fdcanx_rxheade.Identifier);
        printf("len:%d\r\n", g_fdcanx_rxheade.DataLength>>16);
        for (i = 0; i < 8; i++)
        {
            printf("rxdata[%d]:%d\r\n", i, rxdata[i]);
        }
        HAL_FDCAN_ActivateNotification(hfdcan, FDCAN_IT_RX_FIFO0_NEW_MESSAGE, 0);
    }
}

#endif
