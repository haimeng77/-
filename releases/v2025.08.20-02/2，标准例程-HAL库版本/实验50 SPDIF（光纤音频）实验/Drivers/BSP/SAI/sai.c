/**
 ****************************************************************************************************
 * @file        sai.c
 * @author      ����ԭ���Ŷ�(ALIENTEK)
 * @version     V1.0
 * @date        2022-09-06
 * @brief       SAI ��������
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
 * V1.0 20230105
 * ��һ�η���
 * V1.1 20230106
 * ���sai1_saib_init/sai1_rx_dma_init/sai1_rec_start/sai1_rec_stop�Ⱥ���,֧��¼��
 *
 ****************************************************************************************************
 */
 
#include "./BSP/SAI/sai.h"
#include "./BSP/LCD/ltdc.h"
#include "./SYSTEM/usart/usart.h"
#include "./SYSTEM/delay/delay.h"


SAI_HandleTypeDef g_sai1_a_handle;        /* SAI1 Block A��� */
SAI_HandleTypeDef g_sai1_b_handle;        /* SAI1 Block B��� */
DMA_HandleTypeDef g_sai1_tx_dma_handle;   /* DMA���;�� */
DMA_HandleTypeDef g_sai1_rx_dma_handle;   /* DMA���վ�� */


/**
 * @brief       ����SAIA��DMA����,HAL��û���ṩ�˺���
 * @note        ���������Ҫ�Լ������Ĵ�����дһ��
 * @param       ��
 * @retval      ��
 */
void sai1_saia_dma_enable(void)
{
    uint32_t tempreg = 0;
    tempreg = SAI1_Block_A->CR1;            /* �ȶ�����ǰ������ */
    tempreg |= 1 << 17;                     /* ʹ��DMA */
    SAI1_Block_A->CR1 = tempreg;            /* д��CR1�Ĵ����� */
}

/**
 * @brief       ����SAIB��DMA����,HAL��û���ṩ�˺���
 * @note        ���������Ҫ�Լ������Ĵ�����дһ��
 * @param       ��
 * @retval      ��
 */
void sai1_saib_dma_enable(void)
{
    uint32_t tempreg = 0;
    tempreg = SAI1_Block_B -> CR1;          /* �ȶ�����ǰ������ */
    tempreg |= 1 << 17;                     /* ʹ��DMA */
    SAI1_Block_B -> CR1 = tempreg;          /* д��CR1�Ĵ����� */
}

/**
 * @brief       SAI1 Block A��ʼ��, I2S,�����ֱ�׼
 * @param       mode    : 00,��������;01,��������;10,�ӷ�����;11,�ӽ�����
 * @param       cpol    : 0,ʱ���½���ѡͨ;1,ʱ��������ѡͨ
 * @param       datalen : ���ݴ�С,0/1,δ�õ�,2,8λ;3,10λ;4,16λ;5,20λ;6,24λ;7,32λ.
 * @retval      ��
 */
