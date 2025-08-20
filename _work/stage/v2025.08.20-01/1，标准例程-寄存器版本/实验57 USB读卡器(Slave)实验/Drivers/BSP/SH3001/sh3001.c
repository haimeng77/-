/**
 ****************************************************************************************************
 * @file        sh3001.c
 * @author      正点原子团队(ALIENTEK)
 * @version     V1.0
 * @date        2023-03-23
 * @brief       SH3001 驱动代码
 * @license     Copyright (c) 2022-2032, 广州市星翼电子科技有限公司
 ****************************************************************************************************
 * @attention
 *
 * 实验平台:正点原子 STM32H743开发板
 * 在线视频:www.yuanzige.com
 * 技术论坛:www.openedv.com
 * 公司网址:www.alientek.com
 * 购买地址:openedv.taobao.com
 *
 * 修改说明
 * V1.0 20230323
 * 第一次发布
 *
 ****************************************************************************************************
 */

#include "./BSP/IIC/myiic.h"
#include "./BSP/SH3001/sh3001.h"
#include "./SYSTEM/usart/usart.h"
#include "./SYSTEM/delay/delay.h"


static compCoefType g_compcoef;         /* compCoefType结构体,用于存储补偿系数 */
static unsigned char g_store_acc_odr;   /* 保存ACC ODR临时变量 */


/**
 * @brief       从SH3001读取N字节数据
 *   @note      SH3001的命令发送,也是用该函数实现(不带参数的命令, 也会有一个状态寄存器需要读取)
 * @param       devaddr  : 寄存器地址
 * @param       regaddr  : 寄存器命令
 * @param       length   : 读取长度
 * @param       readbuf  : 数据存储buf
 * @retval      0, 操作成功
 *              其他, 操作失败
 */
unsigned char sh3001_read_nbytes(unsigned char devaddr, unsigned char regaddr, unsigned char length, unsigned char *readbuf)
{
    unsigned char i;

    iic_start();
    iic_send_byte(devaddr << 1 | 0X00);

    if (iic_wait_ack())
    {
        iic_stop();
        return 1;
    }

    iic_send_byte(regaddr);             /* 写地址/命令 */
    iic_wait_ack();

    iic_start();
    iic_send_byte((devaddr << 1) + 1);
    iic_wait_ack();

    for (i = 0; i < length; i++)        /* 循环读取 数据 */
    {
        readbuf[i] = iic_read_byte(1);
    }

    iic_read_byte(0);
    iic_stop();
    return (SH3001_TRUE);
}

/**
 * @brief       SH3001写入N字节数据
 * @param       devaddr  : 寄存器地址
 * @param       regaddr  : 寄存器命令
 * @param       length   : 写入长度
 * @param       writebuf : 数据存储buf
 * @retval      0, 操作成功
 *              其他, 操作失败
 */
unsigned char sh3001_write_nbytes(unsigned char devaddr, unsigned char regaddr, unsigned char length, unsigned char *writebuf)
{
    unsigned char i;

    iic_start();
    iic_send_byte(devaddr << 1 | 0X00);
    iic_wait_ack();

    iic_send_byte(regaddr);
    iic_wait_ack();

    for (i = 0; i < length; i++)
    {
        iic_send_byte(writebuf[i]);

        if (iic_wait_ack())
        {
            iic_stop();
            return (SH3001_FALSE);
        }
    }

    iic_stop();
    return (SH3001_TRUE);
}

/**
 * @brief       设置加速度计参数
 * @note        accCutOffRreq = accODR * 0.40 or accODR * 0.25 or accODR * 0.11 or accODR * 0.04;
 * @param       accODR              accRange                accCutOffFreq       accFilterEnble
*               SH3001_ODR_1000HZ   SH3001_ACC_RANGE_16G    SH3001_ACC_ODRX040  SH3001_ACC_FILTER_EN
*               SH3001_ODR_500HZ    SH3001_ACC_RANGE_8G     SH3001_ACC_ODRX025  SH3001_ACC_FILTER_DIS
*               SH3001_ODR_250HZ    SH3001_ACC_RANGE_4G     SH3001_ACC_ODRX011
*               SH3001_ODR_125HZ    SH3001_ACC_RANGE_2G     SH3001_ACC_ODRX004
*               SH3001_ODR_63HZ
*               SH3001_ODR_31HZ
*               SH3001_ODR_16HZ
*               SH3001_ODR_2000HZ
*               SH3001_ODR_4000HZ
*               SH3001_ODR_8000HZ
 * @retval      无
 */
