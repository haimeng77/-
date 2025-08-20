/**
 ****************************************************************************************************
 * @file        spdif.c
 * @author      ����ԭ���Ŷ�(ALIENTEK)
 * @version     V1.0
 * @date        2022-09-06
 * @brief       SPDIF ��������
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

#include "./BSP/SPDIF/spdif.h"
#include "./SYSTEM/delay/delay.h"


SPDIFRX_HandleTypeDef g_spdifin1_handle;  /* SPDIF IN1��� */
DMA_HandleTypeDef g_spdif_dtdma_handle;   /* SPDIF��Ƶ����DMA */

spdif_struct spdif_dev;                   /* SPDIF���ƽṹ�� */

/**
 * @brief       ��ʼ��SPDIF
 * @param       ��
 * @retval      ��
 */
void spdif_rx_init(void)
{
    spdif_dev.clock = 200000000;                                            /* ����SPDIF CLK��Ƶ��,Ϊ200Mhz,Ҫ֧��192Khz�����ʱ��뱣֤clock��135.2Mhz */

    g_spdifin1_handle.Instance = SPDIFRX;
    g_spdifin1_handle.Init.InputSelection = SPDIFRX_INPUT_IN1;              /* SPDIF����1 */
    g_spdifin1_handle.Init.Retries = SPDIFRX_MAXRETRIES_15;                 /* ͬ���׶��������Դ��� */
    g_spdifin1_handle.Init.WaitForActivity = SPDIFRX_WAITFORACTIVITY_ON;    /* �ȴ�ͬ�� */
    g_spdifin1_handle.Init.ChannelSelection = SPDIFRX_CHANNEL_A;            /* ��������ͨ��A��ȡͨ��״̬ */
    g_spdifin1_handle.Init.DataFormat = SPDIFRX_DATAFORMAT_LSB;             /* �Ҷ��� */
    g_spdifin1_handle.Init.StereoMode = SPDIFRX_STEREOMODE_ENABLE;          /* ʹ��������ģʽ */
    g_spdifin1_handle.Init.PreambleTypeMask = SPDIFRX_PREAMBLETYPEMASK_OFF; /* ��ͷ���Ͳ����Ƶ�SPDIFRX_DR�� */
    g_spdifin1_handle.Init.ChannelStatusMask = SPDIFRX_CHANNELSTATUS_OFF;   /* ͨ��״̬���û�λ�����Ƶ�SPDIFRX_DR�� */
    g_spdifin1_handle.Init.ValidityBitMask = SPDIFRX_VALIDITYMASK_ON;       /* ��Ч��λ�����Ƶ�SPDIFRX_DR�� */
    g_spdifin1_handle.Init.ParityErrorMask = SPDIFRX_PARITYERRORMASK_ON;    /* ��żУ�����λ�����Ƶ�SPDIFRX_DR�� */
    HAL_SPDIFRX_Init(&g_spdifin1_handle);

    g_spdifin1_handle.Instance->CR |= SPDIFRX_CR_RXDMAEN;                   /* SPDIF��Ƶ����ʹ��DMA������ */
    
    /* ʹ��SPDIF������������żУ�����  */
    __HAL_SPDIFRX_ENABLE_IT(&g_spdifin1_handle, SPDIFRX_IT_IFEIE | SPDIFRX_IT_PERRIE); 
}

/**
 * @brief       SPDIF�ײ�IO��ʼ����ʱ��ʹ��
 * @note        �˺����ᱻHAL_SPDIF_Init()����
 * @param       hspdif  : SPDIF���
 * @retval      ��
 */
void HAL_SPDIFRX_MspInit(SPDIFRX_HandleTypeDef *hspdif)
{
    GPIO_InitTypeDef gpio_init_struct;
    
    __HAL_RCC_SPDIFRX_CLK_ENABLE();                     /* ʹ��SPDIF RXʱ�� */
    __HAL_RCC_GPIOG_CLK_ENABLE();                       /* ʹ��GPIOGʱ�� */
 
    /* ��ʼ��PG12��SPDIF IN���� */
    gpio_init_struct.Pin = GPIO_PIN_12;                 /* PG12��SPDIF IN���� */
    gpio_init_struct.Mode = GPIO_MODE_AF_PP;            /* ���� */
    gpio_init_struct.Pull = GPIO_NOPULL;                /* �������� */
    gpio_init_struct.Speed = GPIO_SPEED_FREQ_VERY_HIGH; /* ���� */
    gpio_init_struct.Alternate = GPIO_AF8_SPDIF;        /* ����ΪSPDIF RX */
    HAL_GPIO_Init(GPIOG, &gpio_init_struct);

    HAL_NVIC_SetPriority(SPDIF_RX_IRQn, 1, 0);          /* SPDIF�ж� */
    HAL_NVIC_EnableIRQ(SPDIF_RX_IRQn); 
}