void sai1_saia_init(uint32_t mode, uint32_t cpol, uint32_t datalen)
{
    HAL_SAI_DeInit(&g_sai1_a_handle);                            /* �����ǰ������ */

    g_sai1_a_handle.Instance = SAI1_Block_A;                     /* SAI1 Bock A */
    g_sai1_a_handle.Init.AudioMode = mode;                       /* ����SAI1����ģʽ */
    g_sai1_a_handle.Init.Synchro = SAI_ASYNCHRONOUS;             /* ��Ƶģ���첽 */
    g_sai1_a_handle.Init.OutputDrive = SAI_OUTPUTDRIVE_ENABLE;   /* ����������Ƶģ����� */
    g_sai1_a_handle.Init.NoDivider = SAI_MASTERDIVIDER_ENABLE;   /* ʹ����ʱ�ӷ�Ƶ��(MCKDIV) */
    g_sai1_a_handle.Init.FIFOThreshold = SAI_FIFOTHRESHOLD_1QF;  /* ����FIFO��ֵ,1/4 FIFO */
    g_sai1_a_handle.Init.MonoStereoMode = SAI_STEREOMODE;        /* ������ģʽ */
    g_sai1_a_handle.Init.Protocol = SAI_FREE_PROTOCOL;           /* ����SAI1Э��Ϊ:����Э��(֧��I2S/LSB/MSB/TDM/PCM/DSP��Э��) */
    g_sai1_a_handle.Init.DataSize = datalen;                     /* �������ݴ�С */
    g_sai1_a_handle.Init.FirstBit = SAI_FIRSTBIT_MSB;            /* ����MSBλ���� */
    g_sai1_a_handle.Init.ClockStrobing = cpol;                   /* ������ʱ�ӵ�����/�½���ѡͨ */
    
    /* ֡���� */
    g_sai1_a_handle.FrameInit.FrameLength = 64;                  /* ����֡����Ϊ64,��ͨ��32��SCK,��ͨ��32��SCK. */
    g_sai1_a_handle.FrameInit.ActiveFrameLength = 32;            /* ����֡ͬ����Ч��ƽ����,��I2Sģʽ��=1/2֡�� */
    g_sai1_a_handle.FrameInit.FSDefinition = SAI_FS_CHANNEL_IDENTIFICATION;/* FS�ź�ΪSOF�ź�+ͨ��ʶ���ź� */
    g_sai1_a_handle.FrameInit.FSPolarity = SAI_FS_ACTIVE_LOW;    /* FS�͵�ƽ��Ч(�½���) */
    g_sai1_a_handle.FrameInit.FSOffset = SAI_FS_BEFOREFIRSTBIT;  /* ��slot0�ĵ�һλ��ǰһλʹ��FS,��ƥ������ֱ�׼ */

    /* SLOT���� */
    g_sai1_a_handle.SlotInit.FirstBitOffset = 0;                 /* slotƫ��(FBOFF)Ϊ0 */
    g_sai1_a_handle.SlotInit.SlotSize = SAI_SLOTSIZE_32B;        /* slot��СΪ32λ */
    g_sai1_a_handle.SlotInit.SlotNumber = 2;                     /* slot��Ϊ2�� */
    g_sai1_a_handle.SlotInit.SlotActive = SAI_SLOTACTIVE_0 | SAI_SLOTACTIVE_1;/* ʹ��slot0��slot1 */
    
    HAL_SAI_Init(&g_sai1_a_handle);                              /* ��ʼ��SAI */
    __HAL_SAI_ENABLE(&g_sai1_a_handle);                          /* ʹ��SAI */
}

/**
 * @brief       SAI Block B��ʼ��,I2S,�����ֱ�׼
 * @param       mode:����ģʽ,��������:SAI_MODEMASTER_TX/SAI_MODEMASTER_RX/SAI_MODESLAVE_TX/SAI_MODESLAVE_RX
 * @param       cpol:������ʱ�ӵ�����/�½���ѡͨ���������ã�SAI_CLOCKSTROBING_FALLINGEDGE/SAI_CLOCKSTROBING_RISINGEDGE
 * @param       datalen:���ݴ�С,�������ã�SAI_DATASIZE_8/10/16/20/24/32
 * @retval      ��
 */
