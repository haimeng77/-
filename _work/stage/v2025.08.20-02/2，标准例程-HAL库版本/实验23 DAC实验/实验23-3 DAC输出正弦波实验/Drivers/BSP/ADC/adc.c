/**
 ****************************************************************************************************
 * @file        adc.c
 * @author      ����ԭ���Ŷ�(ALIENTEK)
 * @version     V1.3
 * @date        2023-03-22
 * @brief       ADC ��������
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
 * V1.1 20230322
 * 1,֧��ADC��ͨ��DMA�ɼ� 
 * 2,����adc_dma_init��adc_dma_enable����.
 * V1.2 20230322
 * 1,֧��ADC��ͨ��DMA�ɼ� 
 * 2,����adc_nch_dma_init����.
 * V1.3 20230322
 * 1,֧��ADC��ͨ��������
 * 2,����adc_oversample_init����.
 *
 ****************************************************************************************************
 */

#include "./BSP/ADC/adc.h"
#include "./SYSTEM/delay/delay.h"


ADC_HandleTypeDef g_adc_handle;           /* ADC��� */

/**
 * @brief       ADC��ʼ������
 * @param       ��
 * @retval      ��
 */
void adc_init(void)
{ 
    g_adc_handle.Instance = ADC_ADCX;
    g_adc_handle.Init.ClockPrescaler = ADC_CLOCK_ASYNC_DIV2;                /* ��Ƶ��ADCCLK=PER_CK/2=64/2=32MHZ */
    g_adc_handle.Init.Resolution = ADC_RESOLUTION_16B;                      /* 16λģʽ */
    g_adc_handle.Init.ScanConvMode = DISABLE;                               /* ��ɨ��ģʽ */
    g_adc_handle.Init.EOCSelection = ADC_EOC_SINGLE_CONV;                   /* �ر�EOC�ж� */
    g_adc_handle.Init.LowPowerAutoWait = DISABLE;                           /* �Զ��͹��Ĺر� */
    g_adc_handle.Init.ContinuousConvMode = DISABLE;                         /* �ر�����ת�� */
    g_adc_handle.Init.NbrOfConversion = 1;                                  /* 1��ת���ڹ��������� Ҳ����ֻת����������1 */
    g_adc_handle.Init.DiscontinuousConvMode = DISABLE;                      /* ��ֹ����������ģʽ */
    g_adc_handle.Init.NbrOfDiscConversion = 0;                              /* ����������ͨ����Ϊ0 */
    g_adc_handle.Init.ExternalTrigConv = ADC_SOFTWARE_START;                /* ������� */
    g_adc_handle.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE; /* ʹ��������� */
    g_adc_handle.Init.Overrun = ADC_OVR_DATA_OVERWRITTEN;                   /* ���µ����ݵ�����ֱ�Ӹ��ǵ������� */
    g_adc_handle.Init.OversamplingMode = DISABLE;                           /* �������ر� */
    g_adc_handle.Init.ConversionDataManagement = ADC_CONVERSIONDATA_DR;     /* ����ͨ�������ݽ���������DR�Ĵ������� */
    HAL_ADC_Init(&g_adc_handle);                                            /* ��ʼ��  */

    HAL_ADCEx_Calibration_Start(&g_adc_handle, ADC_CALIB_OFFSET, ADC_SINGLE_ENDED); /* ADCУ׼ */
}

/**
 * @brief       ADC�ײ��������������ã�ʱ��ʹ��
 * @param       �˺����ᱻHAL_ADC_Init()����
 * @param       hadc:ADC���
 * @retval      ��
 */