/**
 * @brief       ����SPDIF�Ĺ���ģʽ
 * @param       mode:0,IDLEģʽ
 *                   1,RXͬ��ģʽ
 *                   2,����
 *                   3,��������ģʽ
 * @retval      ��
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
 * @brief       SPDIF RX����DMA����
 * @note        ����Ϊ˫����ģʽ,������DMA��������ж�
 * @param       buf0  : M0AR��ַ.
 * @param       buf1  : M1AR��ַ.
 * @param       num   : ÿ�δ���������
 * @param       width : λ��(�洢��������,ͬʱ����),0,8λ;1,16λ;2,32λ;
 * @retval      ��
 */
void spdif_rx_dma_init(uint32_t *buf0, uint32_t *buf1, uint16_t num, uint8_t width)
{ 
    uint32_t memwidth = 0, perwidth = 0;      /* ����ʹ洢��λ�� */

    switch (width)
    {
        case 0:         /* 8λ */
            memwidth = DMA_MDATAALIGN_BYTE;
            perwidth = DMA_PDATAALIGN_BYTE;
            break;

        case 1:         /* 16λ */
            memwidth = DMA_MDATAALIGN_HALFWORD;
            perwidth = DMA_PDATAALIGN_HALFWORD;
            break;

        case 2:         /* 32λ */
            memwidth = DMA_MDATAALIGN_WORD;
            perwidth = DMA_PDATAALIGN_WORD;
            break;
    }

    __HAL_RCC_DMA1_CLK_ENABLE();                                        /* ʹ��DMA1ʱ�� */
    __HAL_LINKDMA(&g_spdifin1_handle, hdmaDrRx, g_spdif_dtdma_handle);  /* ��DMA��SPDIF��ϵ���� */

    g_spdif_dtdma_handle.Instance = DMA1_Stream1;                       /* DMA1������1 */
    g_spdif_dtdma_handle.Init.Request = DMA_REQUEST_SPDIF_RX_DT;        /* SAI1 Bock B */
    g_spdif_dtdma_handle.Init.Direction = DMA_PERIPH_TO_MEMORY;         /* ���赽�洢��ģʽ */
    g_spdif_dtdma_handle.Init.PeriphInc = DMA_PINC_DISABLE;             /* ���������ģʽ */
    g_spdif_dtdma_handle.Init.MemInc = DMA_MINC_ENABLE;                 /* �洢������ģʽ */
    g_spdif_dtdma_handle.Init.PeriphDataAlignment = perwidth;           /* �������ݳ���:16/32λ */
    g_spdif_dtdma_handle.Init.MemDataAlignment = memwidth;              /* �洢�����ݳ���:16/32λ */
    g_spdif_dtdma_handle.Init.Mode = DMA_CIRCULAR;                      /* ʹ��ѭ��ģʽ  */
    g_spdif_dtdma_handle.Init.Priority = DMA_PRIORITY_MEDIUM;           /* �е����ȼ� */
    g_spdif_dtdma_handle.Init.FIFOMode = DMA_FIFOMODE_DISABLE;          /* ��ʹ��FIFO */
    g_spdif_dtdma_handle.Init.MemBurst = DMA_MBURST_SINGLE;             /* �洢������ͻ������ */
    g_spdif_dtdma_handle.Init.PeriphBurst = DMA_PBURST_SINGLE;          /* ����ͻ�����δ���  */
    HAL_DMA_DeInit(&g_spdif_dtdma_handle);                              /* �������ǰ������ */
    HAL_DMA_Init(&g_spdif_dtdma_handle);                                /* ��ʼ��DMA */
    
    HAL_DMAEx_MultiBufferStart(&g_spdif_dtdma_handle, (uint32_t)&SPDIFRX->DR, (uint32_t)buf0, (uint32_t)buf1, num);/* ����˫���� */
}
 
/**
 * @brief       �ȴ�����ͬ��״̬,ͬ������Ժ��Զ��������״̬
 * @param       ��
 * @retval      ����ֵ:0,δͬ��;1,��ͬ��
 */
uint8_t spdif_rx_waitsync(void)
{
    uint8_t res = 0;
    uint8_t timeout = 0;
    spdif_rx_mode(SPDIF_RX_SYNC);   /* ����Ϊͬ��ģʽ */

    while(1)
    {
        timeout++;
        delay_ms(2);
        if (timeout > 100)
        {
            break;
        }
        if (__HAL_SPDIFRX_GET_FLAG(&g_spdifin1_handle, SPDIFRX_FLAG_SYNCD))/* ͬ�����? */
        {
            res = 1;                    /* ���ͬ����� */
            spdif_rx_mode(SPDIF_RX_RCV);/* �������ģʽ */
            break;
        }
    }
    return res;
}

/**
 * @brief       ��ȡSPDIF RX�յ�����Ƶ������
 * @param       ��
 * @retval      ����ֵ:0,����Ĳ�����
 *              ����ֵ,��Ƶ������
 */