static void sh3001_acc_config(uint8_t accODR, uint8_t accRange, uint8_t accCutOffFreq, uint8_t accFilterEnble)
{
    unsigned char regData = 0;

    /* 使能加速度计数字滤波器 */
    sh3001_read_nbytes(SH3001_ADDRESS, SH3001_ACC_CONF0, 1, &regData);
    regData |= 0x01;
    sh3001_write_nbytes(SH3001_ADDRESS, SH3001_ACC_CONF0, 1, &regData);

    /* 设置采样频率 */
    g_store_acc_odr = accODR;
    sh3001_write_nbytes(SH3001_ADDRESS, SH3001_ACC_CONF1, 1, &accODR);

    /* 设置加速度计满量程范围 */
    sh3001_write_nbytes(SH3001_ADDRESS, SH3001_ACC_CONF2, 1, &accRange);

    /* 设置低通滤波器 */
    sh3001_read_nbytes(SH3001_ADDRESS, SH3001_ACC_CONF3, 1, &regData);
    regData &= 0x17;
    regData |= (accCutOffFreq | accFilterEnble);
    sh3001_write_nbytes(SH3001_ADDRESS, SH3001_ACC_CONF3, 1, &regData);
}

/**
 * @brief       设置陀螺仪参数
 * @note        gyroCutOffFreq = gyroODR * 0.40 or gyroODR * 0.25 or gyroODR * 0.11 or gyroODR * 0.04;
 * @param       gyroODR             gyroRangeX,Y,Z          gyroCutOffFreq      gyroFilterEnble
*               SH3001_ODR_1000HZ   SH3001_GYRO_RANGE_125   SH3001_GYRO_ODRX00  SH3001_GYRO_FILTER_EN
*               SH3001_ODR_500HZ    SH3001_GYRO_RANGE_250   SH3001_GYRO_ODRX01  SH3001_GYRO_FILTER_DIS
*               SH3001_ODR_250HZ    SH3001_GYRO_RANGE_500   SH3001_GYRO_ODRX02
*               SH3001_ODR_125HZ    SH3001_GYRO_RANGE_1000  SH3001_GYRO_ODRX03
*               SH3001_ODR_63HZ     SH3001_GYRO_RANGE_2000
*               SH3001_ODR_31HZ
*               SH3001_ODR_16HZ
*               SH3001_ODR_2000HZ
*               SH3001_ODR_4000HZ
*               SH3001_ODR_8000HZ
*               SH3001_ODR_16000HZ
*               SH3001_ODR_32000HZ
 * @retval      无
 */
static void sh3001_gyro_config(uint8_t gyroODR, uint8_t gyroRangeX, uint8_t gyroRangeY, uint8_t gyroRangeZ, uint8_t gyroCutOffFreq, uint8_t gyroFilterEnble)
{
    unsigned char regData = 0;

    /* 使能陀螺仪数字滤波器 */
    sh3001_read_nbytes(SH3001_ADDRESS, SH3001_GYRO_CONF0, 1, &regData);
    regData |= 0x01;
    sh3001_write_nbytes(SH3001_ADDRESS, SH3001_GYRO_CONF0, 1, &regData);

    /* 设置采样频率 */
    sh3001_write_nbytes(SH3001_ADDRESS, SH3001_GYRO_CONF1, 1, &gyroODR);

    /* 设置满量程范围 */
    sh3001_write_nbytes(SH3001_ADDRESS, SH3001_GYRO_CONF3, 1, &gyroRangeX);
    sh3001_write_nbytes(SH3001_ADDRESS, SH3001_GYRO_CONF4, 1, &gyroRangeY);
    sh3001_write_nbytes(SH3001_ADDRESS, SH3001_GYRO_CONF5, 1, &gyroRangeZ);

    /* 设置低通滤波器 */
    sh3001_read_nbytes(SH3001_ADDRESS, SH3001_GYRO_CONF2, 1, &regData);
    regData &= 0xE3;
    regData |= (gyroCutOffFreq | gyroFilterEnble);
    sh3001_write_nbytes(SH3001_ADDRESS, SH3001_GYRO_CONF2, 1, &regData);
}