void HAL_ADC_MspInit(ADC_HandleTypeDef *hadc)
{
    if (hadc->Instance == ADC_ADCX)
    {
        GPIO_InitTypeDef gpio_init_struct;

        ADC_ADCX_CHY_CLK_ENABLE();                               /* ʹ��ADC1/2ʱ�� */
        ADC_ADCX_CHY_GPIO_CLK_ENABLE();                          /* ����GPIOAʱ�� */

        __HAL_RCC_ADC_CONFIG(RCC_ADCCLKSOURCE_CLKP);             /* ADC����ʱ��ѡ�� */
        
        gpio_init_struct.Pin = ADC_ADCX_CHY_GPIO_PIN;            /* PA5 */
        gpio_init_struct.Mode = GPIO_MODE_ANALOG;                /* ģ�� */
        gpio_init_struct.Pull = GPIO_NOPULL;                     /* ���������� */
        HAL_GPIO_Init(ADC_ADCX_CHY_GPIO_PORT, &gpio_init_struct);
    }
}

/**
 * @brief       ���ADCת����Ľ��
 * @param       ch: ͨ��ֵ 0~19��ȡֵ��ΧΪ��ADC_CHANNEL_0~ADC_CHANNEL_19
 * @retval      ��
 */
uint32_t adc_get_result(uint32_t ch)
{
    ADC_ChannelConfTypeDef adc_ch_conf = {0};

    adc_ch_conf.Channel = ch;                              /* ͨ�� */
    adc_ch_conf.Rank = ADC_REGULAR_RANK_1;                 /* ���� */
    adc_ch_conf.SamplingTime = ADC_SAMPLETIME_810CYCLES_5; /* ����ʱ�䣬��������������: 810.5��ADC���� */
    adc_ch_conf.SingleDiff = ADC_SINGLE_ENDED;             /* ���߲ɼ� */
    adc_ch_conf.OffsetNumber = ADC_OFFSET_NONE;            /* ��ʹ��ƫ������ͨ�� */
    adc_ch_conf.Offset = 0;                                /* ƫ����Ϊ0 */
    HAL_ADC_ConfigChannel(&g_adc_handle, &adc_ch_conf);    /* ͨ������ */

    HAL_ADC_Start(&g_adc_handle);                          /* ����ADC */
    HAL_ADC_PollForConversion(&g_adc_handle, 10);          /* ��ѯת�� */
    return HAL_ADC_GetValue(&g_adc_handle);                /* �������һ��ADC1�������ת����� */
}

/**
 * @brief       ��ȡָ��ͨ����ת��ֵ��ȡtimes��,Ȼ��ƽ��
 * @param       ch: ͨ��ֵ 0~19��ȡֵ��ΧΪ��ADC_CHANNEL_0~ADC_CHANNEL_19
 * @param       times:��ȡ����
 * @retval      ����ֵ:ͨ��ch��times��ת�����ƽ��ֵ
 */
uint32_t adc_get_result_average(uint32_t ch, uint8_t times)
{
    uint32_t temp_val = 0;
    uint8_t t;

    for (t = 0; t < times; t++)    /* ��ȡtimes������ */
    {
        temp_val += adc_get_result(ch);
        delay_ms(5);
    }

    return temp_val / times;       /* ����ƽ��ֵ */
} 

/*************************************** �����ǵ�ͨ��ADC�ɼ�(DMA��ȡ)���� *****************************************/

ADC_HandleTypeDef g_adc_dma_handle;     /* ��DMA������ADC��� */
DMA_HandleTypeDef g_dma_adc_handle;     /* ��ADC������DMA��� */

uint8_t g_adc_dma_sta = 0;              /* DMA����״̬��־, 0,δ���; 1, ����� */

/**
 * @brief       ADC DMA��ȡ ��ʼ������
 * @param       par         : �����ַ
 * @param       mar         : �洢����ַ
 * @retval      ��
 */