uint32_t spdif_rx_getsamplerate(void)
{
    uint16_t spdif_w5;
    uint32_t samplerate;
    
    spdif_w5 = SPDIFRX->SR >> 16;
    samplerate = (spdif_dev.clock * 5) / (spdif_w5 & 0X7FFF);
    samplerate >>= 6;                     /* ����64 */

    if ((8000 - 1500 <= samplerate) && (samplerate <= 8000 + 1500))
    {
        samplerate = 8000;                /* 8K�Ĳ����� */
    }
    else if ((11025 - 1500 <= samplerate) && (samplerate <= 11025 + 1500))
    {
        samplerate = 11025;               /* 11.025K�Ĳ����� */
    }
    else if ((16000 - 1500 <= samplerate) && (samplerate <= 16000 + 1500))
    {
        samplerate= 16000;                /* 16K�Ĳ����� */
    }
    else if ((22050 - 1500 <= samplerate) && (samplerate <= 22050 + 1500))
    {
        samplerate = 22050;                /* 22.05K�Ĳ����� */
    }
    else if ((32000 - 1500 <= samplerate) && (samplerate <= 32000 + 1500))
    {
        samplerate = 32000;                /* 32K�Ĳ����� */
    }
    else if ((44100 - 1500 <= samplerate) && (samplerate <= 44100 + 1500))
    {
        samplerate = 44100;                /* 44.1K�Ĳ����� */
    }
    else if ((48000 - 1500 <= samplerate) && (samplerate <= 48000 + 1500))
    {
        samplerate = 48000;                /* 48K�Ĳ����� */
    }
    else if ((88200 - 1500 <= samplerate) && (samplerate <= 88200 + 1500))
    {
        samplerate = 88200;                /* 88.2K�Ĳ����� */
    }
    else if ((96000 - 1500 <= samplerate) && (samplerate <= 96000 + 1500))
    {
        samplerate = 96000;                /* 96K�Ĳ����� */
    }
    else if ((176400 - 6000 <= samplerate) && (samplerate <= 176400 + 6000))
    {
        samplerate = 176400;               /* 176.4K�Ĳ����� */
    }
    else if ((192000 - 6000 <= samplerate) && (samplerate <= 192000 + 6000))
    {
        samplerate = 192000;               /* 192K�Ĳ����� */
    }
    else 
    {
        samplerate = 0;
    }

    return samplerate;
}


/* SAI DMA�ص�����ָ�� */
void (*spdif_rx_stop_callback)(void);   /* �ص����� */

/**
 * @brief       SPDIF�����жϷ�����
 * @param       ��
 * @retval      ��
 */
void SPDIF_RX_IRQHandler(void)
{
    /* ������ʱ��ͬ����֡�����ж�,�������ж�һ��Ҫ���� */
    if ( __HAL_SPDIFRX_GET_FLAG(&g_spdifin1_handle, SPDIFRX_FLAG_FERR)||\
        __HAL_SPDIFRX_GET_FLAG(&g_spdifin1_handle, SPDIFRX_FLAG_SERR)||\
        __HAL_SPDIFRX_GET_FLAG(&g_spdifin1_handle, SPDIFRX_FLAG_TERR))
    {  
        spdif_play_stop();                  /* �������󣬹ر�SPDIF���� */
        spdif_rx_stop_callback();           /* ���ûص����� */
        spdif_rx_mode(SPDIF_RX_IDLE);       /* ��������ʱ��ͬ����֡�����ʱ��Ҫ��SPDIFRXENд0������ж� */
    }
    if (__HAL_SPDIFRX_GET_FLAG(&g_spdifin1_handle,SPDIFRX_FLAG_OVR))     /* ������� */
    {
        __HAL_SPDIFRX_CLEAR_IT(&g_spdifin1_handle, SPDIFRX_FLAG_OVR);    /* �����������ж� */
    }
    if (__HAL_SPDIFRX_GET_FLAG(&g_spdifin1_handle,SPDIFRX_FLAG_PERR))    /* ��żУ����� */
    {
        __HAL_SPDIFRX_CLEAR_IT(&g_spdifin1_handle, SPDIFRX_FLAG_PERR);   /* �����żУ����� */
    }
} 

/**
 * @brief       SPDIF��ʼ����
 * @param       ��
 * @retval      ��
 */
void spdif_play_start(void)
{ 
    spdif_dev.consta = 1;                       /* ����Ѿ���SPDIF */
    __HAL_DMA_ENABLE(&g_spdif_dtdma_handle);    /* ����DMA TX���� */
}

/**
 * @brief       SPDIF�ر�
 * @param       ��
 * @retval      ��
 */
void spdif_play_stop(void)
{
    spdif_dev.consta = 0;                       /* ����Ѿ��ر�SPDIF */
    spdif_dev.saisync = 0;                      /* ���ͬ��״̬ */
    __HAL_DMA_DISABLE(&g_spdif_dtdma_handle);   /* �ر�DMA TX���� */
} 