/**
 * @brief       设置温度参数
 * @param       tempODR
 *                  SH3001_TEMP_ODR_500
 *                  SH3001_TEMP_ODR_250
 *                  SH3001_TEMP_ODR_125
 *                  SH3001_TEMP_ODR_63
 * @param       tempEnable
 *                  SH3001_TEMP_EN
 *                  SH3001_TEMP_DIS
 * @retval      无
 */
static void sh3001_temp_config(unsigned char tempODR, unsigned char tempEnable)
{
    unsigned char regData = 0;

    /* 设置温度传感器参数 */
    sh3001_read_nbytes(SH3001_ADDRESS, SH3001_TEMP_CONF0, 1, &regData);
    regData &= 0x4F;
    regData |= (tempODR | tempEnable);
    sh3001_write_nbytes(SH3001_ADDRESS, SH3001_TEMP_CONF0, 1, &regData);
}

/**
 * @brief       读取补偿系数
 * @param       g_compcoef    : compCoefType结构体指针
 * @retval      无
 */
static void sh3001_comp_init(compCoefType *g_compcoef)
{
    unsigned char coefData[2] = {0};

    /* Acc Cross */
    sh3001_read_nbytes(SH3001_ADDRESS, 0x81, 2, coefData);
    g_compcoef->cYX = (signed char)coefData[0];
    g_compcoef->cZX = (signed char)coefData[1];
    sh3001_read_nbytes(SH3001_ADDRESS, 0x91, 2, coefData);
    g_compcoef->cXY = (signed char)coefData[0];
    g_compcoef->cZY = (signed char)coefData[1];
    sh3001_read_nbytes(SH3001_ADDRESS, 0xA1, 2, coefData);
    g_compcoef->cXZ = (signed char)coefData[0];
    g_compcoef->cYZ = (signed char)coefData[1];

    /* Gyro Zero */
    sh3001_read_nbytes(SH3001_ADDRESS, 0x60, 1, coefData);
    g_compcoef->jX = (signed char)coefData[0];
    sh3001_read_nbytes(SH3001_ADDRESS, 0x68, 1, coefData);
    g_compcoef->jY = (signed char)coefData[0];
    sh3001_read_nbytes(SH3001_ADDRESS, 0x70, 1, coefData);
    g_compcoef->jZ = (signed char)coefData[0];

    sh3001_read_nbytes(SH3001_ADDRESS, SH3001_GYRO_CONF3, 1, coefData);
    coefData[0] = coefData[0] & 0x07;
    g_compcoef->xMulti = ((coefData[0] < 2) || (coefData[0] >= 7)) ? 1 : (1 << (6 - coefData[0]));
    sh3001_read_nbytes(SH3001_ADDRESS, SH3001_GYRO_CONF4, 1, coefData);
    coefData[0] = coefData[0] & 0x07;
    g_compcoef->yMulti = ((coefData[0] < 2) || (coefData[0] >= 7)) ? 1 : (1 << (6 - coefData[0]));
    sh3001_read_nbytes(SH3001_ADDRESS, SH3001_GYRO_CONF5, 1, coefData);
    coefData[0] = coefData[0] & 0x07;
    g_compcoef->zMulti = ((coefData[0] < 2) || (coefData[0] >= 7)) ? 1 : (1 << (6 - coefData[0]));

    sh3001_read_nbytes(SH3001_ADDRESS, 0x2E, 1, coefData);
    g_compcoef->paramP0 = coefData[0] & 0x1F;
}

/**
 * @brief       复位部分内部模块
 * @param       无
 * @retval      无
 */