void adc_dma_init(uint32_t par, uint32_t mar)
{
    GPIO_InitTypeDef gpio_init_struct;
    ADC_ChannelConfTypeDef adc_ch_conf = {0};

    ADC_ADCX_CHY_GPIO_CLK_ENABLE();                                             /* ����ADCͨ��IO����ʱ�� */
    ADC_ADCX_CHY_CLK_ENABLE();                                                  /* ʹ��ADC1/2ʱ�� */

    if ((uint32_t)ADC_ADCX_DMASx > (uint32_t)DMA2)                              /* �õ���ǰstream������DMA2����DMA1 */
    {
        __HAL_RCC_DMA2_CLK_ENABLE();                                            /* DMA2ʱ��ʹ�� */
    }
    else 
    {
        __HAL_RCC_DMA1_CLK_ENABLE();                                            /* DMA1ʱ��ʹ�� */
    }

    __HAL_RCC_ADC_CONFIG(RCC_ADCCLKSOURCE_CLKP);                                /* ADC����ʱ��ѡ�� */

    /* ��ʼ��ADC�ɼ�ͨ����Ӧ��IO���� */
    gpio_init_struct.Pin = ADC_ADCX_CHY_GPIO_PIN;                               /* ADCͨ��IO���� */
    gpio_init_struct.Mode = GPIO_MODE_ANALOG;                                   /* ģ�� */
    HAL_GPIO_Init(ADC_ADCX_CHY_GPIO_PORT, &gpio_init_struct);

    /* ��ʼ��DMA */
    g_dma_adc_handle.Instance = ADC_ADCX_DMASx;                                 /* ����DMA������ */
    g_dma_adc_handle.Init.Request = ADC_ADCX_DMASx_REQ;                         /* ����ѡ��DMA_REQUEST_ADC1 */
    g_dma_adc_handle.Init.Direction = DMA_PERIPH_TO_MEMORY;                     /* DIR = 1 ,  ���赽�洢��ģʽ */
    g_dma_adc_handle.Init.PeriphInc = DMA_PINC_DISABLE;                         /* ���������ģʽ */
    g_dma_adc_handle.Init.MemInc = DMA_MINC_ENABLE;                             /* �洢������ģʽ */
    g_dma_adc_handle.Init.PeriphDataAlignment = DMA_PDATAALIGN_HALFWORD;        /* �������ݳ���:16λ */
    g_dma_adc_handle.Init.MemDataAlignment = DMA_MDATAALIGN_HALFWORD;           /* �洢�����ݳ���:16λ */
    g_dma_adc_handle.Init.Mode = DMA_NORMAL;                                    /* ��������ģʽ */
    g_dma_adc_handle.Init.Priority = DMA_PRIORITY_MEDIUM;                       /* �е����ȼ� */
    g_dma_adc_handle.Init.FIFOMode = DMA_FIFOMODE_DISABLE;                      /* ��ֹFIFO*/
    HAL_DMA_Init(&g_dma_adc_handle);                                            /* ��ʼ��DMA */

    __HAL_LINKDMA(&g_adc_dma_handle, DMA_Handle, g_dma_adc_handle);             /* ��DMA�����ADC����������� */

    /* ��ʼ��ADC */
    g_adc_dma_handle.Instance = ADC_ADCX;
    g_adc_dma_handle.Init.ClockPrescaler = ADC_CLOCK_ASYNC_DIV2;                /* ����ʱ��2��Ƶ,��adc_ker_ck=per_ck/2=32Mhz */
    g_adc_dma_handle.Init.Resolution = ADC_RESOLUTION_16B;                      /* 16λģʽ */
    g_adc_dma_handle.Init.ScanConvMode = ADC_SCAN_DISABLE;                      /* ��ɨ��ģʽ */
    g_adc_dma_handle.Init.EOCSelection = ADC_EOC_SINGLE_CONV;                   /* �ر�EOC�ж� */
    g_adc_dma_handle.Init.LowPowerAutoWait = DISABLE;                           /* �Զ��͹��Ĺر� */
    g_adc_dma_handle.Init.ContinuousConvMode = ENABLE;                          /* ��������ת�� */
    g_adc_dma_handle.Init.NbrOfDiscConversion = 1;                              /* 1��ת���ڹ��������� Ҳ����ֻת����������1 */
    g_adc_dma_handle.Init.DiscontinuousConvMode = DISABLE;                      /* ��ֹ����������ģʽ */
    g_adc_dma_handle.Init.NbrOfDiscConversion = 0;                              /* ����������ͨ����Ϊ0 */
    g_adc_dma_handle.Init.ExternalTrigConv = ADC_SOFTWARE_START;                /* ������� */
    g_adc_dma_handle.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE; /* ʹ��������� */
    g_adc_dma_handle.Init.Overrun = ADC_OVR_DATA_OVERWRITTEN;                   /* ���µ����ݵ�����ֱ�Ӹ��ǵ������� */
    g_adc_dma_handle.Init.OversamplingMode = DISABLE;                           /* �������ر� */
    g_adc_dma_handle.Init.LeftBitShift = ADC_LEFTBITSHIFT_NONE;                 /* ����ADCת�����������λ�� */
    g_adc_dma_handle.Init.ConversionDataManagement = ADC_CONVERSIONDATA_DMA_ONESHOT;     /* DMA���δ���ADC���� */
    HAL_ADC_Init(&g_adc_dma_handle);                                            /* ��ʼ�� */

    HAL_ADCEx_Calibration_Start(&g_adc_dma_handle, ADC_CALIB_OFFSET, ADC_SINGLE_ENDED);   /* ADCУ׼ */

    /* ����ADCͨ�� */
    adc_ch_conf.Channel = ADC_ADCX_CHY;                                         /* ����ʹ�õ�ADCͨ�� */
    adc_ch_conf.Rank = ADC_REGULAR_RANK_1;                                      /* ����������ĵ�1�� */
    adc_ch_conf.SamplingTime = ADC_SAMPLETIME_810CYCLES_5;                      /* ��������Ϊ810.5��ʱ������ */
    adc_ch_conf.SingleDiff = ADC_SINGLE_ENDED ;                                 /* �������� */
    adc_ch_conf.OffsetNumber = ADC_OFFSET_NONE;                                 /* ��ƫ�� */
    adc_ch_conf.Offset = 0;                                                     /* ��ƫ�Ƶ�����£��˲������� */
    adc_ch_conf.OffsetRightShift = DISABLE;                                     /* ��ֹ���� */
    adc_ch_conf.OffsetSignedSaturation = DISABLE;                               /* ��ֹ�з��ű��� */
    HAL_ADC_ConfigChannel(&g_adc_dma_handle, &adc_ch_conf);                     /* ����ADCͨ�� */

    /* ����DMA�����������ж����ȼ� */
    HAL_NVIC_SetPriority(ADC_ADCX_DMASx_IRQn, 3, 3);                            /* ����DMA�ж����ȼ�Ϊ3�������ȼ�Ϊ3 */
    HAL_NVIC_EnableIRQ(ADC_ADCX_DMASx_IRQn);                                    /* ʹ��DMA�ж� */
    
    HAL_DMA_Start_IT(&g_dma_adc_handle, par, mar, 0);                           /* ����DMA���������ж� */
    HAL_ADC_Start_DMA(&g_adc_dma_handle, &mar, 0);                              /* ��ʼDMA���ݴ��� */
}

