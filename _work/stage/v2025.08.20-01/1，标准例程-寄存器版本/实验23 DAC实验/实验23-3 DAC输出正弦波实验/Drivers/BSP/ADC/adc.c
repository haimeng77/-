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
#include "./BSP/DMA/dma.h"
#include "./SYSTEM/delay/delay.h"


uint8_t g_adc_dma_sta = 0;  /* DMA����״̬��־, 0,δ���; 1, ����� */

/**
 * @brief       ADC��ʼ������
 * @note        ������֧��ADC1/ADC2����ͨ��,���ǲ�֧��ADC3
 *              ����ʹ��16λ����, ADC����ʱ��=32M, ת��ʱ��Ϊ:�������� + 8.5��ADC����
 *              ��������������: 810.5, ��ת��ʱ�� = 819��ADC���� = 25.6us
 * @param       ��
 * @retval      ��
 */
void adc_init(void)
{
    ADC_ADCX_CHY_GPIO_CLK_ENABLE(); /* IO��ʱ��ʹ�� */
    ADC_ADCX_CHY_CLK_ENABLE();      /* ADCʱ��ʹ�� */
    
    sys_gpio_set(ADC_ADCX_CHY_GPIO_PORT, ADC_ADCX_CHY_GPIO_PIN,
                 SYS_GPIO_MODE_AIN, SYS_GPIO_OTYPE_PP, SYS_GPIO_SPEED_MID, SYS_GPIO_PUPD_PU);   /* AD�ɼ�����ģʽ����,ģ������ */

    RCC->AHB1RSTR |= 1 << 5;        /* ADC1/2��λ */
    RCC->AHB1RSTR &= ~(1 << 5);     /* ��λ���� */

    RCC->D3CCIPR &= ~(3 << 16);     /* ADCSEL[1:0]���� */
    RCC->D3CCIPR |= 2 << 16;        /* ADCSEL[1:0]=2,per_ck��ΪADCʱ��Դ,Ĭ��ѡ��hsi_ker_ck��Ϊper_ck,Ƶ��:64Mhz */
    ADC12_COMMON->CCR |= 1 << 18;   /* PRESC[3:0]=1,����ʱ��2��Ƶ,��adc_ker_ck=per_ck=64Mhz,ADC����ʱ��=����ʱ��/2=32M(���ܳ���36Mhz) */

    ADC_ADCX->CR = 0;               /* CR�Ĵ�������,DEEPPWD����,�����˯�߻���. */
    ADC_ADCX->CR |= 1 << 28;        /* ADVREGEN=1,ʹ��ADC��ѹ�� */
    
    delay_ms(10);                   /* �ȴ���ѹ���������,Լ10us,������ʱ��һ��,û��ϵ. */
    
    ADC_ADCX->CR |= 1 << 8;         /* BOOST=1,ADC������boostģʽ(ADCƵ�ʴ���20M��ʱ��,��������boostλ) */
    ADC_ADCX->CFGR &= ~(1 << 13);   /* CONT=0,����ת��ģʽ */
    ADC_ADCX->CFGR |= 1 << 12;      /* OVRMOD=1,��дģʽ(DR�Ĵ����ɱ���д) */
    ADC_ADCX->CFGR &= ~(3 << 10);   /* EXTEN[1:0]=0,������� */
    ADC_ADCX->CFGR &= ~(7 << 2);    /* RES[2:0]λ���� */
    ADC_ADCX->CFGR |= 0 << 2;       /* RES[2:0]=0,16λ�ֱ���(0,16λ;1,14λ;2,12λ;3,10λ;4,8λ.) */

    ADC_ADCX->CFGR2 &= ~((uint32_t)15 << 28);   /* LSHIFT[3:0]=0,������,�����Ҷ���. */
    ADC_ADCX->CFGR2 &= ~((uint32_t)0X3FF << 16);/* OSR[9:0]=0,��ʹ�ù����� */


    ADC_ADCX->CR &= ~((uint32_t)1 << 30);   /* ADCALDIF=0,У׼����ת��ͨ�� */
    ADC_ADCX->CR |= 1 << 16;            /* ADCALLIN=1,����У׼ */
    ADC_ADCX->CR |= (uint32_t)1 << 31;  /* ����У׼ */

    while (ADC_ADCX->CR & ((uint32_t)1 << 31)); /* �ȴ�У׼��� */

    ADC_ADCX->SQR1 &= ~(0XF << 0);  /* L[3:0]���� */
    ADC_ADCX->SQR1 |= 0 << 0;       /* L[3:0]=0,1��ת���ڹ��������� Ҳ����ֻת����������1 */
 
    ADC_ADCX->CR |= 1 << 0; /* ����ADת���� */
}

