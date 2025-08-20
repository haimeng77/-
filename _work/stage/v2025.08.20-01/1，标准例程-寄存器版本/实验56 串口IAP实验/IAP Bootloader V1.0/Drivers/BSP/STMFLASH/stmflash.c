/**
 ****************************************************************************************************
 * @file        stmflash.c
 * @author      ����ԭ���Ŷ�(ALIENTEK)
 * @version     V1.0
 * @date        2023-03-24
 * @brief       STM32�ڲ�FLASH��д ��������
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
 * V1.0 20230324
 * ��һ�η���
 *
 ****************************************************************************************************
 */

#include "./SYSTEM/usart/usart.h"
#include "./SYSTEM/delay/delay.h"
#include "./BSP/STMFLASH/stmflash.h"


/**
 * @brief       ����STM32��FLASH
 * @param       ��
 * @retval      ��
 */
static void stmflash_unlock(void)
{
    FLASH->KEYR1 = STM32_FLASH_KEY1;    /* Bank1 д���������. */
    FLASH->KEYR1 = STM32_FLASH_KEY2;
    
    FLASH->KEYR2 = STM32_FLASH_KEY1;    /* Bank2 д���������. */
    FLASH->KEYR2 = STM32_FLASH_KEY2;
}

/**
 * @brief       flash����
 * @param       ��
 * @retval      ��
 */
static void stmflash_lock(void)
{
    FLASH->CR1 |= 1 << 0;           /* Bank1,���� */
    FLASH->CR2 |= 1 << 0;           /* Bank2,���� */
}

/**
 * @brief       �õ�FLASH�Ĵ���״̬
 * @param       bankx   : 0,��ȡbank1��״̬
 *                        1,��ȡbank2��״̬
 * @retval      ִ�н��
 *   @arg       0   : �����
 *   @arg       ���� : ������
 */
static uint8_t stmflash_get_error_status(uint8_t bankx)
{
    uint32_t res = 0;
    
    if (bankx == 0)
    {
        res = FLASH->SR1;
    }else
    {
        res = FLASH->SR2;
    }
    
    if (res & (1 << 17)) return 1;  /* WRPERR=1,д�������� */
    if (res & (1 << 18)) return 2;  /* PGSERR=1,������д��� */
    if (res & (1 << 19)) return 3;  /* STRBERR=1,��д���� */
    if (res & (1 << 21)) return 4;  /* INCERR=1,����һ���Դ��� */
    if (res & (1 << 22)) return 5;  /* OPERR=1,д/�������� */
    if (res & (1 << 23)) return 6;  /* RDPERR=1,���������� */
    if (res & (1 << 24)) return 7;  /* RDSERR=1,�Ƿ����ʼ��������� */
    if (res & (1 << 25)) return 8;  /* SNECCERR=1,1bit eccУ������ */
    if (res & (1 << 26)) return 9;  /* DBECCERR=1,2bit ecc���� */

    return 0;   /* û���κ�״̬/�������. */
}

/**
 * @brief       �ȴ��������
 * @param       bankx   : 0,bank1; 1,bank2
 * @param       time    : Ҫ��ʱ�ĳ���
 * @retval      ִ�н��
 *   @arg       0   : �����
 *   @arg       0XFF: ��ʱ
 *   @arg       ���� : ������
 */
static uint8_t stmflash_wait_done(uint8_t bankx, uint32_t time)
{
    uint8_t res = 0;
    uint32_t tempreg = 0;

    while (1)
    {
        if (bankx == 0) tempreg = FLASH->SR1;
        else tempreg = FLASH->SR2;
        
        if ((tempreg & 0X07) == 0)
        {
            break;  /* BSY=0,WBNE=0,QW=0,�������� */
        }
        
        time--;

        if (time == 0)return 0XFF;
    }

    res = stmflash_get_error_status(bankx);

    if (res)
    {
        if (bankx == 0)  FLASH->CCR1 = 0X07EE0000;  /* ��bank1���д����־ */
        else FLASH->CCR2 = 0X07EE0000;              /* ��bank2���д����־ */
    }

    return res;
}

/**
 * @brief       ��������
 * @param       saddr   : ������ַ,��Χ��:0~15.
 *                        0~7 ,addr���ڵ�bank1�������
 *                        8~15,addr���ڵ�bank2�������+8,��Ҫ��ȥ8,�ŵ�bank2�������
 * @retval      ִ�н��
 *   @arg       0   : �����
 *   @arg       0XFF: ��ʱ
 *   @arg       ���� : ������
 */