void sai1_saib_init(uint32_t mode, uint32_t cpol, uint32_t datalen)
{
    HAL_SAI_DeInit(&g_sai1_b_handle);                           /* �����ǰ������ */
    g_sai1_b_handle.Instance = SAI1_Block_B;                    /* SAI1 Bock B */
    g_sai1_b_handle.Init.AudioMode = mode;                      /* ����SAI1����ģʽ */
    g_sai1_b_handle.Init.Synchro = SAI_SYNCHRONOUS;             /* ��Ƶģ��ͬ�� */
    g_sai1_b_handle.Init.OutputDrive = SAI_OUTPUTDRIVE_ENABLE;  /* ����������Ƶģ����� */
    g_sai1_b_handle.Init.NoDivider = SAI_MASTERDIVIDER_ENABLE;  /* ʹ����ʱ�ӷ�Ƶ��(MCKDIV) */
    g_sai1_b_handle.Init.FIFOThreshold = SAI_FIFOTHRESHOLD_1QF; /* ����FIFO��ֵ,1/4 FIFO  */
    g_sai1_b_handle.Init.MonoStereoMode = SAI_STEREOMODE;       /* ������ģʽ */
    g_sai1_b_handle.Init.Protocol = SAI_FREE_PROTOCOL;          /* ����SAI1Э��Ϊ:����Э��(֧��I2S/LSB/MSB/TDM/PCM/DSP��Э��) */
    g_sai1_b_handle.Init.DataSize = datalen;                    /* �������ݴ�С */
    g_sai1_b_handle.Init.FirstBit = SAI_FIRSTBIT_MSB;           /* ����MSBλ���� */
    g_sai1_b_handle.Init.ClockStrobing = cpol;                  /* ������ʱ�ӵ�����/�½���ѡͨ */
    
    /* ֡���� */
    g_sai1_b_handle.FrameInit.FrameLength = 64;                 /* ����֡����Ϊ64,��ͨ��32��SCK,��ͨ��32��SCK. */
    g_sai1_b_handle.FrameInit.ActiveFrameLength = 32;           /* ����֡ͬ����Ч��ƽ����,��I2Sģʽ��=1/2֡��. */
    g_sai1_b_handle.FrameInit.FSDefinition = SAI_FS_CHANNEL_IDENTIFICATION;/* FS�ź�ΪSOF�ź�+ͨ��ʶ���ź� */
    g_sai1_b_handle.FrameInit.FSPolarity = SAI_FS_ACTIVE_LOW;   /* FS�͵�ƽ��Ч(�½���) */
    g_sai1_b_handle.FrameInit.FSOffset = SAI_FS_BEFOREFIRSTBIT; /* ��slot0�ĵ�һλ��ǰһλʹ��FS,��ƥ������ֱ�׼ */

    /* SLOT���� */
    g_sai1_b_handle.SlotInit.FirstBitOffset = 0;                /* slotƫ��(FBOFF)Ϊ0 */
    g_sai1_b_handle.SlotInit.SlotSize = SAI_SLOTSIZE_32B;       /* slot��СΪ32λ */
    g_sai1_b_handle.SlotInit.SlotNumber = 2;                    /* slot��Ϊ2�� */
    g_sai1_b_handle.SlotInit.SlotActive = SAI_SLOTACTIVE_0 | SAI_SLOTACTIVE_1;/* ʹ��slot0��slot1  */
    
    HAL_SAI_Init(&g_sai1_b_handle);
    sai1_saib_dma_enable();                                     /* ʹ��SAI��DMA���� */
    __HAL_SAI_ENABLE(&g_sai1_b_handle);                         /* ʹ��SAI */
}

/**
 * @brief       SAI�ײ��������������ã�ʱ��ʹ��
 * @note        �˺����ᱻHAL_SAI_Init()����
 * @param       hsdram:SAI���
 * @retval      ��
 */
