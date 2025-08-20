/**
 ****************************************************************************************************
 * @file        dac.c
 * @author      ����ԭ���Ŷ�(ALIENTEK)
 * @version     V1.2
 * @date        2022-09-06
 * @brief       DAC ��������
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
 * V1.1 20230322
 * ����dac_triangular_wave����
 * V1.2 20230322
 * ����dac_dma_wave_init��dac_dma_wave_enable����
 *
 ****************************************************************************************************
 */

#include "./BSP/DAC/dac.h"
#include "./SYSTEM/delay/delay.h"


DAC_HandleTypeDef g_dac_handle;           /* DAC��� */

/**
 * @brief       DAC��ʼ������
 * @note        ������֧��DAC1_OUT1/2ͨ����ʼ��
 *              DAC������ʱ������APB1, ʱ��Ƶ��=100Mhz=10ns
 *              DAC�����buffer�رյ�ʱ��, �������ʱ��: tSETTLING = 2us (H743�����ֲ���д)
 *              ���DAC���������ٶ�ԼΪ:500Khz, ��10����Ϊһ������, ��������50Khz���ҵĲ���
 *
 * @param       outx: Ҫ��ʼ����ͨ��. 1,ͨ��1; 2,ͨ��2
 * @retval      ��
 */
void dac_init(uint8_t outx)
{
    uint8_t t = outx;

    DAC_ChannelConfTypeDef dac_ch_conf;                         /* DACͨ�����ýṹ�� */
    GPIO_InitTypeDef gpio_init_struct;

    __HAL_RCC_DAC12_CLK_ENABLE();                               /* ʹ��DAC12ʱ�ӣ���оƬֻ��DAC1 */
    __HAL_RCC_GPIOA_CLK_ENABLE();                               /* ʹ��DAC OUT1/2��IO��ʱ��(����PA��,PA4/PA5) */

    gpio_init_struct.Pin = (t==1)? GPIO_PIN_4 : GPIO_PIN_5;     /* STM32��Ƭ��, ����PA4=DAC1_OUT1, PA5=DAC1_OUT2 */
    gpio_init_struct.Mode = GPIO_MODE_ANALOG;                   /* ģ�� */
    HAL_GPIO_Init(GPIOA, &gpio_init_struct);

    g_dac_handle.Instance = DAC1;                               /* DAC1�Ĵ�������ַ */
    HAL_DAC_Init(&g_dac_handle);                                /* ��ʼ��DAC */
    
    dac_ch_conf.DAC_Trigger = DAC_TRIGGER_NONE;                 /* ��ʹ�ô������� */
    dac_ch_conf.DAC_OutputBuffer = DAC_OUTPUTBUFFER_DISABLE;    /* DAC1�������ر� */

    switch (outx)
    {
        case 1 :
            HAL_DAC_ConfigChannel(&g_dac_handle, &dac_ch_conf, DAC_CHANNEL_1); /* ����DACͨ��1 */
            HAL_DAC_Start(&g_dac_handle, DAC_CHANNEL_1);                       /* ����DACͨ��1 */
            break;
        case 2 :
            HAL_DAC_ConfigChannel(&g_dac_handle, &dac_ch_conf, DAC_CHANNEL_2); /* ����DACͨ��2 */
            HAL_DAC_Start(&g_dac_handle, DAC_CHANNEL_2);                       /* ����DACͨ��2 */
            break;
        default : break;
    }
}

/**
 * @brief       ����ͨ��1/2�����ѹ
 * @param       outx: 1,ͨ��1; 2,ͨ��2
 * @param       vol : 0~3300,����0~3.3V
 * @retval      ��
 */
void dac_set_voltage(uint8_t outx, uint16_t vol)
{
    double temp = vol;
    temp /= 1000;
    temp = temp * 4096 / 3.3;

    if (temp >= 4096)
    {
        temp = 4095;   /* ���ֵ���ڵ���4096, ��ȡ4095 */
    }

    if (outx == 1)  /* ͨ��1 */
    {
        HAL_DAC_SetValue(&g_dac_handle, DAC_CHANNEL_1, DAC_ALIGN_12B_R, temp); /* 12λ�Ҷ������ݸ�ʽ����DACֵ */
    }
    else            /* ͨ��2 */
    {
        HAL_DAC_SetValue(&g_dac_handle, DAC_CHANNEL_2, DAC_ALIGN_12B_R, temp); /* 12λ�Ҷ������ݸ�ʽ����DACֵ */
    }
}

/*************************************** ������DAC������ǲ����� *****************************************/

