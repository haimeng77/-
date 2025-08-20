/**
 ****************************************************************************************************
 * @file        usbd_storage.c
 * @author      ����ԭ���Ŷ�(ALIENTEK)
 * @version     V1.0
 * @date        2022-09-06
 * @brief       usbd_storage ��������
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

#include "usbd_storage.h"
#include "./BSP/NAND/ftl.h"
#include "./BSP/NAND/nand.h"
#include "./BSP/SDMMC/sdmmc_sdcard.h"
#include "./BSP/NORFLASH/norflash.h"

/* �ļ�ϵͳ���ⲿFLASH����ʼ��ַ
 * ���Ƕ���SPI FLASHǰ25M���ļ�ϵͳ��, ���Ե�ַ��0��ʼ
 */
#define USB_STORAGE_FLASH_BASE  0

/* �Լ������һ�����USB״̬�ļĴ��� */
/* bit0:��ʾ����������SD��д������ */
/* bit1:��ʾ��������SD���������� */
/* bit2:SD��д���ݴ����־λ */
/* bit3:SD�������ݴ����־λ */
/* bit4:1,��ʾ��������ѯ����(�������ӻ�������) */
volatile uint8_t g_usb_state_reg = 0;

/* USB Mass storage ��׼��ѯ����(ÿ��lunռ36�ֽ�) */
const int8_t STORAGE_Inquirydata_FS[] = {

    /* LUN 0 */
    0x00,
    0x80,
    0x02,
    0x02,
    (STANDARD_INQUIRY_DATA_LEN - 4),
    0x00,
    0x00,
    0x00,
    /* Vendor Identification */
    'A', 'L', 'I', 'E', 'N', 'T', 'E', 'K', ' ',/* 9�ֽ� */
    /* Product Identification */
    'S', 'P', 'I', ' ', 'F', 'l', 'a', 's', 'h',/* 15�ֽ� */
    ' ', 'D', 'i', 's', 'k', ' ',
    /* Product Revision Level */
    '1', '.', '0', ' ',                         /* 4�ֽ� */

    /* LUN 1 */
    0x00,
    0x80,
    0x02,
    0x02,
    (STANDARD_INQUIRY_DATA_LEN - 4),
    0x00,
    0x00,
    0x00,
    /* Vendor Identification */
    'A', 'L', 'I', 'E', 'N', 'T', 'E', 'K', ' ',     /* 9�ֽ� */
    /* Product Identification */
    'N', 'A', 'N', 'D', ' ', 'F', 'l', 'a', 's', 'h',/* 15�ֽ� */
    ' ', 'D', 'i', 's', 'k',
    /* Product Revision Level */
    '1', '.', '0', ' ',                              /* 4�ֽ� */

    /* LUN 2 */
    0x00,
    0x80,
    0x02,
    0x02,
    (STANDARD_INQUIRY_DATA_LEN - 4),
    0x00,
    0x00,
    0x00,
    /* Vendor Identification */
    'A', 'L', 'I', 'E', 'N', 'T', 'E', 'K', ' ',    /* 9�ֽ� */
    /* Product Identification */
    'S', 'D', ' ', 'F', 'l', 'a', 's', 'h', ' ',    /* 15�ֽ� */
    'D', 'i', 's', 'k', ' ', ' ',
    /* Product Revision Level */
    '1', '.', '0', ' ',                             /* 4�ֽ� */
};

static int8_t STORAGE_Init_FS(uint8_t lun);
static int8_t STORAGE_GetCapacity_FS(uint8_t lun, uint32_t *block_num, uint16_t *block_size);
static int8_t STORAGE_IsReady_FS(uint8_t lun);
static int8_t STORAGE_IsWriteProtected_FS(uint8_t lun);
static int8_t STORAGE_Read_FS(uint8_t lun, uint8_t *buf, uint32_t blk_addr, uint16_t blk_len);
static int8_t STORAGE_Write_FS(uint8_t lun, uint8_t *buf, uint32_t blk_addr, uint16_t blk_len);
static int8_t STORAGE_GetMaxLun_FS(void);

USBD_StorageTypeDef USBD_Storage_Interface_fops_FS =
{
  STORAGE_Init_FS,
  STORAGE_GetCapacity_FS,
  STORAGE_IsReady_FS,
  STORAGE_IsWriteProtected_FS,
  STORAGE_Read_FS,
  STORAGE_Write_FS,
  STORAGE_GetMaxLun_FS,
  (int8_t *)STORAGE_Inquirydata_FS
};



/**
 * @brief       ��ʼ���洢�豸
 * @param       lun        : �߼���Ԫ���
 *   @arg                  0, SPI FLASH
 *   @arg                  1, NAND LASH
 *   @arg                  2, SD��
 * @retval      �������
 *   @arg       0    , �ɹ�
 *   @arg       ���� , �������
 */
int8_t STORAGE_Init_FS(uint8_t lun)
{
    uint8_t res = 0;

    switch (lun)
    {
        case 0:                 /* SPI FLASH */
            norflash_init();
            break;

        case 1:                 /* NAND FLASH */
            res = ftl_init();
            break;

        case 2:                 /* SD�� */
            res = sd_init();
            break;
    }

    return res;
}



/**
 * @brief       ��ȡ�洢�豸�������Ϳ��С
 * @param       lun        : �߼���Ԫ���
 *   @arg                  0, SPI FLASH
 *   @arg                  1, NAND FLASH
 *   @arg                  2, SD��
 * @param       block_num  : ������(������)
 * @param       block_size : ���С(������С)
 * @retval      �������
 *   @arg       0    , �ɹ�
 *   @arg       ���� , �������
 */
