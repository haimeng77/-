/**
 ****************************************************************************************************
 * @file        spdif.c
 * @author      正点原子团队(ALIENTEK)
 * @version     V1.0
 * @date        2022-09-06
 * @brief       SPDIF 驱动代码
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

#include "./BSP/SPDIF/spdif.h"
#include "./SYSTEM/delay/delay.h"


SPDIFRX_HandleTypeDef g_spdifin1_handle;  /* SPDIF IN1句柄 */
DMA_HandleTypeDef g_spdif_dtdma_handle;   /* SPDIF音频数据DMA */

spdif_struct spdif_dev;                   /* SPDIF控制结构体 */

/**
 * @brief       初始化SPDIF
 * @param       无
 * @retval      无
 */
void spdif_rx_init(void)
{
    spdif_dev.clock = 200000000;                                            /* 设置SPDIF CLK的频率,为200Mhz,要支持192Khz采样率必须保证clock≥135.2Mhz */

    g_spdifin1_handle.Instance = SPDIFRX;
    g_spdifin1_handle.Init.InputSelection = SPDIFRX_INPUT_IN1;              /* SPDIF输入1 */
    g_spdifin1_handle.Init.Retries = SPDIFRX_MAXRETRIES_15;                 /* 同步阶段允许重试次数 */
    g_spdifin1_handle.Init.WaitForActivity = SPDIFRX_WAITFORACTIVITY_ON;    /* 等待同步 */
    g_spdifin1_handle.Init.ChannelSelection = SPDIFRX_CHANNEL_A;            /* 控制流从通道A获取通道状态 */
    g_spdifin1_handle.Init.DataFormat = SPDIFRX_DATAFORMAT_LSB;             /* 右对齐 */
    g_spdifin1_handle.Init.StereoMode = SPDIFRX_STEREOMODE_ENABLE;          /* 使能立体声模式 */
    g_spdifin1_handle.Init.PreambleTypeMask = SPDIFRX_PREAMBLETYPEMASK_OFF; /* 报头类型不复制到SPDIFRX_DR中 */
    g_spdifin1_handle.Init.ChannelStatusMask = SPDIFRX_CHANNELSTATUS_OFF;   /* 通道状态和用户位不复制到SPDIFRX_DR中 */
    g_spdifin1_handle.Init.ValidityBitMask = SPDIFRX_VALIDITYMASK_ON;       /* 有效性位不复制到SPDIFRX_DR中 */
    g_spdifin1_handle.Init.ParityErrorMask = SPDIFRX_PARITYERRORMASK_ON;    /* 奇偶校验错误位不复制到SPDIFRX_DR中 */
    HAL_SPDIFRX_Init(&g_spdifin1_handle);

    g_spdifin1_handle.Instance->CR |= SPDIFRX_CR_RXDMAEN;                   /* SPDIF音频数据使用DMA来接收 */
    
    /* 使能SPDIF的上溢错误和奇偶校验错误  */
    __HAL_SPDIFRX_ENABLE_IT(&g_spdifin1_handle, SPDIFRX_IT_IFEIE | SPDIFRX_IT_PERRIE); 
}

/**
 * @brief       SPDIF底层IO初始化和时钟使能
 * @note        此函数会被HAL_SPDIF_Init()调用
 * @param       hspdif  : SPDIF句柄
 * @retval      无
 */
void HAL_SPDIFRX_MspInit(SPDIFRX_HandleTypeDef *hspdif)
{
    GPIO_InitTypeDef gpio_init_struct;
    
    __HAL_RCC_SPDIFRX_CLK_ENABLE();                     /* 使能SPDIF RX时钟 */
    __HAL_RCC_GPIOG_CLK_ENABLE();                       /* 使能GPIOG时钟 */
 
    /* 初始化PG12，SPDIF IN引脚 */
    gpio_init_struct.Pin = GPIO_PIN_12;                 /* PG12，SPDIF IN引脚 */
    gpio_init_struct.Mode = GPIO_MODE_AF_PP;            /* 复用 */
    gpio_init_struct.Pull = GPIO_NOPULL;                /* 无上下拉 */
    gpio_init_struct.Speed = GPIO_SPEED_FREQ_VERY_HIGH; /* 高速 */
    gpio_init_struct.Alternate = GPIO_AF8_SPDIF;        /* 复用为SPDIF RX */
    HAL_GPIO_Init(GPIOG, &gpio_init_struct);

    HAL_NVIC_SetPriority(SPDIF_RX_IRQn, 1, 0);          /* SPDIF中断 */
    HAL_NVIC_EnableIRQ(SPDIF_RX_IRQn); 
}

/**
 * @brief       设置SPDIF的工作模式
 * @param       mode:0,IDLE模式
 *                   1,RX同步模式
 *                   2,保留
 *                   3,正常工作模式
 * @retval      无
 */