/*************************************** �����Ƕ�ͨ��ADC�ɼ�(DMA��ȡ)ʵ�� ���� *****************************************/

DMA_HandleTypeDef g_dma_nch_adc_handle;     /* ����Ҫ����ADC��ͨ�����ݵ�DMA��� */
ADC_HandleTypeDef g_adc_nch_dma_handle;     /* ����ADC����ͨ��DMA��ȡ����� */

/**
 * @brief       ADC Nͨ��(6ͨ��) DMA��ȡ ��ʼ������
 * @note        ���ڱ������õ���6��ͨ��, �궨���Ƚ϶�����, ���,�������Ͳ����ú궨��ķ�ʽ���޸�ͨ����,
 *              ֱ���ڱ����������޸�, ��������Ĭ��ʹ��PA0~PA5��6��ͨ��.
 *
 *              ע��: ����������ʹ�� ADC_ADCX(Ĭ��=ADC1) �� ADC_ADCX_DMASx(Ĭ��=DMA1_Stream7) ������ض���
 *              ��Ҫ���޸�adc.h�����������������, ���������ԭ��Ļ����Ͻ����޸�, ������ܵ����޷�����ʹ��.
 *
 * @param       par         : �����ַ
 * @param       mar         : �洢����ַ 
 * @retval      ��
 */
