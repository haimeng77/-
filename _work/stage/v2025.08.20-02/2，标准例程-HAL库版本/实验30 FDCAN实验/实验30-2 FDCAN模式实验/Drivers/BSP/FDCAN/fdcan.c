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
 * @brief       FDCAN初始化
 * @param       presc   : 分频值，取值范围1~512;
 * @param       tsjw    : 重新同步跳跃时间单元.范围:1~128;
 * @param       ntsg1   : 时间段1的时间单元.取值范围2~256;
 * @param       ntsg2   : 时间段2的时间单元.取值范围2~128;
 * @param       mode    : FDCAN_MODE_NORMAL，普通模式; 
                          FDCAN_MODE_INTERNAL_LOOPBACK，内部回环模式;
                          FDCAN_MODE_EXTERNAL_LOOPBACK，外部回环模式;
                          FDCAN_MODE_RESTRICTED_OPERATION，限制操作模式
                          FDCAN_MODE_BUS_MONITORING，总线监控模式
 * @note        以上5个参数, 除了模式选择其余的参数在函数内部会减1, 所以, 任何一个参数都不能等于0
 *              FDCAN其输入时钟频率为 Fpclk1 = 20Mhz
 *              波特率 = Fpclk1 / ((ntsg1 + ntsg2 + 1) * presc);
 *              我们设置 fdcan_init(10, 8, 31, 8, 1), 则CAN波特率为:
 *              20M / ((31 + 8 + 1) * 10) = 500Kbps
 * @retval      0,  初始化成功; 其他, 初始化失败;
 */
uint8_t fdcan_init(uint16_t presc, uint8_t tsjw, uint16_t ntsg1, uint8_t ntsg2, uint32_t mode) 
{
    FDCAN_FilterTypeDef fdcan_filterconfig;

    HAL_FDCAN_DeInit(&g_fdcanx_handle);                              /* 先清除以前的设置 */
    g_fdcanx_handle.Instance = FDCAN1; 
    g_fdcanx_handle.Init.FrameFormat = FDCAN_FRAME_FD_BRS;           /* 位速率变换FDCAN模式 */
    g_fdcanx_handle.Init.Mode = mode;                                /* 模式设置  */
    g_fdcanx_handle.Init.AutoRetransmission = ENABLE;                /* 使能自动重传！传统模式下一定要关闭！！！ */
    g_fdcanx_handle.Init.TransmitPause = ENABLE;                     /* 使能传输暂停 */
    g_fdcanx_handle.Init.ProtocolException = DISABLE;                /* 关闭协议异常处理 */
    /* FDCAN中仲裁段位速率最高1Mbit/s, 数据段位速率最高8Mbit/s */
    /* 数据段通信速率（仅FDCAN模式需配置） = 20M / (1 + dseg1 + dseg2) = 20M / (2 + 1 + 1) = 5 Mbit/s */
    g_fdcanx_handle.Init.DataPrescaler = 10;                         /* 数据段分频系数范围:1~32  */
    g_fdcanx_handle.Init.DataSyncJumpWidth = 16;                     /* 数据段重新同步跳跃宽度1~16 */
    g_fdcanx_handle.Init.DataTimeSeg1 = 2;                           /* 数据段dsg1范围:1~32  5 */
    g_fdcanx_handle.Init.DataTimeSeg2 = 1;                           /* 数据段dsg2范围:1~16  1 */
    
    /* 仲裁段通信速率（FDCAN与传统CAN均需配置） = 20M / (1 + ntsg1 + ntsg2) = 20M / (31 + 8 + 1) = 500Kbit/s */
    g_fdcanx_handle.Init.NominalPrescaler = presc;                   /* 分频系数 */
    g_fdcanx_handle.Init.NominalSyncJumpWidth = tsjw;                /* 重新同步跳跃宽度 */
    g_fdcanx_handle.Init.NominalTimeSeg1 = ntsg1;                    /* tsg1范围：2~256 */
    g_fdcanx_handle.Init.NominalTimeSeg2 = ntsg2;                    /* tsg2范围：2~128 */
    
    g_fdcanx_handle.Init.MessageRAMOffset = 0;                       /* 信息RAM偏移 */
    g_fdcanx_handle.Init.StdFiltersNbr = 28;                         /* 标准信息ID滤波器编号 */
    g_fdcanx_handle.Init.ExtFiltersNbr = 8;                          /* 扩展信息ID滤波器编号 */
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

    /* 配置CAN过滤器 */
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
    /* 配置全局过滤器,拒收所有不匹配的标准帧或扩展帧 */
    if (HAL_FDCAN_ConfigGlobalFilter(&g_fdcanx_handle, FDCAN_REJECT, FDCAN_REJECT, FDCAN_FILTER_REMOTE, FDCAN_FILTER_REMOTE) != HAL_OK)
    {
        return 3;
    }
    /* 启动CAN外围设备 */
    if (HAL_FDCAN_Start(&g_fdcanx_handle) != HAL_OK)
    {
        return 4;
    }

    HAL_FDCAN_ActivateNotification(&g_fdcanx_handle, FDCAN_IT_RX_FIFO0_NEW_MESSAGE, 0);

    return 0;
}

