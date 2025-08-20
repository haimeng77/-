/**
 ****************************************************************************************************
 * @file        main.c
 * @author      ����ԭ���Ŷ�(ALIENTEK)
 * @version     V1.0
 * @date        2022-09-06
 * @brief       SD�� ʵ��
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
 ****************************************************************************************************
 */

#include "./SYSTEM/sys/sys.h"
#include "./SYSTEM/usart/usart.h"
#include "./SYSTEM/delay/delay.h"
#include "./BSP/SDRAM/sdram.h"
#include "./USMART/usmart.h"
#include "./BSP/LED/led.h"
#include "./BSP/LCD/lcd.h"
#include "./BSP/KEY/key.h"
#include "./BSP/MPU/mpu.h"
#include "./BSP/LCD/ltdc.h"
#include "./MALLOC/malloc.h"
#include "./BSP/SDMMC/sdmmc_sdcard.h"


/**
 * @brief       ͨ�����ڴ�ӡSD�������Ϣ
 * @param       ��
 * @retval      ��
 */
void show_sdcard_info(void)
{
    uint64_t card_capacity;       /* SD������ */
    HAL_SD_CardCIDTypeDef sd_card_cid;

    HAL_SD_GetCardCID(&g_sd_handle, &sd_card_cid);       /* ��ȡCID */
    get_sd_card_info(&g_sd_card_info_handle);            /* ��ȡSD����Ϣ */

    switch (g_sd_card_info_handle.CardType)
    {
        case CARD_SDSC:
        {
            if (g_sd_card_info_handle.CardVersion == CARD_V1_X)
            {
                printf("Card Type:SDSC V1\r\n");
            }
            else if (g_sd_card_info_handle.CardVersion == CARD_V2_X)
            {
                printf("Card Type:SDSC V2\r\n");
            }
        }
            break;
        
        case CARD_SDHC_SDXC:
            printf("Card Type:SDHC\r\n");
            break;
    }

    card_capacity = (uint64_t)(g_sd_card_info_handle.LogBlockNbr) * (uint64_t)(g_sd_card_info_handle.LogBlockSize); /* ����SD������ */
    printf("Card ManufacturerID:%d\r\n", sd_card_cid.ManufacturerID);                /* ������ID */
    printf("Card RCA:%d\r\n", g_sd_card_info_handle.RelCardAdd);                     /* ����Ե�ַ */
    printf("LogBlockNbr:%d \r\n", (uint32_t)(g_sd_card_info_handle.LogBlockNbr));    /* ��ʾ�߼������� */
    printf("LogBlockSize:%d \r\n", (uint32_t)(g_sd_card_info_handle.LogBlockSize));  /* ��ʾ�߼����С */
    printf("Card Capacity:%d MB\r\n", (uint32_t)(card_capacity >> 20));              /* ��ʾ���� */
    printf("Card BlockSize:%d\r\n\r\n", g_sd_card_info_handle.BlockSize);            /* ��ʾ���С */
}

/**
 * @brief       ����SD���Ķ�ȡ
 * @note        ��secaddr��ַ��ʼ,��ȡseccnt������������
 * @param       secaddr : ������ַ
 * @param       seccnt  : ������
 * @retval      ��
 */
void sd_test_read(uint32_t secaddr, uint32_t seccnt)
{
    uint32_t i;
    uint8_t *buf;
    uint8_t sta = 0;
    buf = mymalloc(SRAMIN, seccnt * 512);       /* �����ڴ�,��SDRAM�����ڴ� */
    sta = sd_read_disk(buf, secaddr, seccnt);   /* ��ȡsecaddr������ʼ������ */

    if (sta == 0)
    {
        printf("SECTOR %d DATA:\r\n", secaddr);

        for (i = 0; i < seccnt * 512; i++)
        {
            printf("%x ", buf[i]);  /* ��ӡsecaddr��ʼ���������� */
        }
        
        printf("\r\nDATA ENDED\r\n");
    }
    else 
    {
        printf("err:%d\r\n", sta);
    }

    myfree(SRAMIN, buf);            /* �ͷ��ڴ� */
}

/**
 * @brief       ����SD����д��
 * @note        ��secaddr��ַ��ʼ,д��seccnt������������
 *              ����!! ���дȫ��0XFF������,���������SD��.
 *
 * @param       secaddr : ������ַ
 * @param       seccnt  : ������
 * @retval      ��
 */
