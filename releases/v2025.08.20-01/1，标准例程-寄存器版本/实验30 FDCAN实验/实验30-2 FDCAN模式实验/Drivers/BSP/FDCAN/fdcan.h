/**
 ****************************************************************************************************
 * @file        fdcan.h
 * @author      正点原子团队(ALIENTEK)
 * @version     V1.0
 * @date        2023-03-22
 * @brief       FDCAN 驱动代码
 * @license     Copyright (c) 2022-2032, 广州市星翼电子科技有限公司
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
 * V1.0 20230322
 * 第一次发布
 *
 ****************************************************************************************************
 */

#ifndef __CAN_H
#define __CAN_H

#include "./SYSTEM/sys/sys.h"
#include "./BSP/FDCAN/stm32h7xx_hal_fdcan.h"

/******************************************************************************************/
/* FDCAN 引脚 定义 */

#define FDCAN_RX_GPIO_PORT                  GPIOA
#define FDCAN_RX_GPIO_PIN                   SYS_GPIO_PIN11
#define FDCAN_RX_GPIO_CLK_ENABLE()          do{ RCC->AHB4ENR |= 1 << 0; }while(0)       /* PA口时钟使能 */ 

#define FDCAN_TX_GPIO_PORT                  GPIOA
#define FDCAN_TX_GPIO_PIN                   SYS_GPIO_PIN12
#define FDCAN_TX_GPIO_CLK_ENABLE()          do{ RCC->AHB4ENR |= 1 << 0; }while(0)       /* PA口时钟使能 */ 

/******************************************************************************************/

/* FDCAN1接收RX0中断使能 */
#define FDCAN1_RX0_INT_ENABLE   0               /* 0,不使能;1,使能. */

/* 防止加入HAL库 FDCAN驱动后,编译报错,必须实现以下3个宏 */
#define HAL_FDCAN_MODULE_ENABLED                /* 使能FDCAN模块 */
#define assert_param(expr)      ((void)0)       /* assert_param定义 */
#define HAL_GetTick()           10              /* 返回SysTick计数器值,这里没用到SysTick */



uint8_t fdcan_init(uint16_t presc, uint8_t tsjw, uint16_t ntsg1, uint8_t ntsg2, uint32_t mode);  /* CAN初始化 */
uint8_t fdcan_send_msg(uint8_t *msg,uint32_t len);                                               /* 发送数据 */
uint8_t fdcan_receive_msg(uint8_t *buf);                                                         /* 接收数据 */

#endif