/**
 * @brief       ����ADCͨ������ʱ��
 * @param       adcx : adc�ṹ��ָ��, ADC1 / ADC2
 * @param       ch   : ͨ����, 0~19
 * @param       stime: ����ʱ��  0~7, ��Ӧ��ϵΪ:
 *   @arg       000, 1.5��ADCʱ������        001, 2.5��ADCʱ������
 *   @arg       010, 8.5��ADCʱ������        011, 16.5��ADCʱ������
 *   @arg       100, 32.5��ADCʱ������       101, 64.5��ADCʱ������
 *   @arg       110, 387.5��ADCʱ������      111, 810.5��ADCʱ������ 
 * @retval      ��
 */
void adc_channel_set(ADC_TypeDef *adcx, uint8_t ch, uint8_t stime)
{
    if (ch < 10)              /* ͨ��0~9,ʹ��SMPR1���� */
    { 
        adcx->SMPR1 &= ~(7 << (3 * ch));        /* ͨ��ch ����ʱ����� */
        adcx->SMPR1 |= 7 << (3 * ch);           /* ͨ��ch ������������,����Խ�߾���Խ�� */
    }
    else    /* ͨ��10~19,ʹ��SMPR2���� */
    { 
        adcx->SMPR2 &= ~(7 << (3 * (ch - 10))); /* ͨ��ch ����ʱ����� */
        adcx->SMPR2 |= 7 << (3 * (ch - 10));    /* ͨ��ch ������������,����Խ�߾���Խ�� */
    } 
}

/**
 * @brief       ���ADCת����Ľ�� 
 * @param       ch: ͨ����, 0~19
 * @retval      ��
 */
uint32_t adc_get_result(uint8_t ch)
{
    adc_channel_set(ADC_ADCX, ch, 7);   /* ����ADCX��Ӧͨ������ʱ��Ϊ810.5��ʱ������ */
    
    ADC_ADCX->PCSEL |= 1 << ch;         /* ADCת��ͨ��Ԥѡ�� */
    /* ����ת������ */
    ADC_ADCX->SQR1 &= ~(0X1F << 6 * 1); /* ��������1ͨ������ */
    ADC_ADCX->SQR1 |= ch << 6 * 1;      /* ���ù�������1��ת��ͨ��Ϊch */
    ADC_ADCX->CR |= 1 << 2;             /* ��������ת��ͨ�� */

    while (!(ADC_ADCX->ISR & 1 << 2));  /* �ȴ�ת������ */

    return ADC_ADCX->DR;                /* ����adcֵ */
}

/**
 * @brief       ��ȡͨ��ch��ת��ֵ��ȡtimes��,Ȼ��ƽ��
 * @param       ch      : ͨ����, 0~19
 * @param       times   : ��ȡ����
 * @retval      ͨ��ch��times��ת�����ƽ��ֵ
 */
uint32_t adc_get_result_average(uint8_t ch, uint8_t times)
{
    uint32_t temp_val = 0;
    uint8_t t;

    for (t = 0; t < times; t++) /* ��ȡtimes������ */
    {
        temp_val += adc_get_result(ch);
        delay_ms(5);
    }

    return temp_val / times;    /* ����ƽ��ֵ */
}

/**
 * @brief       ADC DMA�ɼ��жϷ�����
 * @param       �� 
 * @retval      ��
 */
void ADC_ADCX_DMASx_IRQHandler(void)
{
    if (ADC_ADCX_DMASx_IS_TC())     /* DMA�������? */
    {
        g_adc_dma_sta = 1;          /* ���DMA������� */
        ADC_ADCX_DMASx_CLR_TC();    /* ���DMA1 ������7 ��������ж� */
    }
}

/**
 * @brief       ADC DMA��ȡ ��ʼ������
 * @note        ����������ʹ��adc_init��ADC���д󲿷�����,�в���ĵط��ٵ�������
 * @param       ADC_ADCX_DMASx : DMA������,DMA1_Stream0~7/DMA2_Stream0~7
 * @param       ch         :  DMAͨ��ѡ��,��Χ:1~115(���<<STM32H7xx�ο��ֲ�>>16.3.2��,Table 116)
 * @param       par         : �����ַ
 * @param       mar         : �洢����ַ 
 * @retval      ��
 */
