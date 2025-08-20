/**
 ****************************************************************************************************
 * @file        stmflash.c
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

#include "./SYSTEM/usart/usart.h"
#include "./SYSTEM/delay/delay.h"
#include "./BSP/STMFLASH/stmflash.h"


/**
 * @brief       ��ָ����ַ��ȡһ���� (32λ����)
 * @param       faddr   : ��ȡ��ַ (�˵�ַ����Ϊ4����!!)
 * @retval      ��ȡ�������� (32λ)
 */
uint32_t stmflash_read_word(uint32_t faddr)
{
    return *(volatile uint32_t *)faddr;
}

/**
 * @brief       ��ָ����ַ��ʼ����ָ�����ȵ�����
 * @param       raddr : ��ʼ��ַ
 * @param       pbuf  : ����ָ��
 * @param       length: Ҫ��ȡ����(32λ)��,��4���ֽڵ�������
 * @retval      ��
 */
void stmflash_read(uint32_t raddr, uint32_t *pbuf, uint32_t length)
{
    uint32_t  i;
    for (i = 0; i < length; i++)
    {
        pbuf[i] = stmflash_read_word(raddr);/* ��ȡ4���ֽ� */
        raddr += 4;                         /* ƫ��4���ֽ� */
    }
}

/**
 * @brief       ��ȡĳ����ַ���ڵ�flash����
 * @param       faddr   : flash��ַ
 * @retval      0~7 ,addr���ڵ�bank1�������
 *              8~15,addr���ڵ�bank2�������+8,��Ҫ��ȥ8,�ŵ�bank2�������
 */
uint8_t stmflash_get_flash_sector(uint32_t addr)
{
    if (addr < BANK1_FLASH_SECTOR_1)return 0;
    else if (addr < BANK1_FLASH_SECTOR_2)return 1;
    else if (addr < BANK1_FLASH_SECTOR_3)return 2;
    else if (addr < BANK1_FLASH_SECTOR_4)return 3;
    else if (addr < BANK1_FLASH_SECTOR_5)return 4;
    else if (addr < BANK1_FLASH_SECTOR_6)return 5;
    else if (addr < BANK1_FLASH_SECTOR_7)return 6;
    else if (addr < BANK2_FLASH_SECTOR_0)return 7;
    else if (addr < BANK2_FLASH_SECTOR_1)return 8;
    else if (addr < BANK2_FLASH_SECTOR_2)return 9;
    else if (addr < BANK2_FLASH_SECTOR_3)return 10;
    else if (addr < BANK2_FLASH_SECTOR_4)return 11;
    else if (addr < BANK2_FLASH_SECTOR_5)return 12;
    else if (addr < BANK2_FLASH_SECTOR_6)return 13;
    else if (addr < BANK2_FLASH_SECTOR_7)return 14;

    return 15;
}

/**
 * @brief       ��ָ����ַ��ʼд��ָ�����ȵ�����
 * @note        �ر�ע�⣺��ΪSTM32H7������ʵ��̫��,û�취���ر�����������,���Ա�����
 *              д��ַ�����0XFF,��ô���Ȳ������������Ҳ�������������.����
 *              д��0XFF�ĵ�ַ,�����������������ݶ�ʧ.����д֮ǰȷ��������
 *              û����Ҫ����,��������������Ȳ�����,Ȼ����������д
 *              �ú�����OTP����Ҳ��Ч!��������дOTP��!
 *              OTP�����ַ��Χ:0X1FFF7800~0X1FFF7A0F(ע�⣺���16�ֽڣ�����OTP���ݿ�����������д����)
 * @param       waddr  : ��ʼ��ַ(�˵�ַ����Ϊ4�ı���!!)
 * @param       pbuf   : ����ָ��
 * @param       length : ��(32λ)��(����Ҫд���32λ���ݵĸ���)
 * @retval      ��
 */
void stmflash_write(uint32_t waddr, uint32_t *pbuf, uint32_t length)
{
    FLASH_EraseInitTypeDef FlashEraseInit;
    HAL_StatusTypeDef FlashStatus = HAL_OK;
    uint32_t SectorError = 0;
    uint32_t  addrx = 0;
    uint32_t  endaddr = 0;

    if (waddr < STM32_FLASH_BASE || waddr % 32 ||       /* д���ַС�� STM32_FLASH_BASE, ����32��������, �Ƿ�. */
        waddr > (STM32_FLASH_BASE + STM32_FLASH_SIZE))  /* д���ַ���� STM32_FLASH_BASE + STM32_FLASH_SIZE, �Ƿ�. */
    {
        return;
    }

    HAL_FLASH_Unlock();                        /* ���� */
    addrx = waddr;                             /* д�����ʼ��ַ */
    endaddr = waddr + length * 4;              /* д��Ľ�����ַ */

    if (addrx < 0X1FF00000)
    {
        while (addrx < endaddr)               /* ɨ��һ���ϰ�.(�Է�FFFFFFFF�ĵط�,�Ȳ���) */
        {
            if (stmflash_read_word(addrx) != 0XFFFFFFFF)            /* �з�0XFFFFFFFF�ĵط�,Ҫ����������� */
            {
                FlashEraseInit.Banks = FLASH_BANK_1;
                FlashEraseInit.TypeErase = FLASH_TYPEERASE_SECTORS;             /* �������ͣ��������� */
                FlashEraseInit.Sector = stmflash_get_flash_sector(addrx);       /* Ҫ���������� */
                FlashEraseInit.NbSectors = 1;                                   /* һ��ֻ����һ������ */
                FlashEraseInit.VoltageRange = FLASH_VOLTAGE_RANGE_3;            /* ��ѹ��Χ��VCC=2.7~3.6V֮��!! */
                if (HAL_FLASHEx_Erase(&FlashEraseInit, &SectorError) != HAL_OK)
                {
                    break;           /* ���������� */
                }

                SCB_CleanInvalidateDCache();              /* �����Ч��D-Cache */
            }
            else
            {
               addrx += 4;
            }
            FLASH_WaitForLastOperation(FLASH_WAITETIME, FLASH_BANK_1);      /* �ȴ��ϴβ������ */
        }
    }
    
    FlashStatus = FLASH_WaitForLastOperation(FLASH_WAITETIME, FLASH_BANK_1);   /* �ȴ��ϴβ������ */
    if (FlashStatus == HAL_OK)
    {
        while (waddr < endaddr)       /* д���� */
        {
            if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_FLASHWORD, waddr, (uint64_t)pbuf) != HAL_OK)    /* д������ */
            {
                break;        /* д���쳣 */
            }
            waddr += 32;
            pbuf += 8;
        }
    }
    HAL_FLASH_Lock();          /* ���� */
}

/******************************************************************************************/
/* �����ô��� */

/**
 * @brief       ����д����(д1����)
 * @param       waddr : ��ʼ��ַ
 * @param       wdata : Ҫд�������
 * @retval      ��ȡ��������
 */
void test_write(uint32_t waddr, uint32_t wdata)
{
    stmflash_write(waddr, &wdata, 1);/* д��һ���� */
}