/**
 * @brief       FDCAN底层驱动，引脚配置，时钟配置，中断配置
 * @note        此函数会被HAL_FDCAN_Init()调用
 * @param       hcan:FDCAN句柄
 * @retval      无;
 */
void HAL_FDCAN_MspInit(FDCAN_HandleTypeDef *hfdcan)
{
    GPIO_InitTypeDef gpio_init_struct = {0};
    RCC_PeriphCLKInitTypeDef fdcan_periphclk = {0};

    if (hfdcan->Instance == FDCAN1)
    {
        /* FDCAN1时钟源配置为PLL1Q */
        fdcan_periphclk.PeriphClockSelection = RCC_PERIPHCLK_FDCAN;
        fdcan_periphclk.FdcanClockSelection = RCC_FDCANCLKSOURCE_PLL;
        HAL_RCCEx_PeriphCLKConfig(&fdcan_periphclk);
        
        __HAL_RCC_FDCAN_CLK_ENABLE();                         /* 使能FDCAN1时钟 */
        FDCAN_RX_GPIO_CLK_ENABLE();                           /* CAN_RX脚时钟使能 */
        FDCAN_TX_GPIO_CLK_ENABLE();                           /* CAN_TX脚时钟使能 */

        gpio_init_struct.Pin = FDCAN_TX_GPIO_PIN;             /* PA12 */
        gpio_init_struct.Mode = GPIO_MODE_AF_PP;              /* 推挽复用 */
        gpio_init_struct.Pull = GPIO_PULLUP;                  /* 上拉 */
        gpio_init_struct.Speed = GPIO_SPEED_FREQ_HIGH;        /* 快速 */
        gpio_init_struct.Alternate = GPIO_AF9_FDCAN1;         /* 复用为FDCAN1 */
        HAL_GPIO_Init(FDCAN_TX_GPIO_PORT, &gpio_init_struct); /* 初始化 */

        gpio_init_struct.Pin = FDCAN_RX_GPIO_PIN;             /* PA11 */
        HAL_GPIO_Init(FDCAN_RX_GPIO_PORT, &gpio_init_struct); /* CAN_RX脚 必须设置成输入模式 */

#if FDCAN1_RX0_INT_ENABLE     
    HAL_NVIC_SetPriority(FDCAN1_IT0_IRQn, 1, 2);
    HAL_NVIC_EnableIRQ(FDCAN1_IT0_IRQn);
#endif
    }
}

/**
 * @brief       FDCAN 发送一组数据
 * @note        发送格式固定为: 标准ID, 数据帧
 * @param       len     :数据长度,取值范围：FDCAN_DLC_BYTES_0 ~ FDCAN_DLC_BYTES_64
 * @param       msg     :数据指针,最大为64个字节
 * @retval      发送状态 0, 成功; 1, 失败;
 */
uint8_t fdcan_send_msg(uint8_t *msg, uint32_t len)
{
    g_fdcanx_txheade.Identifier = 0x12;                              /* 32位ID */
    g_fdcanx_txheade.IdType = FDCAN_STANDARD_ID;                     /* 标准ID */
    g_fdcanx_txheade.TxFrameType = FDCAN_DATA_FRAME;                 /* 使用标准帧 */
    g_fdcanx_txheade.DataLength = len;                               /* 数据长度 */
    g_fdcanx_txheade.ErrorStateIndicator = FDCAN_ESI_ACTIVE;
    g_fdcanx_txheade.BitRateSwitch = FDCAN_BRS_ON;                   /* 开启速率切换 */
    g_fdcanx_txheade.FDFormat = FDCAN_FD_CAN;                        /* FDCAN模式发送 */
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