void adc_dma_init(uint32_t par, uint32_t mar)
{
    adc_init(); /* �ȳ�ʼ��ADC */

    adc_channel_set(ADC_ADCX, ADC_ADCX_CHY, 7); /* ����ADCX��Ӧͨ������ʱ��Ϊ810.5��ʱ������ */
    
    /* ͨ��Ԥѡ�� */
    ADC_ADCX->PCSEL |= 1 << ADC_ADCX_CHY;       /* ADCת��ͨ��Ԥѡ�� */
    
    /* ����ת������ */
    ADC_ADCX->SQR1 &= ~(0X1F << 6 * 1);         /* ��������1ͨ������ */
    ADC_ADCX->SQR1 |= ADC_ADCX_CHY << 6 * 1;    /* ���ù�������1��ת��ͨ��Ϊch */

    /* ����ADC����ת��, DMA���δ���ADC���� */
    ADC_ADCX->CFGR |= 1 << 0;   /* DMNGT[1:0] = 01, DMA���δ���ADC���� */
    ADC_ADCX->CFGR |= 1 << 13;  /* CONT = 1, ����ת��ģʽ */
   
    /* DMA������� */
    dma_mux_init(ADC_ADCX_DMASx, ADC_ADCX_DMASx_REQ);  /* ��ʼ��DMA �������� */    
    ADC_ADCX_DMASx->PAR = par;      /* DMA�����ַ */
    ADC_ADCX_DMASx->M0AR = mar;     /* DMA �洢��0��ַ */
    ADC_ADCX_DMASx->NDTR = 0;       /* Ҫ�������������Ŀ */
    ADC_ADCX_DMASx->CR = 0;         /* ��ȫ����λCR�Ĵ���ֵ */

    ADC_ADCX_DMASx->CR |= 0 << 6;   /* ���赽�洢��ģʽ */
    ADC_ADCX_DMASx->CR |= 0 << 8;   /* ��ѭ��ģʽ(��ʹ����ͨģʽ) */
    ADC_ADCX_DMASx->CR |= 0 << 9;   /* ���������ģʽ */
    ADC_ADCX_DMASx->CR |= 1 << 10;  /* �洢������ģʽ */
    ADC_ADCX_DMASx->CR |= 1 << 11;  /* �������ݳ���:16λ */
    ADC_ADCX_DMASx->CR |= 1 << 13;  /* �洢�����ݳ���:16λ */
    ADC_ADCX_DMASx->CR |= 1 << 16;  /* �е����ȼ� */
    ADC_ADCX_DMASx->CR |= 0 << 21;  /* ����ͻ�����δ��� */
    ADC_ADCX_DMASx->CR |= 0 << 23;  /* �洢��ͻ�����δ��� */
    
    ADC_ADCX_DMASx->CR |= 1 << 4;   /* TCIE = 1, DMA��������ж�ʹ�� */ 
    
    sys_nvic_init(3, 3, ADC_ADCX_DMASx_IRQn, 2);  /* ��2��������ȼ� */
}

/**
 * @brief       ʹ��һ��ADC DMA���� 
 * @param       ndtr: DMA����Ĵ���
 * @retval      ��
 */
void adc_dma_enable(uint16_t ndtr)
{
    ADC_ADCX->CR &= ~(1 << 0);              /* �ȹر�ADC */

    dma_enable(ADC_ADCX_DMASx, ndtr);       /* ����ʹ��DMA���� */

    ADC_ADCX->CR |= 1 << 0;                 /* ��������ADC */
    ADC_ADCX->CR |= 1 << 2;                 /* ��������ת��ͨ�� */
}

/**
 * @brief       ADC Nͨ��(6ͨ��) DMA��ȡ ��ʼ������
 * @note        ����������ʹ��adc_init��ADC���д󲿷�����,�в���ĵط��ٵ�������
 *              ����,���ڱ������õ���6��ͨ��, �궨���Ƚ϶�����, ���,�������Ͳ����ú궨��ķ�ʽ���޸�ͨ����,
 *              ֱ���ڱ����������޸�, ��������Ĭ��ʹ��PA0~PA5��6��ͨ��.
 *
 *              ע��: ����������ʹ�� ADC_ADCX(Ĭ��=ADC1) �� ADC_ADCX_DMASx(Ĭ��=DMA1_Stream7) ������ض���
 *              ��Ҫ���޸�adc.h�����������������, ���������ԭ��Ļ����Ͻ����޸�, ������ܵ����޷�����ʹ��.
 *
 * @param       mar         : �洢����ַ 
 * @retval      ��
 */