void HAL_SAI_MspInit(SAI_HandleTypeDef *hsai)
{
    GPIO_InitTypeDef gpio_init_struct;

    SAI1_SAI_CLK_ENABLE();                     /* ʹ��SAI1ʱ�� */
    SAI1_CLK_GPIO_CLK_ENABLE();                /* CLK����ʱ��ʹ�� */
    SAI1_SCK_GPIO_CLK_ENABLE();                /* SCK����ʱ��ʹ�� */
    SAI1_FSA_GPIO_CLK_ENABLE();                /* FSA����ʱ��ʹ�� */
    SAI1_SDA_GPIO_CLK_ENABLE();                /* SDA����ʱ��ʹ�� */
    SAI1_SDB_GPIO_CLK_ENABLE();                /* SDB����ʱ��ʹ�� */
    
    gpio_init_struct.Pin = SAI1_CLK_GPIO_PIN;  
    gpio_init_struct.Mode = GPIO_MODE_AF_PP;                /* ���츴�� */
    gpio_init_struct.Pull = GPIO_PULLUP;                    /* ���� */
    gpio_init_struct.Speed = GPIO_SPEED_HIGH;               /* ���� */
    gpio_init_struct.Alternate = GPIO_AF6_SAI1;             /* ����ΪSAI */
    HAL_GPIO_Init(SAI1_CLK_GPIO_PORT, &gpio_init_struct);   /* ��ʼ��CLK���� */
    
    gpio_init_struct.Pin = SAI1_SCK_GPIO_PIN;
    HAL_GPIO_Init(SAI1_SCK_GPIO_PORT,  &gpio_init_struct);  /* ��ʼ��SCK���� */
    
    gpio_init_struct.Pin = SAI1_FSA_GPIO_PIN;
    HAL_GPIO_Init(SAI1_FSA_GPIO_PORT,  &gpio_init_struct);  /* ��ʼ��FSA���� */
    
    gpio_init_struct.Pin = SAI1_SDA_GPIO_PIN;
    HAL_GPIO_Init(SAI1_SDA_GPIO_PORT,  &gpio_init_struct);  /* ��ʼ��SDA���� */
    
    gpio_init_struct.Pin = SAI1_SDB_GPIO_PIN;
    HAL_GPIO_Init(SAI1_SDB_GPIO_PORT,  &gpio_init_struct);  /* ��ʼ��SDB���� */
}

/* SAI Block A���������� 
 * �����ʼ��㹫ʽ(��NOMCK=0,OSR=0Ϊǰ��):
 * Fmclk = 256*Fs = sai_x_ker_ck / MCKDIV[5:0]
 * Fs = sai_x_ker_ck / (256 * MCKDIV[5:0])
 * Fsck = Fmclk * (FRL[7:0]+1) / 256 = Fs * (FRL[7:0] + 1)
 * ����:
 * sai_x_ker_ck = (HSE / PLL2DIVM) * (PLL2DIVN + 1) / (PLL2DIVP + 1)
 * HSE:һ��Ϊ25Mhz
 * PLL2DIVM     :1~63,��ʾ1~63��Ƶ
 * PLL2DIVN     :3~511,��ʾ4~512��Ƶ
 * PLL2DIVP     :0~127,��ʾ1~128��Ƶ
 * MCKDIV       :0~63,��ʾ1~63��Ƶ,0Ҳ��1��Ƶ,�Ƽ�����Ϊż��
 * SAI A��Ƶϵ����@PLL2DIVM=25,HSE=25Mhz,��vco����Ƶ��Ϊ1Mhz
 * ���ʽ:
 * ������|(PLL2DIVN+1)|(PLL2DIVP+1)|MCKDIV
 */
const uint16_t SAI_PSC_TBL[][5]=
{
    { 800 , 344, 7, 0, 12 },    /* 8Khz������ */
    { 1102, 429, 2, 18, 2 },    /* 11.025Khz������ */
    { 1600, 344, 7, 0, 6 },     /* 16Khz������ */
    { 2205, 429, 2, 18,1 },     /* 22.05Khz������ */
    { 3200, 344, 7, 0, 3 },     /* 32Khz������ */
    { 4410, 429, 2, 18, 0 },    /* 44.1Khz������ */
    { 4800, 344, 7, 0, 2 },     /* 48Khz������ */
    { 8820, 271, 2, 2, 1 },     /* 88.2Khz������ */
    { 9600, 344, 7, 0, 1 },     /* 96Khz������ */
    { 17640, 271, 2, 2, 0 },    /* 176.4Khz������  */
    { 19200, 344, 7, 0, 0 },    /* 192Khz������ */
};


/**
 * @brief       ����SAIA�Ĳ�����(@MCKEN)
 * @param       samplerate:������, ��λ:Hz
 * @retval      0,���óɹ�
 *              1,�޷�����
 */