/**
 * @brief       ����DAC_OUT1������ǲ�
 * @note        ���Ƶ�� �� 1000 / (dt * samples) Khz, ������dt��С��ʱ��,����С��5usʱ, ����delay_us
 *              ����Ͳ�׼��(���ú���,����ȶ���Ҫʱ��,��ʱ��С��ʱ��,��Щʱ���Ӱ�쵽��ʱ), Ƶ�ʻ�ƫС.
 * 
 * @param       maxval : ���ֵ(0 < maxval < 4096), (maxval + 1)������ڵ���samples/2
 * @param       dt     : ÿ�����������ʱʱ��(��λ: us)
 * @param       samples: ������ĸ���, samples����С�ڵ���(maxval + 1) * 2 , ��maxval���ܵ���0
 * @param       n      : ������θ���,0~65535
 *
 * @retval      ��
 */
void dac_triangular_wave(uint16_t maxval, uint16_t dt, uint16_t samples, uint16_t n)
{
    uint16_t i, j;
    float incval;                           /* ������ */
    float curval;                           /* ��ǰֵ */

    if ((maxval + 1) <= samples)
    {
        return ;     /* ���ݲ��Ϸ� */
    }

    incval = (maxval + 1) / (samples / 2);  /* ��������� */

    for (j = 0; j < n; j++)
    {
        curval = 0;
        HAL_DAC_SetValue(&g_dac_handle, DAC_CHANNEL_1, DAC_ALIGN_12B_R, curval);      /* �����0 */

        for (i = 0; i < (samples / 2); i++)                                           /* ��������� */
        { 
            curval  +=  incval;                                                       /* �µ����ֵ */
            HAL_DAC_SetValue(&g_dac_handle, DAC_CHANNEL_1, DAC_ALIGN_12B_R, curval);  /* �üĴ����������λ���ȶ� */
            delay_us(dt);
        } 
        for (i = 0; i < (samples / 2); i++)                                           /* ����½��� */
        {
            curval  -=  incval;                                                       /* �µ����ֵ */
            HAL_DAC_SetValue(&g_dac_handle, DAC_CHANNEL_1, DAC_ALIGN_12B_R, curval);  /* �üĴ����������λ���ȶ� */
            delay_us(dt);
        }
    }
}

/*************************************** ������DAC������Ҳ����� *****************************************/

DMA_HandleTypeDef g_dma_dac_handle;     /* ����Ҫ����DAC���ݵ�DMA��� */
DAC_HandleTypeDef g_dac_dma_handle;     /* ����DAC��DMA�������� */

extern uint16_t g_dac_sin_buf[4096];    /* �������ݻ����� */

/**
 * @brief       DAC DMA������Ҳ���ʼ������
 * @note        DAC������ʱ������APB1, ʱ��Ƶ��=100Mhz=10ns
 *              DAC�����buffer�رյ�ʱ��, �������ʱ��: tSETTLING = 2us (H743�����ֲ���д)
 *              ���DAC���������ٶ�ԼΪ:500Khz, ��10����Ϊһ������, ��������50Khz���ҵĲ���
 * @param       ��
 * @retval      ��
 */