int8_t STORAGE_GetCapacity_FS(uint8_t lun, uint32_t *block_num, uint16_t *block_size)
{
    switch (lun)
    {
        case 0:       /* SPI FLASH */
            *block_size = 512;
            *block_num = 1024 * 1024 * 25 / 512;    /* SPI FLASH��ǰ��25M�ֽ�,�ļ�ϵͳ�� */
            break;

        case 1:       /* NAND FLASH */
            *block_size = 512;
            *block_num = nand_dev.valid_blocknum * nand_dev.block_pagenum * nand_dev.page_mainsize / 512;
            break;

        case 2:       /* SD�� */
            HAL_SD_GetCardInfo(&g_sd_handle, &g_sd_card_info_handle);
            *block_num = g_sd_card_info_handle.LogBlockNbr - 1;
            *block_size = g_sd_card_info_handle.LogBlockSize;
            break;
    }

    return 0;
}

/**
 * @brief       �鿴�洢�豸�Ƿ����
 * @param       lun        : �߼���Ԫ���
 *   @arg                  0, SPI FLASH
 *   @arg                  1, NAND FLASH
 *   @arg                  2, SD��
 * @retval      ����״̬
 *   @arg       0    , ����
 *   @arg       ���� , δ����
 */
int8_t STORAGE_IsReady_FS(uint8_t lun)
{
    g_usb_state_reg |= 0X10;    /* �����ѯ */
    return 0;
}


/**
 * @brief       �鿴�洢�豸�Ƿ�д����
 * @param       lun        : �߼���Ԫ���
 *   @arg                  0, SPI FLASH
 *   @arg                  1, NAND FLASH
 *   @arg                  2, SD��
 * @retval      ������״̬
 *   @arg       0    , û�ж�����
 *   @arg       ���� , �ж�����
 */
int8_t STORAGE_IsWriteProtected_FS(uint8_t lun)
{
    return 0;
}


/**
 * @brief       �Ӵ洢�豸��ȡ����
 * @param       lun        : �߼���Ԫ���
 *   @arg                  0, SPI FLASH
 *   @arg                  1, NAND FLASH
 *   @arg                  2, SD��
 * @param       buf        : ���ݴ洢���׵�ַָ��
 * @param       blk_addr   : Ҫ��ȡ�ĵ�ַ(������ַ)
 * @param       blk_len    : Ҫ��ȡ�Ŀ���(������)
 * @retval      �������
 *   @arg       0    , �ɹ�
 *   @arg       ���� , �������
 */
int8_t STORAGE_Read_FS(uint8_t lun, uint8_t *buf, uint32_t blk_addr, uint16_t blk_len)
{
    int8_t res = 0;
    g_usb_state_reg |= 0X02;    /* ������ڶ����� */
    switch (lun)
    {
        case 0:           /* SPI FLASH */
            norflash_read(buf, USB_STORAGE_FLASH_BASE + blk_addr * 512, blk_len * 512);
            break;

        case 1:           /* NAND FLASH */
            res = ftl_read_sectors(buf, blk_addr, 512, blk_len);
            break;

        case 2:           /* SD�� */
            res = sd_read_disk(buf, blk_addr, blk_len);
            break;
    }

    if (res)
    {
        printf("rerr:%d,%d", lun, res);
        g_usb_state_reg |= 0X08;    /* ������! */
    }
    return res;
}


/**
 * @brief       ��洢�豸д����
 * @param       lun        : �߼���Ԫ���
 *   @arg                  0, SPI FLASH
 *   @arg                  1, NAND FLASH
 *   @arg                  2, SD��
 * @param       buf        : ���ݴ洢���׵�ַָ��
 * @param       blk_addr   : Ҫд��ĵ�ַ(������ַ)
 * @param       blk_len    : Ҫд��Ŀ���(������)
 * @retval      �������
 *   @arg       0    , �ɹ�
 *   @arg       ���� , �������
 */
int8_t STORAGE_Write_FS(uint8_t lun, uint8_t *buf, uint32_t blk_addr, uint16_t blk_len)
{
    int8_t res = 0;
    g_usb_state_reg |= 0X01;    /* �������д���� */

    switch (lun)
    {
        case 0:                /* SPI FLASH */
            norflash_write(buf, USB_STORAGE_FLASH_BASE + blk_addr * 512, blk_len * 512);
            break;

        case 1:                /* NAND FLASH */
            res = ftl_write_sectors(buf, blk_addr, 512, blk_len);
            break;

        case 2:                /* SD�� */
            res = sd_write_disk(buf, blk_addr, blk_len);
            break;
    }

    if (res)
    {
        g_usb_state_reg |= 0X04;    /* д����! */
        printf("werr:%d,%d", lun, res);
    }

    return res;
}


/**
 * @brief       ��ȡ֧�ֵ�����߼���Ԫ����
 *   @note      ע��, ���ﷵ�ص��߼���Ԫ�����Ǽ�ȥ��1��.
 *              0, �ͱ�ʾ1��; 1, ��ʾ2��; �Դ�����
 * @param       ��
 * @retval      ֧�ֵ��߼���Ԫ���� - 1
 */
int8_t STORAGE_GetMaxLun_FS(void)
{
    HAL_SD_GetCardInfo(&g_sd_handle,&g_sd_card_info_handle);

    /* STORAGE_LUN_NBR ��usbd_conf.h���涨��, Ĭ����2 */
    if (g_sd_card_info_handle.LogBlockNbr)    /* ���SD������, ��֧��2������ */
    {
        return STORAGE_LUN_NBR - 1;
    }
    else    /* SD��������, ��ֻ֧��1������ */
    {
        return STORAGE_LUN_NBR - 2;
    }
}