uint8_t sai1_samplerate_set(uint32_t samplerate)
{   
    uint8_t i = 0; 
    
    RCC_PeriphCLKInitTypeDef rcc_sai1_sture;  

    for (i = 0; i < (sizeof(SAI_PSC_TBL) / 10); i++)                    /* �����Ĳ������Ƿ����֧�� */
    {
        if ((samplerate / 10) == SAI_PSC_TBL[i][0])
        {
            break;
        }
    }

    if (i == (sizeof(SAI_PSC_TBL) / 10))
    {
        return 1;                       /* �ѱ���Ҳ�Ҳ��� */
    }

    rcc_sai1_sture.PeriphClockSelection = RCC_PERIPHCLK_SAI1;           /* ����ʱ��Դѡ�� */ 
    rcc_sai1_sture.Sai1ClockSelection = RCC_SAI1CLKSOURCE_PLL2;         /* ����PLL2 */
    rcc_sai1_sture.PLL2.PLL2M = 25;                                     /* ����PLL2M */
    rcc_sai1_sture.PLL2.PLL2N = (uint32_t)SAI_PSC_TBL[i][1];            /* ����PLL2N */
    rcc_sai1_sture.PLL2.PLL2P = (uint32_t)SAI_PSC_TBL[i][2];            /* ����PLL2P */
    HAL_RCCEx_PeriphCLKConfig(&rcc_sai1_sture);                         /* ����ʱ�� */

    __HAL_SAI_DISABLE(&g_sai1_a_handle);                                /* �ر�SAI */
    g_sai1_a_handle.Init.AudioFrequency = samplerate;                   /* ���ò���Ƶ�� */
    HAL_SAI_Init(&g_sai1_a_handle);                                     /* ��ʼ��SAI */
    sai1_saia_dma_enable();                                             /* ����SAI��DMA���� */
    __HAL_SAI_ENABLE(&g_sai1_a_handle);                                 /* ����SAI */

    return 0;
}   

/**
 * @brief       SAIA TX DMA����
 * @note        ����Ϊ˫����ģʽ,������DMA��������ж�
 * @param       buf0:M0AR��ַ.
 * @param       buf1:M1AR��ַ.
 * @param       num:ÿ�δ���������
 * @param       width:λ��(�洢��������,ͬʱ����),0,8λ;1,16λ;2,32λ;
 * @retval      0,���óɹ�
 *              1,�޷�����
 */
void sai1_tx_dma_init(uint8_t *buf0, uint8_t *buf1, uint16_t num, uint8_t width)
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

    SAI1_TX_DMA_CLK_ENABLE();                                        /* ʹ��DMAʱ�� */
    __HAL_LINKDMA(&g_sai1_a_handle, hdmatx, g_sai1_tx_dma_handle);   /* ��DMA��SAI��ϵ���� */

    g_sai1_tx_dma_handle.Instance = SAI1_TX_DMASx;                   /* DMA2������3 */
    g_sai1_tx_dma_handle.Init.Request = DMA_REQUEST_SAI1_A;          /* SAI1 Block A */
    g_sai1_tx_dma_handle.Init.Direction = DMA_MEMORY_TO_PERIPH;      /* �洢��������ģʽ */
    g_sai1_tx_dma_handle.Init.PeriphInc = DMA_PINC_DISABLE;          /* ���������ģʽ */
    g_sai1_tx_dma_handle.Init.MemInc = DMA_MINC_ENABLE;              /* �洢������ģʽ */
    g_sai1_tx_dma_handle.Init.PeriphDataAlignment = perwidth;        /* �������ݳ���:16/32λ */
    g_sai1_tx_dma_handle.Init.MemDataAlignment = memwidth;           /* �洢�����ݳ���:16/32λ */
    g_sai1_tx_dma_handle.Init.Mode = DMA_CIRCULAR;                   /* ʹ��ѭ��ģʽ */ 
    g_sai1_tx_dma_handle.Init.Priority = DMA_PRIORITY_HIGH;          /* �����ȼ� */
    g_sai1_tx_dma_handle.Init.FIFOMode = DMA_FIFOMODE_DISABLE;       /* ��ʹ��FIFO */
    g_sai1_tx_dma_handle.Init.MemBurst = DMA_MBURST_SINGLE;          /* �洢������ͻ������ */
    g_sai1_tx_dma_handle.Init.PeriphBurst = DMA_PBURST_SINGLE;       /* ����ͻ�����δ��� */ 
    HAL_DMA_DeInit(&g_sai1_tx_dma_handle);                           /* �������ǰ������ */
    HAL_DMA_Init(&g_sai1_tx_dma_handle);                             /* ��ʼ��DMA */

    HAL_DMAEx_MultiBufferStart(&g_sai1_tx_dma_handle, (uint32_t)buf0, (uint32_t)&SAI1_Block_A->DR, (uint32_t)buf1, num);/* ����˫���� */
    __HAL_DMA_DISABLE(&g_sai1_tx_dma_handle);                        /* �ȹر�DMA */
    delay_us(10);                                                    /* 10us��ʱ����ֹ-O2�Ż�������  */

    __HAL_DMA_ENABLE_IT(&g_sai1_tx_dma_handle, DMA_IT_TC);           /* ������������ж� */
    __HAL_DMA_CLEAR_FLAG(&g_sai1_tx_dma_handle, SAI1_TX_DMASx_FLAG); /* ���DMA��������жϱ�־λ */
    HAL_NVIC_SetPriority(SAI1_TX_DMASx_IRQ, 0, 0);                   /* DMA�ж����ȼ� */
    HAL_NVIC_EnableIRQ(SAI1_TX_DMASx_IRQ);
}