void sd_test_write(uint32_t secaddr, uint32_t seccnt)
{
    uint32_t i;
    uint8_t *buf;
    uint8_t sta = 0;
    buf = mymalloc(SRAMIN, seccnt * 512);   /* ��SDRAM�����ڴ� */

    for (i = 0; i < seccnt * 512; i++)      /* ��ʼ��д�������,��3�ı���. */
    {
        buf[i] = i * 3;
    }

    sta = sd_write_disk(buf, secaddr, seccnt); /* ��secaddr������ʼд��seccnt���������� */

    if (sta == 0)
    {
        printf("Write over!\r\n");
    }
    else 
    {
        printf("err:%d\r\n", sta);
    }

    myfree(SRAMIN, buf);    /* �ͷ��ڴ� */
}

int main(void)
{
    uint8_t key;
    uint32_t sd_size;
    uint8_t t = 0;
    uint8_t *buf;
    uint64_t card_capacity;                 /* SD������ */

    sys_cache_enable();                     /* ��L1-Cache */
    HAL_Init();                             /* ��ʼ��HAL�� */
    sys_stm32_clock_init(160, 5, 2, 4);     /* ����ʱ��, 400Mhz */
    delay_init(400);                        /* ��ʱ��ʼ�� */
    usart_init(115200);                     /* ���ڳ�ʼ�� */
    usmart_init(200);                       /* ��ʼ��USMART */
    mpu_memory_protection();                /* ������ش洢���� */
    led_init();                             /* ��ʼ��LED */
    sdram_init();                           /* ��ʼ��SDRAM */
    lcd_init();                             /* ��ʼ��LCD */
    key_init();                             /* ��ʼ��KEY */

    my_mem_init(SRAMIN);                    /* ��ʼ���ڲ��ڴ��(AXI) */
    my_mem_init(SRAMEX);                    /* ��ʼ���ⲿ�ڴ��(SDRAM) */
    my_mem_init(SRAM12);                    /* ��ʼ��SRAM12�ڴ��(SRAM1+SRAM2) */
    my_mem_init(SRAM4);                     /* ��ʼ��SRAM4�ڴ��(SRAM4) */
    my_mem_init(SRAMDTCM);                  /* ��ʼ��DTCM�ڴ��(DTCM) */
    my_mem_init(SRAMITCM);                  /* ��ʼ��ITCM�ڴ��(ITCM) */
    
    lcd_show_string(30, 50, 200, 16, 16, "STM32", RED);
    lcd_show_string(30, 70, 200, 16, 16, "SD  TEST", RED);
    lcd_show_string(30, 90, 200, 16, 16, "ATOM@ALIENTEK", RED);
    lcd_show_string(30, 110, 200, 16, 16, "KEY0:Read Sector 0", RED);

    while (sd_init())    /* ��ⲻ��SD�� */
    {
        lcd_show_string(30, 150, 200, 16, 16, "SD Card Error!", RED);
        delay_ms(500);
        lcd_show_string(30, 150, 200, 16, 16, "Please Check! ", RED);
        delay_ms(500);
        LED0_TOGGLE();   /* �����˸ */
    }

    /* ��ӡSD�������Ϣ */
    show_sdcard_info(); 

    /* ���SD���ɹ� */
    lcd_show_string(30, 150, 200, 16, 16, "SD Card OK    ", BLUE);
    lcd_show_string(30, 170, 200, 16, 16, "SD Card Size:     MB", BLUE);
    card_capacity = (uint64_t)(g_sd_card_info_handle.LogBlockNbr) * (uint64_t)(g_sd_card_info_handle.LogBlockSize); /* ����SD������ */
    lcd_show_num(30 + 13 * 8, 170, card_capacity >> 20, 5, 16, BLUE); /* ��ʾSD������ */

    while (1)
    {
        key = key_scan(0);

        if (key == KEY0_PRES)                       /* KEY0������ */
        {
            buf = mymalloc(0, 512);                 /* �����ڴ� */
            key = sd_read_disk(buf, 0, 1);

            if (key == 0)                           /* ��ȡ0���������� */
            {
                lcd_show_string(30, 190, 200, 16, 16, "USART1 Sending Data...", BLUE);
                printf("SECTOR 0 DATA:\r\n");

                for (sd_size = 0; sd_size < 512; sd_size++)
                {
                    printf("%x ", buf[sd_size]);    /* ��ӡ0�������� */
                }
                
                printf("\r\nDATA ENDED\r\n");
                lcd_show_string(30, 190, 200, 16, 16, "USART1 Send Data Over!", BLUE);
            }
            else 
            {
                printf("err:%d\r\n", key);
            }

            myfree(0, buf);                        /* �ͷ��ڴ� */
        }

        t++;
        delay_ms(10);

        if (t == 20)
        {
            LED0_TOGGLE();  /* �����˸ */
            t = 0;
        }
    }
}


