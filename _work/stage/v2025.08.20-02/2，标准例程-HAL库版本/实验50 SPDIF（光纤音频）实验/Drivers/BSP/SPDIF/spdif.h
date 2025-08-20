/**
 ****************************************************************************************************
 * @file        spdif.h
 * @author      正点原子团队(ALIENTEK)
 * @version     V1.0
 * @date        2022-09-06
 * @brief       SPDIF 驱动代码
 * @license     Copyright (c) 2020-2032, 广州市星翼电子科技有限公司
 ****************************************************************************************************
 * @attention
 *
 * 实验平台:正点原子 阿波罗 H743开发板
 * 在线视频:www.yuanzige.com
 * 技术论坛:www.openedv.com
 * 公司网址:www.alientek.com
 * 购买地址:openedv.taobao.com
 *
 * 修改说明
 * V1.0 20220906
 * 第一次发布
 *
 ****************************************************************************************************
 */


#ifndef __SPDIF_H
#define __SPDIF_H

#include "./SYSTEM/sys/sys.h"


/* SPDIF控制结构体 */
typedef struct
{
    uint8_t consta;          /* 连接状态，0 未连接;1 连接上 */
    uint8_t saisync;         /* 与SAI时钟是否同步,0,未同步;1,已同步. */
    uint32_t samplerate;     /* SPDIF采样率 */
    uint32_t clock;          /* SPDIF时钟频率 */
}spdif_struct;

extern spdif_struct spdif_dev;
extern void (*spdif_rx_stop_callback)(void);    /* SPDIF RX停止时的回调函数 */

/* SPDIF RX的三种模式设置 */
#define SPDIF_RX_IDLE   0   /* IDLE模式 */
#define SPDIF_RX_SYNC   1   /* 同步模式 */
#define SPDIF_RX_RCV    3   /* 正常工作模式 */

/******************************************************************************************/

void spdif_rx_init(void); 
void spdif_rx_mode(uint8_t mode);
void spdif_rx_dma_init(uint32_t *buf0, uint32_t *buf1, uint16_t num, uint8_t width);
uint8_t spdif_rx_waitsync(void);
uint32_t spdif_rx_getsamplerate(void);
void spdif_play_start(void);
void spdif_play_stop(void); 

#endif