void spdif_rx_mode(uint8_t mode)
{
    if (mode == 0)
    {
       __HAL_SPDIFRX_IDLE(&g_spdifin1_handle); 
    }
    else if (mode == 1)
    {
        __HAL_SPDIFRX_SYNC(&g_spdifin1_handle);
    }
    else if (mode == 3)
    {
        __HAL_SPDIFRX_RCV(&g_spdifin1_handle);
    }
}

/**
 * @brief       SPDIF RX数据DMA配置
 * @note        设置为双缓冲模式,并开启DMA传输完成中断
 * @param       buf0  : M0AR地址.
 * @param       buf1  : M1AR地址.
 * @param       num   : 每次传输数据量
 * @param       width : 位宽(存储器和外设,同时设置),0,8位;1,16位;2,32位;
 * @retval      无
 */
void spdif_rx_dma_init(uint32_t *buf0, uint32_t *buf1, uint16_t num, uint8_t width)
{ 
    uint32_t memwidth = 0, perwidth = 0;      /* 外设和存储器位宽 */

    switch (width)
    {
        case 0:         /* 8位 */
            memwidth = DMA_MDATAALIGN_BYTE;
            perwidth = DMA_PDATAALIGN_BYTE;
            break;

        case 1:         /* 16位 */
            memwidth = DMA_MDATAALIGN_HALFWORD;
            perwidth = DMA_PDATAALIGN_HALFWORD;
            break;

        case 2:         /* 32位 */
            memwidth = DMA_MDATAALIGN_WORD;
            perwidth = DMA_PDATAALIGN_WORD;
            break;
    }

    __HAL_RCC_DMA1_CLK_ENABLE();                                        /* 使能DMA1时钟 */
    __HAL_LINKDMA(&g_spdifin1_handle, hdmaDrRx, g_spdif_dtdma_handle);  /* 将DMA与SPDIF联系起来 */

    g_spdif_dtdma_handle.Instance = DMA1_Stream1;                       /* DMA1数据流1 */
    g_spdif_dtdma_handle.Init.Request = DMA_REQUEST_SPDIF_RX_DT;        /* SAI1 Bock B */
    g_spdif_dtdma_handle.Init.Direction = DMA_PERIPH_TO_MEMORY;         /* 外设到存储器模式 */
    g_spdif_dtdma_handle.Init.PeriphInc = DMA_PINC_DISABLE;             /* 外设非增量模式 */
    g_spdif_dtdma_handle.Init.MemInc = DMA_MINC_ENABLE;                 /* 存储器增量模式 */
    g_spdif_dtdma_handle.Init.PeriphDataAlignment = perwidth;           /* 外设数据长度:16/32位 */
    g_spdif_dtdma_handle.Init.MemDataAlignment = memwidth;              /* 存储器数据长度:16/32位 */
    g_spdif_dtdma_handle.Init.Mode = DMA_CIRCULAR;                      /* 使用循环模式  */
    g_spdif_dtdma_handle.Init.Priority = DMA_PRIORITY_MEDIUM;           /* 中等优先级 */
    g_spdif_dtdma_handle.Init.FIFOMode = DMA_FIFOMODE_DISABLE;          /* 不使用FIFO */
    g_spdif_dtdma_handle.Init.MemBurst = DMA_MBURST_SINGLE;             /* 存储器单次突发传输 */
    g_spdif_dtdma_handle.Init.PeriphBurst = DMA_PBURST_SINGLE;          /* 外设突发单次传输  */
    HAL_DMA_DeInit(&g_spdif_dtdma_handle);                              /* 先清除以前的设置 */
    HAL_DMA_Init(&g_spdif_dtdma_handle);                                /* 初始化DMA */
    
    HAL_DMAEx_MultiBufferStart(&g_spdif_dtdma_handle, (uint32_t)&SPDIFRX->DR, (uint32_t)buf0, (uint32_t)buf1, num);/* 开启双缓冲 */
}
 
/**
 * @brief       等待进入同步状态,同步完成以后自动进入接收状态
 * @param       无
 * @retval      返回值:0,未同步;1,已同步
 */
uint8_t spdif_rx_waitsync(void)
{
    uint8_t res = 0;
    uint8_t timeout = 0;
    spdif_rx_mode(SPDIF_RX_SYNC);   /* 设置为同步模式 */

    while(1)
    {
        timeout++;
        delay_ms(2);
        if (timeout > 100)
        {
            break;
        }
        if (__HAL_SPDIFRX_GET_FLAG(&g_spdifin1_handle, SPDIFRX_FLAG_SYNCD))/* 同步完成? */
        {
            res = 1;                    /* 标记同步完成 */
            spdif_rx_mode(SPDIF_RX_RCV);/* 进入接收模式 */
            break;
        }
    }
    return res;
}

/**
 * @brief       获取SPDIF RX收到的音频采样率
 * @param       无
 * @retval      返回值:0,错误的采样率
 *              其他值,音频采样率
 */