/**
 * @brief       SAIA RX DMA����
 * @note        ����Ϊ˫����ģʽ,������DMA��������ж�
 * @param       buf0:M0AR��ַ.
 * @param       buf1:M1AR��ַ.
 * @param       num:ÿ�δ���������
 * @param       width:λ��(�洢��������,ͬʱ����),0,8λ;1,16λ;2,32λ;
 * @retval      0,���óɹ�
 *              1,�޷�����
 */
void sai1_rx_dma_init(uint8_t *buf0, uint8_t *buf1, uint16_t num, uint8_t width)
{ 
    uint32_t memwidth = 0, perwidth = 0;      /* ����ʹ洢��λ�� */

    switch(width)
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

    SAI1_RX_DMA_CLK_ENABLE();                                          /* ʹ��DMAʱ�� */
    __HAL_LINKDMA(&g_sai1_b_handle,hdmarx,g_sai1_rx_dma_handle);       /* ��DMA��SAI��ϵ���� */

    g_sai1_rx_dma_handle.Instance = SAI1_RX_DMASx;                      /* DMA2������5 */
    g_sai1_rx_dma_handle.Init.Request = DMA_REQUEST_SAI1_B;             /* SAI1 Block B */
    g_sai1_rx_dma_handle.Init.Direction = DMA_PERIPH_TO_MEMORY;         /* ���赽�洢��ģʽ */
    g_sai1_rx_dma_handle.Init.PeriphInc = DMA_PINC_DISABLE;             /* ���������ģʽ */
    g_sai1_rx_dma_handle.Init.MemInc = DMA_MINC_ENABLE;                 /* �洢������ģʽ */
    g_sai1_rx_dma_handle.Init.PeriphDataAlignment = perwidth;           /* �������ݳ���:16/32λ */
    g_sai1_rx_dma_handle.Init.MemDataAlignment = memwidth;              /* �洢�����ݳ���:16/32λ */
    g_sai1_rx_dma_handle.Init.Mode = DMA_CIRCULAR;                      /* ʹ��ѭ��ģʽ */
    g_sai1_rx_dma_handle.Init.Priority = DMA_PRIORITY_MEDIUM;           /* �е����ȼ� */
    g_sai1_rx_dma_handle.Init.FIFOMode = DMA_FIFOMODE_DISABLE;          /* ��ʹ��FIFO */
    g_sai1_rx_dma_handle.Init.MemBurst = DMA_MBURST_SINGLE;             /* �洢������ͻ������ */
    g_sai1_rx_dma_handle.Init.PeriphBurst = DMA_PBURST_SINGLE;          /* ����ͻ�����δ��� */
    HAL_DMA_DeInit(&g_sai1_rx_dma_handle);                              /* �������ǰ������ */
    HAL_DMA_Init(&g_sai1_rx_dma_handle);                                /* ��ʼ��DMA */
    
    HAL_DMAEx_MultiBufferStart(&g_sai1_rx_dma_handle, (uint32_t)&SAI1_Block_B->DR, (uint32_t)buf0, (uint32_t)buf1, num);/* ����˫���� */
    __HAL_DMA_DISABLE(&g_sai1_rx_dma_handle);                           /* �ȹرս���DMA */
    delay_us(10);                                                       /* 10us��ʱ����ֹ-O2�Ż������� */

    __HAL_DMA_CLEAR_FLAG(&g_sai1_rx_dma_handle, SAI1_RX_DMASx_FLAG);    /* ���DMA��������жϱ�־λ */
    __HAL_DMA_ENABLE_IT(&g_sai1_rx_dma_handle, DMA_IT_TC);              /* ������������ж� */

    HAL_NVIC_SetPriority(SAI1_RX_DMASx_IRQ, 0, 1);                      /* DMA�ж����ȼ� */
    HAL_NVIC_EnableIRQ(SAI1_RX_DMASx_IRQ);
}