static void sh3001_module_reset(void)
{
    const unsigned char regAddr[8] = {0xC0, 0xD3, 0xD3, 0xD5, 0xD4, 0xBB, 0xB9, 0xBA};

    /* MCC版本配置 */
    const unsigned char mcc_regDataA[8] = {0x38, 0xC6, 0xC1, 0x02, 0x0C, 0x18, 0x18, 0x18};
    const unsigned char mcc_regDataB[8] = {0x3D, 0xC2, 0xC2, 0x00, 0x04, 0x00, 0x00, 0x00};

    /* MCD版本配置 */
    const unsigned char mcd_regDataA[8] = {0x38, 0xD6, 0xD1, 0x02, 0x08, 0x18, 0x18, 0x18};
    const unsigned char mcd_regDataB[8] = {0x3D, 0xD2, 0xD2, 0x00, 0x00, 0x00, 0x00, 0x00};

    /* MCF版本配置 */
    const unsigned char mcf_regDataA[8] = {0x38, 0x16, 0x11, 0x02, 0x08, 0x18, 0x18, 0x18};
    const unsigned char mcf_regDataB[8] = {0x3E, 0x12, 0x12, 0x00, 0x00, 0x00, 0x00, 0x00};

    unsigned char regData = 0;
    unsigned char *regDataA = (unsigned char *)mcd_regDataA;    /* 默认用MCD配置 */
    unsigned char *regDataB = (unsigned char *)mcd_regDataB;

    sh3001_read_nbytes(SH3001_ADDRESS, SH3001_CHIP_VERSION, 1, &regData);   /* 读取 SH3001_CHIP_VERSION */

    if (regData == SH3001_CHIP_VERSION_MCC)         /* MCC版本 */
    {
        regDataA = (unsigned char *)mcc_regDataA;
        regDataB = (unsigned char *)mcc_regDataB;

    }
    else if (regData == SH3001_CHIP_VERSION_MCF)     /* MCF版本 */
    {
        regDataA = (unsigned char *)mcf_regDataA;
        regDataB = (unsigned char *)mcf_regDataB;
    }

    /* Drive Start */
    sh3001_write_nbytes(SH3001_ADDRESS, regAddr[0], 1, &regDataA[0]);
    sh3001_write_nbytes(SH3001_ADDRESS, regAddr[1], 1, &regDataA[1]);
    delay_ms(100);
    sh3001_write_nbytes(SH3001_ADDRESS, regAddr[0], 1, &regDataB[0]);
    sh3001_write_nbytes(SH3001_ADDRESS, regAddr[1], 1, &regDataB[1]);
    delay_ms(50);

    /* ADC Resett */
    sh3001_write_nbytes(SH3001_ADDRESS, regAddr[2], 1, &regDataA[2]);
    sh3001_write_nbytes(SH3001_ADDRESS, regAddr[3], 1, &regDataA[3]);
    delay_ms(1);
    sh3001_write_nbytes(SH3001_ADDRESS, regAddr[2], 1, &regDataB[2]);
    delay_ms(1);
    sh3001_write_nbytes(SH3001_ADDRESS, regAddr[3], 1, &regDataB[3]);
    delay_ms(50);

    /* CVA Resett */
    sh3001_write_nbytes(SH3001_ADDRESS, regAddr[4], 1, &regDataA[4]);
    delay_ms(10);
    sh3001_write_nbytes(SH3001_ADDRESS, regAddr[4], 1, &regDataB[4]);

    delay_ms(1);

    sh3001_write_nbytes(SH3001_ADDRESS, regAddr[5], 1, &regDataA[5]);
    delay_ms(10);
    sh3001_write_nbytes(SH3001_ADDRESS, regAddr[6], 1, &regDataA[6]);
    delay_ms(10);
    sh3001_write_nbytes(SH3001_ADDRESS, regAddr[7], 1, &regDataA[7]);
    delay_ms(10);
    sh3001_write_nbytes(SH3001_ADDRESS, regAddr[5], 1, &regDataB[5]);
    delay_ms(10);
    sh3001_write_nbytes(SH3001_ADDRESS, regAddr[6], 1, &regDataB[6]);
    delay_ms(10);
    sh3001_write_nbytes(SH3001_ADDRESS, regAddr[7], 1, &regDataB[7]);
    delay_ms(10);

    /* 设置 INT and INT1 脚开漏输出 */
    regData = 0x00;
    sh3001_write_nbytes(SH3001_ADDRESS, 0x44, 1, &regData);
}

/**
 * @brief       切换SH3001电源模式
 *              Normal mode: 1.65mA; Sleep mode: 162uA; Acc normal mode:393uA;
 * @param       powerMode
 *                  SH3001_NORMAL_MODE
 *                  SH3001_SLEEP_MODE
 *                  SH3001_POWERDOWN_MODE
 *                  SH3001_ACC_NORMAL_MODE
 * @retval      SH3001_TRUE 成功
 *              SH3001_FALSE 异常
 */