void adc_nch_dma_init(uint32_t mar)
{
    adc_init(); /* �ȳ�ʼ��ADC */
    
    /* ����ADC1ͨ��14~19�Ĳ���ʱ�� */
    adc_channel_set(ADC_ADCX, 14, 7); /* ����ADCX��ͨ��14(��ӦPA2��)�Ĳ���ʱ��Ϊ810.5��ʱ������ */
    adc_channel_set(ADC_ADCX, 15, 7); /* ����ADCX��ͨ��15(��ӦPA3��)�Ĳ���ʱ��Ϊ810.5��ʱ������ */
    adc_channel_set(ADC_ADCX, 16, 7); /* ����ADCX��ͨ��16(��ӦPA0��)�Ĳ���ʱ��Ϊ810.5��ʱ������ */
    adc_channel_set(ADC_ADCX, 17, 7); /* ����ADCX��ͨ��17(��ӦPA1��)�Ĳ���ʱ��Ϊ810.5��ʱ������ */
    adc_channel_set(ADC_ADCX, 18, 7); /* ����ADCX��ͨ��18(��ӦPA4��)�Ĳ���ʱ��Ϊ810.5��ʱ������ */
    adc_channel_set(ADC_ADCX, 19, 7); /* ����ADCX��ͨ��19(��ӦPA5��)�Ĳ���ʱ��Ϊ810.5��ʱ������ */

    /* ����ADC����ת��, DMA���δ���ADC���� */
    ADC_ADCX->CFGR |= 1 << 0;       /* DMNGT[1:0] = 01, DMA���δ���ADC���� */
    ADC_ADCX->CFGR |= 1 << 13;      /* CONT = 1, ����ת��ģʽ */
   
    ADC_ADCX->SQR1 = 0;             /* SQR1���� */
    ADC_ADCX->SQR1 |= 5 << 0;       /* L[3:0]=5,6��ת���ڹ��������� Ҳ����ת����������1~6 */

    /* ���ù�������1~6��Ӧ��ADCת��ͨ�� */
    ADC_ADCX->SQR1 |= 14 << 6;      /* SQ1[4:0] = 14, ��������1 = ͨ��14(PA2) */
    ADC_ADCX->SQR1 |= 15 << 12;     /* SQ2[4:0] = 15, ��������2 = ͨ��15(PA3) */
    ADC_ADCX->SQR1 |= 16 << 18;     /* SQ3[4:0] = 16, ��������3 = ͨ��16(PA0) */
    ADC_ADCX->SQR1 |= 17 << 24;     /* SQ4[4:0] = 17, ��������4 = ͨ��17(PA1) */
    ADC_ADCX->SQR2 |= 18 << 0;      /* SQ5[4:0] = 18, ��������5 = ͨ��18(PA4) */
    ADC_ADCX->SQR2 |= 19 << 6;      /* SQ6[4:0] = 19, ��������6 = ͨ��19(PA5) */

    /* ����ADC1ͨ��14~19��Ӧ��IO��ģ������ */
    /* PA2��ӦADC1_INP14 */
    sys_gpio_set(GPIOA, SYS_GPIO_PIN2,
                 SYS_GPIO_MODE_AIN, SYS_GPIO_OTYPE_PP, SYS_GPIO_SPEED_MID, SYS_GPIO_PUPD_PU);   /* AD�ɼ�����ģʽ����,ģ������ */
    
    /* PA3��ӦADC1_INP15 */
    sys_gpio_set(GPIOA, SYS_GPIO_PIN3,
                 SYS_GPIO_MODE_AIN, SYS_GPIO_OTYPE_PP, SYS_GPIO_SPEED_MID, SYS_GPIO_PUPD_PU);   /* AD�ɼ�����ģʽ����,ģ������ */
    
    /* PA0��ӦADC1_INP16 */
    sys_gpio_set(GPIOA, SYS_GPIO_PIN0,
                 SYS_GPIO_MODE_AIN, SYS_GPIO_OTYPE_PP, SYS_GPIO_SPEED_MID, SYS_GPIO_PUPD_PU);   /* AD�ɼ�����ģʽ����,ģ������ */
    
    /* PA1��ӦADC1_INP17 */
    sys_gpio_set(GPIOA, SYS_GPIO_PIN1,
                 SYS_GPIO_MODE_AIN, SYS_GPIO_OTYPE_PP, SYS_GPIO_SPEED_MID, SYS_GPIO_PUPD_PU);   /* AD�ɼ�����ģʽ����,ģ������ */
    
    /* PA4��ӦADC1_INP18 */
    sys_gpio_set(GPIOA, SYS_GPIO_PIN4,
                 SYS_GPIO_MODE_AIN, SYS_GPIO_OTYPE_PP, SYS_GPIO_SPEED_MID, SYS_GPIO_PUPD_PU);   /* AD�ɼ�����ģʽ����,ģ������ */
                 
    /* PA5��ӦADC1_INP19 */
    sys_gpio_set(GPIOA, SYS_GPIO_PIN5,
                 SYS_GPIO_MODE_AIN, SYS_GPIO_OTYPE_PP, SYS_GPIO_SPEED_MID, SYS_GPIO_PUPD_PU);   /* AD�ɼ�����ģʽ����,ģ������ */


    /* ����ͨ��Ԥѡ,ѡ��ͨ��14~19,6��ͨ�� */
    ADC_ADCX->PCSEL |= 0X3F << 14;  /* ADCת��ͨ��Ԥѡ�� */

    /* DMA������� */
    dma_mux_init(ADC_ADCX_DMASx, ADC_ADCX_DMASx_REQ);   /* ��ʼ��DMA �������� */    
    ADC_ADCX_DMASx->PAR = (uint32_t)&ADC_ADCX->DR;      /* DMA�����ַ */
    ADC_ADCX_DMASx->M0AR = mar;     /* DMA �洢��0��ַ */
    ADC_ADCX_DMASx->NDTR = 0;       /* DMA �洢��0��ַ */
    ADC_ADCX_DMASx->CR = 0;         /* ��ȫ����λCR�Ĵ���ֵ */

    ADC_ADCX_DMASx->CR |= 0 << 6;   /* ���赽�洢��ģʽ */
    ADC_ADCX_DMASx->CR |= 0 << 8;   /* ��ѭ��ģʽ(��ʹ����ͨģʽ) */
    ADC_ADCX_DMASx->CR |= 0 << 9;   /* ���������ģʽ */
    ADC_ADCX_DMASx->CR |= 1 << 10;  /* �洢������ģʽ */
    ADC_ADCX_DMASx->CR |= 1 << 11;  /* �������ݳ���:16λ */
    ADC_ADCX_DMASx->CR |= 1 << 13;  /* �洢�����ݳ���:16λ */
    ADC_ADCX_DMASx->CR |= 1 << 16;  /* �е����ȼ� */
    ADC_ADCX_DMASx->CR |= 0 << 21;  /* ����ͻ�����δ��� */
    ADC_ADCX_DMASx->CR |= 0 << 23;  /* �洢��ͻ�����δ��� */
    
    ADC_ADCX_DMASx->CR |= 1 << 4;   /* TCIE = 1, DMA��������ж�ʹ�� */

    sys_nvic_init(3, 3, ADC_ADCX_DMASx_IRQn, 2);    /* ��2��������ȼ� */
}