void adc_nch_dma_init(uint32_t par, uint32_t mar)
{
    GPIO_InitTypeDef gpio_init_struct;
    ADC_ChannelConfTypeDef adc_ch_conf = {0};
    
    __HAL_RCC_GPIOA_CLK_ENABLE();                                             /* ����GPIOA����ʱ�� */
    ADC_ADCX_CHY_CLK_ENABLE();                                                /* ʹ��ADC1/2ʱ�� */

    if ((uint32_t)ADC_ADCX_DMASx > (uint32_t)DMA2)                            /* �õ���ǰstream������DMA2����DMA1 */
    {
        __HAL_RCC_DMA2_CLK_ENABLE();                                          /* DMA2ʱ��ʹ�� */
    }
    else
    {
        __HAL_RCC_DMA1_CLK_ENABLE();                                          /* DMA1ʱ��ʹ�� */
    }

    __HAL_RCC_ADC_CONFIG(RCC_ADCCLKSOURCE_CLKP);                              /* ADC����ʱ��ѡ�� */
    
    /* ��ʼ��ADC��ͨ����Ӧ��GPIO
     * PA0-ADC_CHANNEL_16��PA1-ADC_CHANNEL_17��PA2-ADC_CHANNEL_14
     * PA3-ADC_CHANNEL_15��PA4-ADC_CHANNEL_18��PA5-ADC_CHANNEL_19
     */
    gpio_init_struct.Pin = GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3 | GPIO_PIN_4 | GPIO_PIN_5; /* GPIOA0~5 */
    gpio_init_struct.Mode = GPIO_MODE_ANALOG;                                 /* ģ�� */
    HAL_GPIO_Init(GPIOA, &gpio_init_struct);
    
    /* ��ʼ��DMA */
    g_dma_nch_adc_handle.Instance = ADC_ADCX_DMASx;                                 /* ����DMA������ */
    g_dma_nch_adc_handle.Init.Request = ADC_ADCX_DMASx_REQ;                         /* ����ѡ��DMA_REQUEST_ADC1 */
    g_dma_nch_adc_handle.Init.Direction = DMA_PERIPH_TO_MEMORY;                     /* DIR = 1 ,  ���赽�洢��ģʽ */
    g_dma_nch_adc_handle.Init.PeriphInc = DMA_PINC_DISABLE;                         /* ���������ģʽ */
    g_dma_nch_adc_handle.Init.MemInc = DMA_MINC_ENABLE;                             /* �洢������ģʽ */
    g_dma_nch_adc_handle.Init.PeriphDataAlignment = DMA_PDATAALIGN_HALFWORD;        /* �������ݳ���:16λ */
    g_dma_nch_adc_handle.Init.MemDataAlignment = DMA_MDATAALIGN_HALFWORD;           /* �洢�����ݳ���:16λ */
    g_dma_nch_adc_handle.Init.Mode = DMA_NORMAL;                                    /* ��������ģʽ */
    g_dma_nch_adc_handle.Init.Priority = DMA_PRIORITY_MEDIUM;                       /* �е����ȼ� */
    g_dma_nch_adc_handle.Init.FIFOMode = DMA_FIFOMODE_DISABLE;                      /* ��ֹFIFO*/
    HAL_DMA_Init(&g_dma_nch_adc_handle);                                            /* ��ʼ��DMA */
    
    __HAL_LINKDMA(&g_adc_nch_dma_handle, DMA_Handle, g_dma_nch_adc_handle);         /* ��DMA��adc��ϵ���� */

    /* ��ʼ��ADC */
    g_adc_nch_dma_handle.Instance = ADC_ADCX;
    g_adc_nch_dma_handle.Init.ClockPrescaler = ADC_CLOCK_ASYNC_DIV2;                /* ����ʱ��2��Ƶ,��adc_ker_ck=per_ck/2=32Mhz */
    g_adc_nch_dma_handle.Init.Resolution = ADC_RESOLUTION_16B;                      /* 16λģʽ */
    g_adc_nch_dma_handle.Init.ScanConvMode = ADC_SCAN_ENABLE;                       /* ɨ��ģʽ */
    g_adc_nch_dma_handle.Init.EOCSelection = ADC_EOC_SINGLE_CONV;                   /* �ر�EOC�ж� */
    g_adc_nch_dma_handle.Init.LowPowerAutoWait = DISABLE;                           /* �Զ��͹��Ĺر� */
    g_adc_nch_dma_handle.Init.ContinuousConvMode = ENABLE;                          /* ��������ת�� */
    g_adc_nch_dma_handle.Init.NbrOfDiscConversion = 6;                              /* ��ֵ��Χ��1~16����ʵ���õ�6��ͨ�� */
    g_adc_nch_dma_handle.Init.DiscontinuousConvMode = DISABLE;                      /* ��ֹ����������ģʽ */
    g_adc_nch_dma_handle.Init.NbrOfDiscConversion = 0;                              /* ����������ͨ����Ϊ0 */
    g_adc_nch_dma_handle.Init.ExternalTrigConv = ADC_SOFTWARE_START;                /* ������� */
    g_adc_nch_dma_handle.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE; /* ʹ��������� */
    g_adc_nch_dma_handle.Init.Overrun = ADC_OVR_DATA_OVERWRITTEN;                   /* ���µ����ݵ�����ֱ�Ӹ��ǵ������� */
    g_adc_nch_dma_handle.Init.OversamplingMode = DISABLE;                           /* �������ر� */
    g_adc_nch_dma_handle.Init.LeftBitShift = ADC_LEFTBITSHIFT_NONE;                 /* ����ADCת�����������λ�� */
    g_adc_nch_dma_handle.Init.ConversionDataManagement = ADC_CONVERSIONDATA_DMA_ONESHOT;     /* DMA���δ���ADC���� */
    HAL_ADC_Init(&g_adc_nch_dma_handle);                                            /* ��ʼ�� */

    HAL_ADCEx_Calibration_Start(&g_adc_nch_dma_handle, ADC_CALIB_OFFSET, ADC_SINGLE_ENDED); /* ADCУ׼ */
    
    /* ����ADCͨ�� */
    adc_ch_conf.Channel = ADC_CHANNEL_14;                       /* ����ʹ�õ�ADCͨ�� */
    adc_ch_conf.Rank = ADC_REGULAR_RANK_1;                      /* ����������ĵ�1�� */
    adc_ch_conf.SamplingTime = ADC_SAMPLETIME_810CYCLES_5;      /* ��������Ϊ810.5��ʱ������ */
    adc_ch_conf.SingleDiff = ADC_SINGLE_ENDED ;                 /* �������� */
    adc_ch_conf.OffsetNumber = ADC_OFFSET_NONE;                 /* ��ƫ�� */
    adc_ch_conf.Offset = 0;                                     /* ��ƫ�Ƶ�����£��˲������� */
    adc_ch_conf.OffsetRightShift = DISABLE;                     /* ��ֹ���� */
    adc_ch_conf.OffsetSignedSaturation = DISABLE;               /* ��ֹ�з��ű��� */
    HAL_ADC_ConfigChannel(&g_adc_nch_dma_handle, &adc_ch_conf); /* ����ADCͨ�� */

    adc_ch_conf.Channel = ADC_CHANNEL_15;                       /* ����ʹ�õ�ADCͨ�� */
    adc_ch_conf.Rank = ADC_REGULAR_RANK_2;                      /* ����������ĵ�2�� */
    HAL_ADC_ConfigChannel(&g_adc_nch_dma_handle, &adc_ch_conf); /* ����ADCͨ�� */

    adc_ch_conf.Channel = ADC_CHANNEL_16;                       /* ����ʹ�õ�ADCͨ�� */
    adc_ch_conf.Rank = ADC_REGULAR_RANK_3;                      /* ����������ĵ�3�� */
    HAL_ADC_ConfigChannel(&g_adc_nch_dma_handle, &adc_ch_conf); /* ����ADCͨ�� */

    adc_ch_conf.Channel = ADC_CHANNEL_17;                       /* ����ʹ�õ�ADCͨ�� */
    adc_ch_conf.Rank = ADC_REGULAR_RANK_4;                      /* ����������ĵ�4�� */
    HAL_ADC_ConfigChannel(&g_adc_nch_dma_handle, &adc_ch_conf); /* ����ADCͨ�� */

    adc_ch_conf.Channel = ADC_CHANNEL_18;                       /* ����ʹ�õ�ADCͨ�� */
    adc_ch_conf.Rank = ADC_REGULAR_RANK_5;                      /* ����������ĵ�5�� */
    HAL_ADC_ConfigChannel(&g_adc_nch_dma_handle, &adc_ch_conf); /* ����ADCͨ�� */

    adc_ch_conf.Channel = ADC_CHANNEL_19;                       /* ����ʹ�õ�ADCͨ�� */
    adc_ch_conf.Rank = ADC_REGULAR_RANK_6;                      /* ����������ĵ�6�� */
    HAL_ADC_ConfigChannel(&g_adc_nch_dma_handle, &adc_ch_conf); /* ����ADCͨ�� */

    /* ����DMA�����������ж����ȼ� */
    HAL_NVIC_SetPriority(ADC_ADCX_DMASx_IRQn, 3, 3);
    HAL_NVIC_EnableIRQ(ADC_ADCX_DMASx_IRQn);

    HAL_DMA_Start_IT(&g_dma_nch_adc_handle, par, mar, 0);       /* ����DMA���������ж� */
    HAL_ADC_Start_DMA(&g_adc_nch_dma_handle, &mar, 0);          /* ����ADC��ͨ��DMA������ */
}