void dac_dma_wave_init(void)
{
    DAC_ChannelConfTypeDef dac_ch_conf = {0};

    GPIO_InitTypeDef gpio_init_struct;

    __HAL_RCC_GPIOA_CLK_ENABLE();                                           /* DACͨ�����Ŷ˿�ʱ��ʹ�� */
    __HAL_RCC_DAC12_CLK_ENABLE();                                           /* DAC����ʱ��ʹ�� */
    __HAL_RCC_DMA2_CLK_ENABLE();                                            /* DMAʱ��ʹ�� */

    gpio_init_struct.Pin = GPIO_PIN_4;                                      /* PA4 */
    gpio_init_struct.Mode = GPIO_MODE_ANALOG;                               /* ģ�� */
    HAL_GPIO_Init(GPIOA, &gpio_init_struct);                                /* ��ʼ��DAC���� */

    g_dma_dac_handle.Instance = DMA2_Stream6;                               /* ʹ�õ�DAM2 Stream6 */
    g_dma_dac_handle.Init.Request = DMA_REQUEST_DAC1_CH1;                   /* DAC����DMA���� */
    g_dma_dac_handle.Init.Direction = DMA_MEMORY_TO_PERIPH;                 /* �洢��������ģʽ */
    g_dma_dac_handle.Init.PeriphInc = DMA_PINC_DISABLE;                     /* �����ַ��ֹ���� */
    g_dma_dac_handle.Init.MemInc = DMA_MINC_ENABLE;                         /* �洢����ַ���� */
    g_dma_dac_handle.Init.PeriphDataAlignment = DMA_PDATAALIGN_HALFWORD;    /* �������ݳ���:16λ */
    g_dma_dac_handle.Init.MemDataAlignment = DMA_MDATAALIGN_HALFWORD;       /* �洢�����ݳ���:16λ */
    g_dma_dac_handle.Init.Mode = DMA_CIRCULAR;                              /* ѭ��ģʽ */
    g_dma_dac_handle.Init.Priority = DMA_PRIORITY_MEDIUM;                   /* �е����ȼ� */
    g_dma_dac_handle.Init.FIFOMode = DMA_FIFOMODE_DISABLE;                  /* ��ʹ��FIFO */
    HAL_DMA_Init(&g_dma_dac_handle);                                        /* ��ʼ��DMA */

    __HAL_LINKDMA(&g_dac_dma_handle, DMA_Handle1, g_dma_dac_handle);        /* DMA�����DAC������� */

    g_dac_dma_handle.Instance = DAC1;                                       /* ѡ���ĸ�DAC */
    HAL_DAC_Init(&g_dac_dma_handle);                                        /* DAC��ʼ�� */

    dac_ch_conf.DAC_SampleAndHold = DAC_SAMPLEANDHOLD_DISABLE;              /* �رղ�������ģʽ�����ģʽ��Ҫ���ڵ͹��� */
    dac_ch_conf.DAC_Trigger = DAC_TRIGGER_T7_TRGO;                          /* ���ö�ʱ��7���� */
    dac_ch_conf.DAC_OutputBuffer = DAC_OUTPUTBUFFER_ENABLE;                 /* ʹ��������� */
    dac_ch_conf.DAC_ConnectOnChipPeripheral = DAC_CHIPCONNECT_DISABLE;      /* ����DAC���ӵ�Ƭ������ */
    dac_ch_conf.DAC_UserTrimming = DAC_TRIMMING_FACTORY;                    /* ʹ�ó���У׼ */
    HAL_DAC_ConfigChannel(&g_dac_dma_handle, &dac_ch_conf, DAC_CHANNEL_1);  /* DACͨ��������� */
}

/**
 * @brief       DAC DMAʹ�ܲ������
 * @note        TIM7������ʱ��Ƶ��(f)����APB1, f = 100M * 2 = 200Mhz.
 *              DAC����Ƶ�� ftrgo = f / ((psc + 1) * (arr + 1))
 *              ����Ƶ�� = ftrgo / ndtr;
 *
 * @param       ndtr        : DMAͨ�����δ���������
 * @param       arr         : TIM7���Զ���װ��ֵ
 * @param       psc         : TIM7�ķ�Ƶϵ��
 * @retval      ��
 */
void dac_dma_wave_enable(uint16_t ndtr, uint16_t arr, uint16_t psc)
{
    TIM_HandleTypeDef tim7_handle = {0};
    TIM_MasterConfigTypeDef master_config = {0};

    __HAL_RCC_TIM7_CLK_ENABLE();                                         /* TIM7ʱ��ʹ�� */

    tim7_handle.Instance = TIM7;                                         /* ѡ��ʱ��7 */
    tim7_handle.Init.Prescaler = psc;                                    /* ��Ƶϵ�� */
    tim7_handle.Init.CounterMode = TIM_COUNTERMODE_UP;                   /* �������� */
    tim7_handle.Init.Period = arr;                                       /* ��װ��ֵ */
    tim7_handle.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;  /* �Զ���װ */
    HAL_TIM_Base_Init(&tim7_handle);                                     /* ��ʼ����ʱ��7 */

    master_config.MasterOutputTrigger = TIM_TRGO_UPDATE;                 /* ��ʱ�������¼����ڴ��� */
    master_config.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
    HAL_TIMEx_MasterConfigSynchronization(&tim7_handle, &master_config); /* ����TIM7 TRGO */
    HAL_TIM_Base_Start(&tim7_handle);                                    /* ʹ�ܶ�ʱ��7 */

    HAL_DAC_Stop_DMA(&g_dac_dma_handle, DAC_CHANNEL_1);                  /* ��ֹ֮ͣǰ�Ĵ��� */
    HAL_DAC_Start_DMA(&g_dac_dma_handle, DAC_CHANNEL_1, (uint32_t *)g_dac_sin_buf, ndtr, DAC_ALIGN_12B_R);
}


