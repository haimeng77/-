/**
 ****************************************************************************************************
 * @file        main.c
 * @author      ����ԭ���Ŷ�(ALIENTEK)
 * @version     V1.0
 * @date        2022-09-06
 * @brief       ���STM32����, ���������ļ�(.s�ļ�), δʹ���κο��ļ�
 *              �ô���ʵ�ֹ��ܣ�ͨ��PB1����LED0��˸
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


/* ���߻���ַ���� */
#define PERIPH_BASE         0x40000000                                      /* �������ַ */

#define D2_APB1PERIPH_BASE  PERIPH_BASE                                     /* APB1���߻���ַ */
#define D3_AHB1PERIPH_BASE  (PERIPH_BASE + 0x18020000UL)                    /* AHB4���߻���ַ */

/* �������ַ���� */
#define RCC_BASE            (D3_AHB1PERIPH_BASE + 0x4400UL)                 /* RCC����ַ */
#define GPIOB_BASE          (D3_AHB1PERIPH_BASE + 0x0400UL)                 /* GPIOB����ַ */


/* ������ؼĴ���ӳ��(����) */
#define RCC_AHB4ENR         *(volatile unsigned int *)(RCC_BASE + 0xE0)     /* RCC_AHB4ENR�Ĵ���ӳ�� */
    
#define GPIOB_MODER         *(volatile unsigned int *)(GPIOB_BASE + 0x00)   /* GPIOB_MODER�Ĵ���ӳ�� */
#define GPIOB_OTYPER        *(volatile unsigned int *)(GPIOB_BASE + 0x04)   /* GPIOB_OTYPER�Ĵ���ӳ�� */
#define GPIOB_OSPEEDR       *(volatile unsigned int *)(GPIOB_BASE + 0x08)   /* GPIOB_OSPEEDR�Ĵ���ӳ�� */
#define GPIOB_PUPDR         *(volatile unsigned int *)(GPIOB_BASE + 0x0C)   /* GPIOB_PUPDR�Ĵ���ӳ�� */
#define GPIOB_IDR           *(volatile unsigned int *)(GPIOB_BASE + 0x10)   /* GPIOB_IDR�Ĵ���ӳ�� */
#define GPIOB_ODR           *(volatile unsigned int *)(GPIOB_BASE + 0x14)   /* GPIOB_ODR�Ĵ���ӳ�� */


/* ��ʱ���� */
static void delay_x(volatile unsigned int t)
{
    while(t--);
}

/* main���� */
int main(void)
{
    /* δִ���κ�PLLʱ������, Ĭ��ʹ��HSI(64M)����, �൱�ڹ�������Ƶ64MhzƵ���� */

    RCC_AHB4ENR |= 1 << 1;           /* GPIOB ʱ��ʹ�� */
    GPIOB_MODER &= ~(0X03UL << 2);   /* MODER1[1:0], ���� */
    GPIOB_MODER |= 0X01UL << 2;      /* MODER1[1:0]=1, PB1���ģʽ */
    GPIOB_OTYPER &= ~(0X01UL << 1);  /* OT1, ����, ������� */
    GPIOB_OSPEEDR &= ~(0X03UL << 2); /* OSPEEDR1[1:0], ���� */
    GPIOB_OSPEEDR |= 0X01UL << 2;    /* OSPEEDR1[1:0]=1, ���� */
    GPIOB_PUPDR &= ~(0X03UL << 2);   /* PUPDR1[1:0], ���� */
    GPIOB_PUPDR |= 0X01UL << 2;      /* PUPDR1[1:0]=1, ���� */

    while (1)
    {
        GPIOB_ODR |= 1 << 1;         /* PB1 = 1, LED0�� */
        delay_x(5000000);            /* ��ʱһ��ʱ�� */
        GPIOB_ODR &= ~(1UL << 1);    /* PB1 = 0, LED0�� */
        delay_x(5000000);            /* ��ʱһ��ʱ�� */
    }
}