/**
 * @brief       ADC ������ ��ʼ������
 * @note        ����������ʹ��adc_init��ADC���д󲿷�����,�в���ĵط��ٵ�������
 *              ���������Կ���ADC��������Χ��1x ~ 1024x, �õ����26λ�ֱ��ʵ�ADת�����
 * @param       osr : ����������, 0 ~ 1023, ��ʾ:1x ~ 1024x����������
 * @param       ovss: ����������λ��, 0~11, ��ʾ����0λ~11λ. 
 * @note        ��������, ADC��ת��ʱ����Ӧ�Ļ��� osr��. 
 * @retval      ��
 */
void adc_oversample_init(uint32_t osr, uint8_t ovss)
{
    adc_init(); /* �ȳ�ʼ��ADC */
    
    ADC_ADCX->CR &= ~(1 << 0);  /* �ر�ADת���� */

    ADC_ADCX->CFGR2 = 0;        /* CFGR2���� */
    ADC_ADCX->CFGR2 |= (osr & 0X3FF) << 16; /* OSR[9:0]=osr, ���ù����� */
    ADC_ADCX->CFGR2 |= 0 << 10; /* ROVSM=0, ����ģʽ */
    ADC_ADCX->CFGR2 |= 0 << 9;  /* TROVS=0, ������������ɹ����� */
    ADC_ADCX->CFGR2 |= (uint32_t)ovss << 5; /* OVSS[3:0]=ovss, ���ù���������λ�� */
    ADC_ADCX->CFGR2 |= 1 << 0;  /* ROVSE=1, ʹ�ܳ�������� */
   
    ADC_ADCX->CR |= 1 << 0;     /* ����ADת���� */
}






























































