/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    bsp_fmc_sdram.h
  * @brief   BSP FMC SDRAM模块头文件
  *          W9825G6KH (32MB, 16-bit)按阿波罗板硬件设计
  ******************************************************************************
  */
/* USER CODE END Header */

#ifndef __BSP_FMC_SDRAM_H__
#define __BSP_FMC_SDRAM_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32h7xx_hal.h"
#include <stdint.h>
#include <stddef.h>

/* External variables --------------------------------------------------------*/
extern SDRAM_HandleTypeDef hsdram1;

/* 调试开关（默认关闭） */
#define SDRAM_DEBUG 0

/* SDRAM配置参数（恢复Bank1） */
#define SDRAM_BANK_ADDR           ((uint32_t)0xC0000000U)  /* FMC Bank1基地址 */

/* SDRAM配置参数 */
#define SDRAM_SIZE                (32 * 1024 * 1024)       /* 32MB总容量 */
#define SDRAM_TIMEOUT             ((uint32_t)0xFFFF)       /* 操作超时 */

/* W9825G6KH规格参数 */
#define SDRAM_MEMORY_WIDTH        FMC_SDRAM_MEM_BUS_WIDTH_16   /* 16位数据宽度 */
#define SDRAM_CAS_LATENCY         FMC_SDRAM_CAS_LATENCY_2      /* CAS延迟2周期（官方值） */
#define SDRAM_WRITE_PROTECTION    FMC_SDRAM_WRITE_PROTECTION_DISABLE
#define SDRAM_SDCLK_PERIOD        FMC_SDRAM_CLOCK_PERIOD_2     /* SDCLK=HCLK/2 */
#define SDRAM_READ_BURST          FMC_SDRAM_RBURST_DISABLE     /* 读突发禁用（先关闭保稳） */
#define SDRAM_READ_PIPE_DELAY     FMC_SDRAM_RPIPE_DELAY_1      /* 读流水线延迟 */

/* 行列地址配置 */
#define SDRAM_ROW_BITS_NUMBER     FMC_SDRAM_ROW_BITS_NUM_13    /* 13位行地址 */
#define SDRAM_COLUMN_BITS_NUMBER  FMC_SDRAM_COLUMN_BITS_NUM_9  /* 9位列地址 */
#define SDRAM_INTERN_BANKS_NUM    FMC_SDRAM_INTERN_BANKS_NUM_4 /* 4个内部Bank */

/* 官方适配时序参数（适应HCLK/2和CAS=2） */
#define LOADTOACTIVEDELAY         2    /* tMRD: Mode Register to Active */
#define EXITSELFREFRESHDELAY      8    /* tXSR: Exit self-refresh */
#define SELFREFRESHTIME           6    /* tRAS: Self refresh time */
#define ROWCYCLEDELAY             6    /* tRC: Row cycle delay */
#define WRITERECOVERYTIME         3    /* tWR: Write recovery time */
#define RPDELAY                   2    /* tRP: Row precharge delay */
#define RCDDELAY                  2    /* tRCD: Row to column delay */

/* 刷新参数：按100MHz SDCLK重算 (200MHz HCLK/2) */
#define REFRESH_COUNT             ((uint32_t)760)   /* 64ms*100MHz/8192-20≈760 */

/* 自检参数 */
#define SDRAM_SELFTEST_ROUNDS     5                    /* 测试轮数 */
#define SDRAM_BLOCK_SIZE          (128 * 1024)        /* 分块大小128KB */
#define SDRAM_TIMEOUT_PER_ROUND   15000               /* 每轮超时15秒 */

/* 函数声明 */
void SDRAM_GPIO_Init(void);
int SDRAM_Controller_Init(void);
void SDRAM_Print_MinimalHealth(void);

#if SDRAM_DEBUG
/* 调试测试函数（仅在SDRAM_DEBUG=1时可用） */
int SDRAM_HalfwordSmokeTest(void);
int SDRAM_SmokeTest(void);
int SDRAM_SingleBlockTest(uint32_t base);
int SDRAM_SelfTest(uint32_t base, size_t bytes);
void SDRAM_Print_Status(void);
void SDRAM_Print_GPIO_Status(void);
#endif

#ifdef __cplusplus
}
#endif

#endif /* __BSP_FMC_SDRAM_H__ */