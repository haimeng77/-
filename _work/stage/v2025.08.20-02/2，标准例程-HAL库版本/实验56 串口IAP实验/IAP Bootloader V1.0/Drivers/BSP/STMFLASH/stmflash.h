/**
 ****************************************************************************************************
 * @file        stmflash.h
 * @author      ����ԭ���Ŷ�(ALIENTEK)
 * @version     V1.0
 * @date        2022-09-06
 * @brief       STM32�ڲ�FLASH��д ��������
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

#ifndef __STMFLASH_H
#define __STMFLASH_H

#include "./SYSTEM/sys/sys.h"


/* FLASH��ʼ��ַ */
#define STM32_FLASH_SIZE        0X200000                    /* STM32 FLASH �ܴ�С */
#define STM32_FLASH_BASE        0x08000000                  /* STM32 FLASH ��ʼ��ַ */

#define FLASH_WAITETIME         50000                       /* FLASH�ȴ���ʱʱ�� */

/* FLASH ��������ʼ��ַ,��2��bank,ÿ��bank 1MB */
/* BANK1 */
#define BANK1_FLASH_SECTOR_0    ((uint32_t)0x08000000)      /* Bank1����0��ʼ��ַ, 128 Kbytes */
#define BANK1_FLASH_SECTOR_1    ((uint32_t)0x08020000)      /* Bank1����1��ʼ��ַ, 128 Kbytes */
#define BANK1_FLASH_SECTOR_2    ((uint32_t)0x08040000)      /* Bank1����2��ʼ��ַ, 128 Kbytes */
#define BANK1_FLASH_SECTOR_3    ((uint32_t)0x08060000)      /* Bank1����3��ʼ��ַ, 128 Kbytes */
#define BANK1_FLASH_SECTOR_4    ((uint32_t)0x08080000)      /* Bank1����4��ʼ��ַ, 128 Kbytes */
#define BANK1_FLASH_SECTOR_5    ((uint32_t)0x080A0000)      /* Bank1����5��ʼ��ַ, 128 Kbytes */
#define BANK1_FLASH_SECTOR_6    ((uint32_t)0x080C0000)      /* Bank1����6��ʼ��ַ, 128 Kbytes */
#define BANK1_FLASH_SECTOR_7    ((uint32_t)0x080E0000)      /* Bank1����7��ʼ��ַ, 128 Kbytes */

/* BNAK2 */
#define BANK2_FLASH_SECTOR_0    ((uint32_t)0x08100000)      /* Bank2����0��ʼ��ַ, 128 Kbytes */
#define BANK2_FLASH_SECTOR_1    ((uint32_t)0x08120000)      /* Bank2����1��ʼ��ַ, 128 Kbytes */
#define BANK2_FLASH_SECTOR_2    ((uint32_t)0x08140000)      /* Bank2����2��ʼ��ַ, 128 Kbytes */
#define BANK2_FLASH_SECTOR_3    ((uint32_t)0x08160000)      /* Bank2����3��ʼ��ַ, 128 Kbytes */
#define BANK2_FLASH_SECTOR_4    ((uint32_t)0x08180000)      /* Bank2����4��ʼ��ַ, 128 Kbytes */
#define BANK2_FLASH_SECTOR_5    ((uint32_t)0x081A0000)      /* Bank2����5��ʼ��ַ, 128 Kbytes */
#define BANK2_FLASH_SECTOR_6    ((uint32_t)0x081C0000)      /* Bank2����6��ʼ��ַ, 128 Kbytes */
#define BANK2_FLASH_SECTOR_7    ((uint32_t)0x081E0000)      /* Bank2����7��ʼ��ַ, 128 Kbytes */

/******************************************************************************************/

uint32_t stmflash_read_word(uint32_t faddr);                             /* ������ */
void stmflash_write(uint32_t waddr, uint32_t *pbuf, uint32_t length);    /* ��ָ����ַ��ʼд��ָ�����ȵ����� */
void stmflash_read(uint32_t raddr, uint32_t *pbuf, uint32_t length);     /* ��ָ����ַ��ʼ����ָ�����ȵ����� */

void test_write(uint32_t waddr, uint32_t wdata);   /* ����д�� */

#endif