unsigned char sh3001_switch_powermode(unsigned char powerMode)
{
    unsigned  char regAddr[10] = {0xCF, 0x22, 0x2F, 0xCB, 0xCE, 0xD3, 0xD4, 0xD5, 0xD6, 0xD7};
    unsigned char regData[10] = {0};
    unsigned char i = 0;
    unsigned char accODR = SH3001_ODR_1000HZ;

    if ((powerMode != SH3001_NORMAL_MODE)
            && (powerMode != SH3001_SLEEP_MODE)
            && (powerMode != SH3001_POWERDOWN_MODE)
            && (powerMode != SH3001_ACC_NORMAL_MODE))
    {
        return (SH3001_FALSE);
    }


    for (i = 0; i < 10; i++)
    {
        sh3001_read_nbytes(SH3001_ADDRESS, regAddr[i], 1, &regData[i]);
    }

    switch (powerMode)
    {
        case SH3001_NORMAL_MODE:
            /* restore accODR */
            sh3001_write_nbytes(SH3001_ADDRESS, SH3001_ACC_CONF1, 1, &g_store_acc_odr);

            regData[0] = (regData[0] & 0xF8);
            regData[1] = (regData[1] & 0x7F);
            regData[2] = (regData[2] & 0xF7);
            regData[3] = (regData[3] & 0xF7);
            regData[4] = (regData[4] & 0xFE);
            regData[5] = (regData[5] & 0xFC) | 0x02;
            regData[6] = (regData[6] & 0x9F);
            regData[7] = (regData[7] & 0xF9);

            for (i = 0; i < 8; i++)
            {
                sh3001_write_nbytes(SH3001_ADDRESS, regAddr[i], 1, &regData[i]);
            }

            regData[7] = (regData[7] & 0x87);
            regData[8] = (regData[8] & 0x1F);
            regData[9] = (regData[9] & 0x03);

            for (i = 7; i < 10; i++)
            {
                sh3001_write_nbytes(SH3001_ADDRESS, regAddr[i], 1, &regData[i]);
            }

            sh3001_module_reset();
            return (SH3001_TRUE);

        case SH3001_SLEEP_MODE:
            /* store current acc ODR */
            sh3001_read_nbytes(SH3001_ADDRESS, SH3001_ACC_CONF1, 1, &g_store_acc_odr);
            /* set acc ODR=1000Hz */
            sh3001_write_nbytes(SH3001_ADDRESS, SH3001_ACC_CONF1, 1, &accODR);

            regData[0] = (regData[0] & 0xF8) | 0x07;
            regData[1] = (regData[1] & 0x7F) | 0x80;
            regData[2] = (regData[2] & 0xF7) | 0x08;
            regData[3] = (regData[3] & 0xF7) | 0x08;
            regData[4] = (regData[4] & 0xFE);
            regData[5] = (regData[5] & 0xFC) | 0x01;
            regData[6] = (regData[6] & 0x9F);
            regData[7] = (regData[7] & 0xF9) | 0x06;

            for (i = 0; i < 8; i++)
            {
                sh3001_write_nbytes(SH3001_ADDRESS, regAddr[i], 1, &regData[i]);
            }

            regData[7] = (regData[7] & 0x87);
            regData[8] = (regData[8] & 0x1F);
            regData[9] = (regData[9] & 0x03);

            for (i = 7; i < 10; i++)
            {
                sh3001_write_nbytes(SH3001_ADDRESS, regAddr[i], 1, &regData[i]);
            }

            return (SH3001_TRUE);

        case SH3001_POWERDOWN_MODE:
            regData[0] = (regData[0] & 0xF8);
            regData[1] = (regData[1] & 0x7F) | 0x80;
            regData[2] = (regData[2] & 0xF7) | 0x08;
            regData[3] = (regData[3] & 0xF7) | 0x08;
            regData[4] = (regData[4] & 0xFE);
            regData[5] = (regData[5] & 0xFC) | 0x01;
            regData[6] = (regData[6] & 0x9F) | 0x60;
            regData[7] = (regData[7] & 0xF9) | 0x06;

            for (i = 0; i < 8; i++)
            {
                sh3001_write_nbytes(SH3001_ADDRESS, regAddr[i], 1, &regData[i]);
            }

            regData[7] = (regData[7] & 0x87);
            regData[8] = (regData[8] & 0x1F);
            regData[9] = (regData[9] & 0x03);

            for (i = 7; i < 10; i++)
            {
                sh3001_write_nbytes(SH3001_ADDRESS, regAddr[i], 1, &regData[i]);
            }

            return (SH3001_TRUE);

        case SH3001_ACC_NORMAL_MODE:
            regData[0] = (regData[0] & 0xF8);
            regData[1] = (regData[1] & 0x7F);
            regData[2] = (regData[2] & 0xF7);
            regData[3] = (regData[3] & 0xF7);
            regData[4] = (regData[4] | 0x01);
            regData[5] = (regData[5] & 0xFC) | 0x01;
            regData[6] = (regData[6] & 0x9F);
            regData[7] = (regData[7] & 0xF9) | 0x06;

            for (i = 0; i < 8; i++)
            {
                sh3001_write_nbytes(SH3001_ADDRESS, regAddr[i], 1, &regData[i]);
            }

            regData[7] = (regData[7] & 0x87) | 0x78;
            regData[8] = (regData[8] & 0x1F) | 0xE0;
            regData[9] = (regData[9] & 0x03) | 0xFC;

            for (i = 7; i < 10; i++)
            {
                sh3001_write_nbytes(SH3001_ADDRESS, regAddr[i], 1, &regData[i]);
            }

            return (SH3001_TRUE);

        default:
            return (SH3001_FALSE);
    }
}