/* SAI DMA�ص�����ָ�� */
void (*sai1_tx_callback)(void);  /* TX�ص����� */
void (*sai1_rx_callback)(void);  /* RX�ص����� */

/**
 * @brief       DMA2_Stream3�жϷ�����
 * @param       ��
 * @retval      ��
 */
void SAI1_TX_DMASx_IRQHandler(void)
{
    if (__HAL_DMA_GET_FLAG(&g_sai1_tx_dma_handle, SAI1_TX_DMASx_FLAG) != RESET) /* DMA������� */
    {
        __HAL_DMA_CLEAR_FLAG(&g_sai1_tx_dma_handle, SAI1_TX_DMASx_FLAG);        /* ���DMA��������жϱ�־λ */
        if (sai1_tx_callback != NULL)
        {
            sai1_tx_callback();  /* ִ�лص�����,��ȡ���ݵȲ����������洦�� */
        }
    }
}

/**
 * @brief       DMA2_Stream5�жϷ�����
 * @param       ��
 * @retval      ��
 */
void SAI1_RX_DMASx_IRQHandler(void)
{
    if (__HAL_DMA_GET_FLAG(&g_sai1_rx_dma_handle, SAI1_RX_DMASx_FLAG) != RESET) /* DMA������� */
    {
        __HAL_DMA_CLEAR_FLAG(&g_sai1_rx_dma_handle, SAI1_RX_DMASx_FLAG);        /* ���DMA��������жϱ�־λ */
        if (sai1_rx_callback != NULL)
        {
            sai1_rx_callback();  /* ִ�лص�����,��ȡ���ݵȲ����������洦�� */
        }
    }
}

/**
 * @brief       SAI��ʼ����
 * @param       ��
 * @retval      ��
 */
void sai1_play_start(void)
{
    __HAL_DMA_ENABLE(&g_sai1_tx_dma_handle);   /* ����DMA TX���� */
}

/**
 * @brief       �ر�SAI����
 * @param       ��
 * @retval      ��
 */
void sai1_play_stop(void)
{   
    __HAL_DMA_DISABLE(&g_sai1_tx_dma_handle);  /* �������� */
} 

/**
 * @brief       SAI��ʼ¼��
 * @param       ��
 * @retval      ��
 */
void sai1_rec_start(void)
{ 
    __HAL_DMA_ENABLE(&g_sai1_rx_dma_handle);  /* ����DMA RX���� */
}

/**
 * @brief       SAI�ر�¼��
 * @param       ��
 * @retval      ��
 */
void sai1_rec_stop(void)
{   
    __HAL_DMA_DISABLE(&g_sai1_rx_dma_handle); /* ����¼�� */
}