uint32_t spdif_rx_getsamplerate(void)
{
    uint16_t spdif_w5;
    uint32_t samplerate;
    
    spdif_w5 = SPDIFRX->SR >> 16;
    samplerate = (spdif_dev.clock * 5) / (spdif_w5 & 0X7FFF);
    samplerate >>= 6;                     /* 除以64 */

    if ((8000 - 1500 <= samplerate) && (samplerate <= 8000 + 1500))
    {
        samplerate = 8000;                /* 8K的采样率 */
    }
    else if ((11025 - 1500 <= samplerate) && (samplerate <= 11025 + 1500))
    {
        samplerate = 11025;               /* 11.025K的采样率 */
    }
    else if ((16000 - 1500 <= samplerate) && (samplerate <= 16000 + 1500))
    {
        samplerate= 16000;                /* 16K的采样率 */
    }
    else if ((22050 - 1500 <= samplerate) && (samplerate <= 22050 + 1500))
    {
        samplerate = 22050;                /* 22.05K的采样率 */
    }
    else if ((32000 - 1500 <= samplerate) && (samplerate <= 32000 + 1500))
    {
        samplerate = 32000;                /* 32K的采样率 */
    }
    else if ((44100 - 1500 <= samplerate) && (samplerate <= 44100 + 1500))
    {
        samplerate = 44100;                /* 44.1K的采样率 */
    }
    else if ((48000 - 1500 <= samplerate) && (samplerate <= 48000 + 1500))
    {
        samplerate = 48000;                /* 48K的采样率 */
    }
    else if ((88200 - 1500 <= samplerate) && (samplerate <= 88200 + 1500))
    {
        samplerate = 88200;                /* 88.2K的采样率 */
    }
    else if ((96000 - 1500 <= samplerate) && (samplerate <= 96000 + 1500))
    {
        samplerate = 96000;                /* 96K的采样率 */
    }
    else if ((176400 - 6000 <= samplerate) && (samplerate <= 176400 + 6000))
    {
        samplerate = 176400;               /* 176.4K的采样率 */
    }
    else if ((192000 - 6000 <= samplerate) && (samplerate <= 192000 + 6000))
    {
        samplerate = 192000;               /* 192K的采样率 */
    }
    else 
    {
        samplerate = 0;
    }

    return samplerate;
}


/* SAI DMA回调函数指针 */
void (*spdif_rx_stop_callback)(void);   /* 回调函数 */

/**
 * @brief       SPDIF接收中断服务函数
 * @param       无
 * @retval      无
 */
void SPDIF_RX_IRQHandler(void)
{
    /* 发生超时、同步和帧错误中断,这三个中断一定要处理！ */
    if ( __HAL_SPDIFRX_GET_FLAG(&g_spdifin1_handle, SPDIFRX_FLAG_FERR)||\
        __HAL_SPDIFRX_GET_FLAG(&g_spdifin1_handle, SPDIFRX_FLAG_SERR)||\
        __HAL_SPDIFRX_GET_FLAG(&g_spdifin1_handle, SPDIFRX_FLAG_TERR))
    {  
        spdif_play_stop();                  /* 发生错误，关闭SPDIF播放 */
        spdif_rx_stop_callback();           /* 调用回调函数 */
        spdif_rx_mode(SPDIF_RX_IDLE);       /* 当发生超时、同步和帧错误的时候要将SPDIFRXEN写0来清除中断 */
    }
    if (__HAL_SPDIFRX_GET_FLAG(&g_spdifin1_handle,SPDIFRX_FLAG_OVR))     /* 上溢错误 */
    {
        __HAL_SPDIFRX_CLEAR_IT(&g_spdifin1_handle, SPDIFRX_FLAG_OVR);    /* 清除上溢错误中断 */
    }
    if (__HAL_SPDIFRX_GET_FLAG(&g_spdifin1_handle,SPDIFRX_FLAG_PERR))    /* 奇偶校验错误 */
    {
        __HAL_SPDIFRX_CLEAR_IT(&g_spdifin1_handle, SPDIFRX_FLAG_PERR);   /* 清除奇偶校验错误 */
    }
} 

/**
 * @brief       SPDIF开始播放
 * @param       无
 * @retval      无
 */
void spdif_play_start(void)
{ 
    spdif_dev.consta = 1;                       /* 标记已经打开SPDIF */
    __HAL_DMA_ENABLE(&g_spdif_dtdma_handle);    /* 开启DMA TX传输 */
}

/**
 * @brief       SPDIF关闭
 * @param       无
 * @retval      无
 */
void spdif_play_stop(void)
{
    spdif_dev.consta = 0;                       /* 标记已经关闭SPDIF */
    spdif_dev.saisync = 0;                      /* 清空同步状态 */
    __HAL_DMA_DISABLE(&g_spdif_dtdma_handle);   /* 关闭DMA TX传输 */
} 