/************************* �����ǵ�ͨ��ADC�ɼ�(DMA��ȡ)ʵ��Ͷ�ͨ��ADC�ɼ�(DMA��ȡ)ʵ�� ���ô��� *******************************/

/**
 * @brief       ʹ��һ��ADC DMA���� 
 * @param       ndtr: DMA����Ĵ���
 * @retval      ��
 */
void adc_dma_enable(uint16_t ndtr)
{
    ADC_ADCX->CR &= ~(1 << 0);         /* �ȹر�ADC */

    ADC_ADCX_DMASx->CR &= ~(1 << 0);   /* �ر�DMA���� */
    while (ADC_ADCX_DMASx->CR & 0X1);  /* ȷ��DMA���Ա����� */
    ADC_ADCX_DMASx->NDTR = ndtr;       /* Ҫ�������������Ŀ */
    ADC_ADCX_DMASx->CR |= 1 << 0;      /* ����DMA���� */
 
    ADC_ADCX->CR |= 1 << 0;            /* ��������ADC */
    ADC_ADCX->CR |= 1 << 2;            /* ��������ת��ͨ�� */
}

/**
 * @brief       ADC DMA�ɼ��жϷ�����
 * @param       ��
 * @retval      ��
 */
void ADC_ADCX_DMASx_IRQHandler(void)
{
    if (ADC_ADCX_DMASx_IS_TC())        /* �ж�DMA���ݴ������ */
    {
        g_adc_dma_sta = 1;             /* ���DMA������� */
        ADC_ADCX_DMASx_CLR_TC();       /* ���DMA1 ������7 ��������ж� */
    }
}

