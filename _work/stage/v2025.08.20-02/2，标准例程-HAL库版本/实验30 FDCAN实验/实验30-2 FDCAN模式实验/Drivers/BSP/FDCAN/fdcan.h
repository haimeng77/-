/**
 ****************************************************************************************************
 * @file        fdcan.h
 * @author      正点原子团队(ALIENTEK)
 * @version     V1.0
 * @date        2022-09-06
 * @brief       FDCAN 驱动代码
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

#ifndef __CAN_H
#define __CAN_H

#include "./SYSTEM/sys/sys.h"

/******************************************************************************************/

/* FDCAN 引脚 定义 */
#define FDCAN_RX_GPIO_PORT                GPIOA
#define FDCAN_RX_GPIO_PIN                 GPIO_PIN_11
#define FDCAN_RX_GPIO_CLK_ENABLE()        do{ __HAL_RCC_GPIOA_CLK_ENABLE(); }while(0)            /* PA口时钟使能 */

#define FDCAN_TX_GPIO_PORT                GPIOA
#define FDCAN_TX_GPIO_PIN                 GPIO_PIN_12
#define FDCAN_TX_GPIO_CLK_ENABLE()        do{ __HAL_RCC_GPIOA_CLK_ENABLE(); }while(0)            /* PA口时钟使能 */

/* FDCAN1接收RX0中断使能 */
#define FDCAN1_RX0_INT_ENABLE   0         /* 0,不使能;1,使能. */

/******************************************************************************************/

uint8_t fdcan_init(uint16_t presc, uint8_t tsjw, uint16_t ntsg1, uint8_t ntsg2, uint32_t mode);  /* CAN初始化 */
uint8_t fdcan_send_msg(uint8_t *msg,uint32_t len);                                               /* 发送数据 */
uint8_t fdcan_receive_msg(uint8_t *buf);                                                         /* 接收数据 */

#endif

