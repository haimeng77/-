/**
 ****************************************************************************************************
 * @file        spi.c
 * @author      ����ԭ���Ŷ�(ALIENTEK)
 * @version     V1.0
 * @date        2022-09-06
 * @brief       SPI ��������
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

#include "./BSP/SPI/spi.h"

SPI_HandleTypeDef g_spi2_handle;                          /* SPI��� */

/**
 * @brief       SPI�ڳ�ʼ��SPI
 * @note        ģ��ĳ�ʼ�����룬���ó�����ģʽ
 * @param       ��
 * @retval      ��
 */
void spi2_init(void)
{
    g_spi2_handle.Instance = SPI2_SPI;                                      /* SPI2 */
    g_spi2_handle.Init.Mode = SPI_MODE_MASTER;                              /* ����SPI����ģʽ������Ϊ��ģʽ */
    g_spi2_handle.Init.Direction = SPI_DIRECTION_2LINES;                    /* ����SPI�������˫�������ģʽ:SPI����Ϊ˫��ģʽ */
    g_spi2_handle.Init.DataSize = SPI_DATASIZE_8BIT;                        /* ����SPI�����ݴ�С:SPI���ͽ���8λ֡�ṹ */
    g_spi2_handle.Init.CLKPolarity = SPI_POLARITY_HIGH;                     /* ����ͬ��ʱ�ӵĿ���״̬Ϊ�ߵ�ƽ */
    g_spi2_handle.Init.CLKPhase = SPI_PHASE_2EDGE;                          /* ����ͬ��ʱ�ӵĵڶ��������أ��������½������ݱ����� */
    g_spi2_handle.Init.NSS = SPI_NSS_SOFT;                                  /* NSS�ź���Ӳ����NSS�ܽţ����������ʹ��SSIλ������:�ڲ�NSS�ź���SSIλ���� */
    g_spi2_handle.Init.NSSPMode=SPI_NSS_PULSE_DISABLE;                      /* NSS�ź�����ʧ�� */
    g_spi2_handle.Init.MasterKeepIOState=SPI_MASTER_KEEP_IO_STATE_ENABLE;   /* SPI��ģʽIO״̬����ʹ�� */
    g_spi2_handle.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_256;       /* ���岨����Ԥ��Ƶ��ֵ:������Ԥ��ƵֵΪ256 */
    g_spi2_handle.Init.FirstBit = SPI_FIRSTBIT_MSB;                         /* ָ�����ݴ����MSBλ����LSBλ��ʼ:���ݴ����MSBλ��ʼ */
    g_spi2_handle.Init.TIMode = SPI_TIMODE_DISABLE;                         /* �ر�TIģʽ */
    g_spi2_handle.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;         /* �ر�Ӳ��CRCУ�� */
    g_spi2_handle.Init.CRCPolynomial = 7;                                   /* CRCֵ����Ķ���ʽ */
    HAL_SPI_Init(&g_spi2_handle);                                           /* ��ʼ�� */
    
    __HAL_SPI_ENABLE(&g_spi2_handle);                                       /* ʹ��SPI2 */

    spi2_read_write_byte(0Xff);                                             /* �������� */
}

/**
 * @brief       SPI2�ײ�������ʱ��ʹ�ܣ���������
 * @note        �˺����ᱻHAL_SPI_Init()����
 * @param       hspi:SPI���
 * @retval      ��
 */
void HAL_SPI_MspInit(SPI_HandleTypeDef *hspi)
{
    GPIO_InitTypeDef gpio_init_struct;
    RCC_PeriphCLKInitTypeDef spi2_clk_init;
    
    SPI2_SCK_GPIO_CLK_ENABLE();                                     /* SCK��ʱ��ʹ�� */
    SPI2_MISO_GPIO_CLK_ENABLE();                                    /* MISO��ʱ��ʹ�� */
    SPI2_MOSI_GPIO_CLK_ENABLE();                                    /* MOSI��ʱ��ʹ�� */
    SPI2_SPI_CLK_ENABLE();                                          /* ʹ��SPI2ʱ�� */

    spi2_clk_init.PeriphClockSelection = RCC_PERIPHCLK_SPI2;        /* ����SPI2ʱ��Դ */
    spi2_clk_init.Spi123ClockSelection = RCC_SPI123CLKSOURCE_PLL;   /* SPI2ʱ��Դʹ��PLL1Q */
    HAL_RCCEx_PeriphCLKConfig(&spi2_clk_init);
    
    /* PB13,14,15 */
    gpio_init_struct.Pin = SPI2_SCK_GPIO_PIN | SPI2_MISO_GPIO_PIN | SPI2_MOSI_GPIO_PIN;
    gpio_init_struct.Mode = GPIO_MODE_AF_PP;                        /* �������� */
    gpio_init_struct.Pull = GPIO_PULLUP;                            /* ���� */
    gpio_init_struct.Speed = GPIO_SPEED_FAST;                       /* ���� */
    gpio_init_struct.Alternate = GPIO_AF5_SPI2;                     /* ����ΪSPI2 */
    HAL_GPIO_Init(SPI2_SCK_GPIO_PORT, &gpio_init_struct);
}

/**
 * @brief       SPI2�ٶ����ú���
 * @note        SPI2ʱ��ѡ������pll1_q_ck, Ϊ200Mhz
 *              SPI�ٶ� = spi_ker_ck / 2^(speed + 1)
 * @param       speed: SPI2ʱ�ӷ�Ƶϵ��
 * @retval      ��
 */
void spi2_set_speed(unsigned long speed)
{
    assert_param(IS_SPI_BAUDRATE_PRESCALER(speed));   /* �ж���Ч�� */
    __HAL_SPI_DISABLE(&g_spi2_handle);                /* �ر�SPI */
    g_spi2_handle.Instance->CR1 &= 0XFFC7;            /* λ3-5���㣬�������ò����� */
    g_spi2_handle.Instance->CR1 |= speed;             /* ����SPI�ٶ� */
    __HAL_SPI_ENABLE(&g_spi2_handle);                 /* ʹ��SPI */
}

/**
 * @brief       SPI2 ��дһ���ֽ�
 * @param       txdata:Ҫд����ֽ�
 * @retval      ����ֵ:��ȡ�����ֽ�
 */
uint8_t spi2_read_write_byte(uint8_t txdata)
{
    uint8_t rxdata;

    HAL_SPI_TransmitReceive(&g_spi2_handle, &txdata, &rxdata, 1, 1000);

    return rxdata;                                    /* �����յ������� */
}