/****************************************** �����ǵ�ͨ��ADC������(26λ�ֱ���)ʵ�� ���� ********************************************/

/**
 * @brief       ADC ������ ��ʼ������
 * @note        ���������Կ���ADC��������Χ��1x ~ 1024x, �õ����26λ�ֱ��ʵ�ADת�����
 * @param       osr : ����������, 0 ~ 1023, ��ʾ:1x ~ 1024x����������
 * @param       ovss: ����������λ��, 0~11, ��ʾ����0λ~11λ.
 * @note        ��������, ADC��ת��ʱ����Ӧ�Ļ��� osr��.
 * @retval      ��
 */
void adc_oversample_init(uint32_t osr, uint32_t ovss)
{
    GPIO_InitTypeDef gpio_init_struct;

    ADC_ADCX_CHY_GPIO_CLK_ENABLE();                                                             /* ����ADCͨ��IO����ʱ�� */
    ADC_ADCX_CHY_CLK_ENABLE();                                                                  /* ʹ��ADC1/2ʱ�� */
    __HAL_RCC_ADC_CONFIG(RCC_ADCCLKSOURCE_CLKP);                                                /* ADC����ʱ��ѡ�� */

    gpio_init_struct.Pin = ADC_ADCX_CHY_GPIO_PIN;                                               /* ADCͨ��IO���� */
    gpio_init_struct.Mode = GPIO_MODE_ANALOG;                                                   /* ģ�� */
    HAL_GPIO_Init(ADC_ADCX_CHY_GPIO_PORT, &gpio_init_struct);

    g_adc_handle.Instance = ADC_ADCX;                                                           /* ѡ���ĸ�ADC */
    g_adc_handle.Init.ClockPrescaler = ADC_CLOCK_ASYNC_DIV2;                                    /* ����ʱ��2��Ƶ,��adc_ker_ck=per_ck/2=32Mhz */
    g_adc_handle.Init.Resolution = ADC_RESOLUTION_16B;                                          /* 16λģʽ  */
    g_adc_handle.Init.ScanConvMode = ADC_SCAN_DISABLE;                                          /* ��ɨ��ģʽ�����õ�һ��ͨ�� */
    g_adc_handle.Init.EOCSelection = ADC_EOC_SINGLE_CONV;                                       /* �ر�EOC�ж� */
    g_adc_handle.Init.LowPowerAutoWait = DISABLE;                                               /* �Զ��͹��Ĺر� */
    g_adc_handle.Init.ContinuousConvMode = DISABLE;                                             /* �ر�����ת�� */
    g_adc_handle.Init.NbrOfConversion = 1;                                                      /* ��ֵ��Χ��1~16����ʵ���õ�1��ͨ�� */
    g_adc_handle.Init.DiscontinuousConvMode = DISABLE;                                          /* ��ֹ����ת���鲻��������ģʽ */
    g_adc_handle.Init.NbrOfDiscConversion = 0;                                                  /* ���ò���������ģʽ��ͨ��������ֹ����ת���鲻��������ģʽ�󣬴˲������� */
    g_adc_handle.Init.ExternalTrigConv = ADC_SOFTWARE_START;                                    /* ������� */
    g_adc_handle.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;                     /* ������������Ļ�����λ���� */
    g_adc_handle.Init.ConversionDataManagement = ADC_CONVERSIONDATA_DR;                         /* ����ͨ�������ݽ���������DR�Ĵ������� */
    g_adc_handle.Init.Overrun = ADC_OVR_DATA_OVERWRITTEN;                                       /* ���µ����ݺ�ֱ�Ӹ��ǵ������� */
    g_adc_handle.Init.LeftBitShift = ADC_LEFTBITSHIFT_NONE;                                     /* ����ADCת�����������λ�� */
    g_adc_handle.Init.OversamplingMode = ENABLE;                                                /* ���������� */
    g_adc_handle.Init.Oversampling.Ratio = osr;                                                 /* ����osr+1�������� */
    g_adc_handle.Init.Oversampling.RightBitShift = ovss;                                        /* ��������ovss  bit */
    g_adc_handle.Init.Oversampling.TriggeredMode = ADC_TRIGGEREDMODE_SINGLE_TRIGGER;            /* ���ڴ������������ͨ�������й�����ת�� */
    g_adc_handle.Init.Oversampling.OversamplingStopReset = ADC_REGOVERSAMPLING_CONTINUED_MODE;  /* ROVSE=1, ʹ�ܳ�������� */
    HAL_ADC_Init(&g_adc_handle);                                                                /* ��ʼ�� */

    HAL_ADCEx_Calibration_Start(&g_adc_handle, ADC_CALIB_OFFSET, ADC_SINGLE_ENDED);             /* ADCУ׼ */
}