/**
 * @brief       初始化SH3001
 * @param       无
 * @retval      SH3001_TRUE, 成功(0)
 *              SH3001_FALSE, 异常(1)
 */
unsigned char sh3001_init(void)
{
    unsigned char i = 0;
    unsigned char regData = 0;

    iic_init(); /* 初始化IIC */
    
    /* 读取CHIP ID */
    do
    {
        sh3001_read_nbytes(SH3001_ADDRESS, SH3001_CHIP_ID, 1, &regData);
    } while ((regData != 0x61) && (i++ < 3));

    if (regData != 0x61)
    {
        printf("SH3001 CHIP ID:0X%X\r\n", regData); /* 打印错误的ID */
        return SH3001_FALSE;
    }

    sh3001_module_reset();  /* 重置内部模块 */

    /* ACC配置: 500Hz, 16G, cut off Freq(BW)=500*0.25Hz=125Hz, enable filter; */
    sh3001_acc_config(SH3001_ODR_500HZ,
                      SH3001_ACC_RANGE_16G,
                      SH3001_ACC_ODRX025,
                      SH3001_ACC_FILTER_EN);

    /* GYRO配置: 500Hz, X\Y\Z 2000deg/s, cut off Freq(BW)=181Hz, enable filter; */
    sh3001_gyro_config( SH3001_ODR_500HZ,
                        SH3001_GYRO_RANGE_2000,
                        SH3001_GYRO_RANGE_2000,
                        SH3001_GYRO_RANGE_2000,
                        SH3001_GYRO_ODRX00,
                        SH3001_GYRO_FILTER_EN);

    /* 温度配置: 输出速率63Hz, 使能温度测量 */
    sh3001_temp_config(SH3001_TEMP_ODR_63, SH3001_TEMP_EN);

    /* SH3001进入正常工作模式 */
    sh3001_switch_powermode(SH3001_NORMAL_MODE);

    /* 读取补偿系数 */
    sh3001_comp_init(&g_compcoef);

    return SH3001_TRUE;
}

/**
 * @brief       读温度值
 * @param       无
 * @retval      温度值,单位为℃(float类型)
 */
float sh3001_get_temprate(void)
{
    unsigned char regData[2] = {0};
    unsigned short int tempref[2] = {0};

    /* 读取温度传感器数据 12bits;   SH3001_TEMP_CONF0..SH3001_TEMP_CONF1 */
    sh3001_read_nbytes(SH3001_ADDRESS, SH3001_TEMP_CONF0, 2, &regData[0]);
    tempref[0] = ((unsigned short int)(regData[0] & 0x0F) << 8) | regData[1];

    sh3001_read_nbytes(SH3001_ADDRESS, SH3001_TEMP_ZL, 2, &regData[0]);
    tempref[1] = ((unsigned short int)(regData[1] & 0x0F) << 8) | regData[0];

    return ( (((float)(tempref[1] - tempref[0])) / 16.0f) + 25.0f );
}

/**
 * @brief       读取SH3001陀螺仪和加速度的数据(未作补偿!不推荐,仅供调试)
 * @param       accData[3]  : acc X,Y,Z;
 * @param       gyroData[3] : gyro X,Y,Z;
 * @retval      无
 */