static uint8_t stmflash_erase_sector(uint32_t saddr)
{
    uint8_t res = 0;
    res = stmflash_wait_done(saddr / 8, 0XFFFFFFFF);    /* �ȴ��ϴβ������� */

    if (res == 0)
    {
        if (saddr < 8)  /* BANK1 ���� */
        {
            FLASH->CR1 &= ~(7 << 8);            /* SNB1[2:0]=0,���ԭ�������� */
            FLASH->CR1 &= ~(3 << 4);            /* PSIZE1[1:0]=0,���ԭ�������� */
            FLASH->CR1 |= (uint32_t)saddr << 8; /* ����Ҫ�������������,0~7 */
            FLASH->CR1 |= 2 << 4;               /* ����Ϊ32bit�� */
            FLASH->CR1 |= 1 << 2;               /* SER1=1,�������� */
            FLASH->CR1 |= 1 << 7;               /* START1=1,��ʼ���� */
        }
        else            /* BANK2 ���� */
        {
            FLASH->CR2 &= ~(7 << 8);            /* SNB2[2:0]=0,���ԭ�������� */
            FLASH->CR2 &= ~(3 << 4);            /* PSIZE2[1:0]=0,���ԭ�������� */
            FLASH->CR2 |= (uint32_t)(saddr - 8) << 8;   /* ����Ҫ�������������,0~7 */
            FLASH->CR2 |= 2 << 4;               /* ����Ϊ32bit�� */
            FLASH->CR2 |= 1 << 2;               /* SER2=1,�������� */
            FLASH->CR2 |= 1 << 7;               /* START2=1,��ʼ���� */
        }

        res = stmflash_wait_done(saddr / 8, 0XFFFFFFFF);    /* �ȴ��������� */

        if (saddr < 8)FLASH->CR1 &= ~(1 << 2);  /* SER1=0,�������������־ */
        else FLASH->CR2 &= ~(1 << 2);           /* SER2=0,�������������־ */
    }

    return res;
}

/**
 * @brief       ��FLASHָ����ַд8���� (256λ����)
 *   @note      ������256bitΪ��λ���
 * @param       faddr   : д���ַ (�˵�ַ����Ϊ4�ı���!!)
 * @param       pdata   : Ҫд��������׵�ַ����8���֣�
 * @retval      ִ�н��
 *   @arg       0   : �����
 *   @arg       0XFF: ��ʱ
 *   @arg       ���� : ������
 */
static uint8_t stmflash_write_8word(uint32_t faddr, uint32_t *pdata)
{
    uint8_t nword = 8;  /* ÿ��д8����,256bit */
    uint8_t res;
    uint8_t bankx = 0;

    if (faddr < BANK2_FLASH_SECTOR_0)bankx = 0; /* �жϵ�ַ����bank0,������bank1 */
    else bankx = 1;

    res = stmflash_wait_done(bankx, 0XFFFF);

    if (res == 0)       /* OK */
    {
        if (bankx == 0) /* BANK1 ��� */
        {
            FLASH->CR1 &= ~(3 << 4);    /* PSIZE1[1:0]=0,���ԭ�������� */
            FLASH->CR1 |= 2 << 4;       /* ����Ϊ32bit�� */
            FLASH->CR1 |= 1 << 1;       /* PG1=1,���ʹ�� */
        }
        else            /* BANK2 ��� */
        {
            FLASH->CR2 &= ~(3 << 4);    /* PSIZE2[1:0]=0,���ԭ�������� */
            FLASH->CR2 |= 2 << 4;       /* ����Ϊ32bit�� */
            FLASH->CR2 |= 1 << 1;       /* PG2=1,���ʹ�� */
        }

        while (nword)
        {
            *(volatile uint32_t *)faddr = *pdata;   /* д������ */
            faddr += 4;                 /* д��ַ+4 */
            pdata++;                    /* ƫ�Ƶ���һ�������׵�ַ */
            nword--;
        }

        __DSB();        /* д������ɺ�,��������ͬ��,ʹCPU����ִ��ָ������ */
        res = stmflash_wait_done(bankx, 0XFFFF);/* �ȴ��������,һ���ֱ��,���100us. */

        if (bankx == 0)FLASH->CR1 &= ~(1 << 1); /* PG1=0,�������������־ */
        else FLASH->CR2 &= ~(1 << 1);           /* PG2=0,�������������־ */
    }

    return res;
}