void sh3001_get_imu_data( short accData[3], short gyroData[3] )
{
    unsigned char regData[12] = {0};

    sh3001_read_nbytes(SH3001_ADDRESS, SH3001_ACC_XL, 12, regData);
    accData[0] = ((short)regData[1] << 8) | regData[0];
    accData[1] = ((short)regData[3] << 8) | regData[2];
    accData[2] = ((short)regData[5] << 8) | regData[4];

    gyroData[0] = ((short)regData[7] << 8) | regData[6];
    gyroData[1] = ((short)regData[9] << 8) | regData[8];
    gyroData[2] = ((short)regData[11] << 8) | regData[10];

    //printf("%d %d %d %d %d %d\r\n", accData[0], accData[1], accData[2], gyroData[0], gyroData[1], gyroData[2]);
}

/**
 * @brief       读取补偿后SH3001陀螺仪和加速度的数据(推荐使用)
 * @param       accData[3]  : acc X,Y,Z;
 * @param       gyroData[3] : gyro X,Y,Z;
 * @retval      无
 */
void sh3001_get_imu_compdata(short accData[3], short gyroData[3])
{
    unsigned char regData[15] = {0};
    unsigned char paramP;
    int accTemp[3], gyroTemp[3];

    sh3001_read_nbytes(SH3001_ADDRESS, SH3001_ACC_XL, 15, regData);
    accData[0] = ((short)regData[1] << 8) | regData[0];
    accData[1] = ((short)regData[3] << 8) | regData[2];
    accData[2] = ((short)regData[5] << 8) | regData[4];
    gyroData[0] = ((short)regData[7] << 8) | regData[6];
    gyroData[1] = ((short)regData[9] << 8) | regData[8];
    gyroData[2] = ((short)regData[11] << 8) | regData[10];
    paramP = regData[14] & 0x1F;

    accTemp[0] = (int)( accData[0] + \
                        accData[1] * ((float)g_compcoef.cXY / 1024.0f) + \
                        accData[2] * ((float)g_compcoef.cXZ / 1024.0f) );

    accTemp[1] = (int)( accData[0] * ((float)g_compcoef.cYX / 1024.0f) + \
                        accData[1] + \
                        accData[2] * ((float)g_compcoef.cYZ / 1024.0f) );

    accTemp[2] = (int)( accData[0] * ((float)g_compcoef.cZX / 1024.0f) + \
                        accData[1] * ((float)g_compcoef.cZY / 1024.0f) + \
                        accData[2] );

    if (accTemp[0] > 32767)
    {
        accTemp[0] = 32767;
    }
    else if (accTemp[0] < -32768)
    {
        accTemp[0] = -32768;
    }

    if (accTemp[1] > 32767)
    {
        accTemp[1] = 32767;
    }
    else if (accTemp[1] < -32768)
    {
        accTemp[1] = -32768;
    }

    if (accTemp[2] > 32767)
    {
        accTemp[2] = 32767;
    }
    else if (accTemp[2] < -32768)
    {
        accTemp[2] = -32768;
    }

    accData[0] = (short)accTemp[0];
    accData[1] = (short)accTemp[1];
    accData[2] = (short)accTemp[2];

    gyroTemp[0] = gyroData[0] - (paramP - g_compcoef.paramP0) * g_compcoef.jX * g_compcoef.xMulti;
    gyroTemp[1] = gyroData[1] - (paramP - g_compcoef.paramP0) * g_compcoef.jY * g_compcoef.yMulti;
    gyroTemp[2] = gyroData[2] - (paramP - g_compcoef.paramP0) * g_compcoef.jZ * g_compcoef.zMulti;

    if (gyroTemp[0] > 32767)
    {
        gyroTemp[0] = 32767;
    }
    else if (gyroTemp[0] < -32768)
    {
        gyroTemp[0] = -32768;
    }

    if (gyroTemp[1] > 32767)
    {
        gyroTemp[1] = 32767;
    }
    else if (gyroTemp[1] < -32768)
    {
        gyroTemp[1] = -32768;
    }

    if (gyroTemp[2] > 32767)
    {
        gyroTemp[2] = 32767;
    }
    else if (gyroTemp[2] < -32768)
    {
        gyroTemp[2] = -32768;
    }

    gyroData[0] = (short)gyroTemp[0];
    gyroData[1] = (short)gyroTemp[1];
    gyroData[2] = (short)gyroTemp[2];

    //printf("%d %d %d %d %d %d\r\n", accData[0], accData[1], accData[2], gyroData[0], gyroData[1], gyroData[2]);
}



