/**
 * @brief       ��ָ����ַ��ȡһ���� (32λ����)
 * @param       faddr   : ��ȡ��ַ (�˵�ַ����Ϊ4�ı���!!)
 * @retval      ��ȡ�������� (32λ)
 */
uint32_t stmflash_read_word(uint32_t faddr)
{
    return *(volatile uint32_t *)faddr;
}

/**
 * @brief       ��ȡĳ����ַ���ڵ�flash����
 * @param       faddr   : flash��ַ
 * @retval      0~7 ,addr���ڵ�bank1�������
 *              8~15,addr���ڵ�bank2�������+8,��Ҫ��ȥ8,�ŵ�bank2�������
 */
static uint8_t stmflash_get_flash_sector(uint32_t addr)
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
 * @brief       ��FLASH ָ��λ��, д��ָ�����ȵ�����(�Զ�����)
 * @note        ��ΪSTM32H7������ʵ��̫��,û�취���ر�����������,���Ա�����д��ַ�����0XFF
 *              ,��ô���Ȳ������������Ҳ�������������.����д��0XFF�ĵ�ַ,�����������������ݶ�ʧ.
 *              ����д֮ǰȷ��������û����Ҫ����,��������������Ȳ�����,Ȼ����������д.
 * @param       waddr   : ��ʼ��ַ (�˵�ַ����Ϊ32�ı���!!,����д�����!)
 * @param       pbuf    : ����ָ��
 * @param       length  : Ҫд��� ��(32λ)��(����Ҫд���32λ���ݵĸ���,һ������д��32�ֽ�,��8����)
 * @retval      ��
 */
void stmflash_write(uint32_t waddr, uint32_t *pbuf, uint32_t length)
{

    uint8_t status = 0;
    uint32_t addrx = 0;
    uint32_t endaddr = 0;

    if (waddr < STM32_FLASH_BASE || waddr % 32 ||       /* д���ַС�� STM32_FLASH_BASE, ����32��������, �Ƿ�. */
        waddr > (STM32_FLASH_BASE + STM32_FLASH_SIZE))  /* д���ַ���� STM32_FLASH_BASE + STM32_FLASH_SIZE, �Ƿ�. */
    {
        return;
    }
    
    stmflash_unlock();              /* FLASH���� */
    
    addrx = waddr;                  /* д�����ʼ��ַ */
    endaddr = waddr + length * 4;   /* д��Ľ�����ַ */

    if (addrx < 0X1FF00000)         /* ֻ�����洢��,����Ҫִ�в�������!! */
    {
        while (addrx < endaddr)     /* ɨ��һ���ϰ�.(�Է�FFFFFFFF�ĵط�,�Ȳ���) */
        {
            if (stmflash_read_word(addrx) != 0XFFFFFFFF)    /* �з�0XFFFFFFFF�ĵط�,Ҫ����������� */
            {
                status = stmflash_erase_sector(stmflash_get_flash_sector(addrx));

                if (status)break;   /* ���������� */
                
                SCB_CleanInvalidateDCache();    /* �����Ч��D-Cache */
            }
            else
            {
                addrx += 4;
            }
        }
    }

    if (status == 0)
    {
        while (waddr < endaddr)     /* д���� */
        {
            if (stmflash_write_8word(waddr, pbuf))  /* д������ */
            {
                break;              /* д���쳣 */
            }

            waddr += 32;
            pbuf += 8;
        }
    }

    stmflash_lock();                /* ���� */
}

/**
 * @brief       ��ָ����ַ��ʼ����ָ�����ȵ�����
 * @param       raddr : ��ʼ��ַ
 * @param       pbuf  : ����ָ��
 * @param       length: Ҫ��ȡ�� ��(32λ)��(����Ҫ��ȡ��32λ���ݵĸ���)
 * @retval      ��
 */
void stmflash_read(uint32_t raddr, uint32_t *pbuf, uint32_t length)
{
    uint32_t i;

    for (i = 0; i < length; i++)
    {
        pbuf[i] = stmflash_read_word(raddr);    /* ��ȡ4���ֽ�(1����) */
        raddr += 4;                             /* ƫ��4���ֽ� */
    }
}

/******************************************************************************************/
/* �����ô��� */

/**
 * @brief       ����д����(д1����)
 * @param       waddr : ��ʼ��ַ
 * @param       wdata : Ҫд�������
 * @retval      ��
 */
void test_write(uint32_t waddr, uint32_t wdata)
{
    stmflash_write(waddr, &wdata, 1);       /* д��һ���� */
}


















