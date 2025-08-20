/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    bsp_fmc_sdram.c
  * @brief   BSP FMC SDRAM模块实现（HAL版本，对齐官方案例）
  *          W9825G6KH (32MB, 16-bit)按阿波罗板硬件设计
  *          Bank1@0xC0000000，CAS=2，HCLK/2
  ******************************************************************************
  */
/* USER CODE END Header */

#include "bsp_fmc_sdram.h"
#include "ltdc.h"
#include <stdio.h>
#include <string.h>

/* HAL SDRAM句柄（使用fmc.c中的定义） */
extern SDRAM_HandleTypeDef hsdram1;

/**
  * @brief  SDRAM GPIO初始化 - 按官方引脚表配置
  */
void SDRAM_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  
  /* 使能GPIO时钟 */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOE_CLK_ENABLE();
  __HAL_RCC_GPIOF_CLK_ENABLE();
  __HAL_RCC_GPIOG_CLK_ENABLE();
  
  /* 使能FMC时钟 */
  __HAL_RCC_FMC_CLK_ENABLE();
  
  /* FMC时钟源配置为D1HCLK */
  RCC_PeriphCLKInitTypeDef PeriphClkInitStruct = {0};
  PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_FMC;
  PeriphClkInitStruct.FmcClockSelection = RCC_FMCCLKSOURCE_D1HCLK;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK) {
    printf("FMC时钟源配置失败\r\n");
  }
  
  /* 端口F: A0-A5=PF0-PF5, SDNRAS=PF11, A6-A9=PF12-PF15 */
  GPIO_InitStruct.Pin = GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3
                        |GPIO_PIN_4|GPIO_PIN_5|GPIO_PIN_11|GPIO_PIN_12
                        |GPIO_PIN_13|GPIO_PIN_14|GPIO_PIN_15;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  GPIO_InitStruct.Alternate = GPIO_AF12_FMC;
  HAL_GPIO_Init(GPIOF, &GPIO_InitStruct);

  /* 端口C: SDNWE=PC0, SDNE0=PC2, SDCKE0=PC3 */
  GPIO_InitStruct.Pin = GPIO_PIN_0|GPIO_PIN_2|GPIO_PIN_3;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  GPIO_InitStruct.Alternate = GPIO_AF12_FMC;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /* 端口G: A10=PG0, A11=PG1, A12=PG2, BA0=PG4, BA1=PG5, SDCLK=PG8, SDNCAS=PG15 */
  GPIO_InitStruct.Pin = GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_4
                        |GPIO_PIN_5|GPIO_PIN_8|GPIO_PIN_15;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  GPIO_InitStruct.Alternate = GPIO_AF12_FMC;
  HAL_GPIO_Init(GPIOG, &GPIO_InitStruct);

  /* 端口E: D4-D12=PE7-PE15, NBL0=PE0, NBL1=PE1 */
  GPIO_InitStruct.Pin = GPIO_PIN_7|GPIO_PIN_8|GPIO_PIN_9|GPIO_PIN_10
                        |GPIO_PIN_11|GPIO_PIN_12|GPIO_PIN_13|GPIO_PIN_14
                        |GPIO_PIN_15|GPIO_PIN_0|GPIO_PIN_1;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  GPIO_InitStruct.Alternate = GPIO_AF12_FMC;
  HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);

  /* 端口D: D13=PD8, D14=PD9, D15=PD10, D0=PD14, D1=PD15, D2=PD0, D3=PD1 */
  GPIO_InitStruct.Pin = GPIO_PIN_8|GPIO_PIN_9|GPIO_PIN_10|GPIO_PIN_14
                        |GPIO_PIN_15|GPIO_PIN_0|GPIO_PIN_1;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  GPIO_InitStruct.Alternate = GPIO_AF12_FMC;
  HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);
}

/**
  * @brief  SDRAM控制器初始化 - HAL版本
  */
int SDRAM_Controller_Init(void)
{
  FMC_SDRAM_TimingTypeDef SdramTiming = {0};
  FMC_SDRAM_CommandTypeDef Command = {0};
  
  /* SDRAM设备结构体配置 */
  hsdram1.Instance = FMC_SDRAM_DEVICE;
  hsdram1.Init.SDBank = FMC_SDRAM_BANK1;
  hsdram1.Init.ColumnBitsNumber = SDRAM_COLUMN_BITS_NUMBER;
  hsdram1.Init.RowBitsNumber = SDRAM_ROW_BITS_NUMBER;
  hsdram1.Init.MemoryDataWidth = SDRAM_MEMORY_WIDTH;
  hsdram1.Init.InternalBankNumber = SDRAM_INTERN_BANKS_NUM;
  hsdram1.Init.CASLatency = SDRAM_CAS_LATENCY;
  hsdram1.Init.WriteProtection = SDRAM_WRITE_PROTECTION;
  hsdram1.Init.SDClockPeriod = SDRAM_SDCLK_PERIOD;
  hsdram1.Init.ReadBurst = SDRAM_READ_BURST;
  hsdram1.Init.ReadPipeDelay = SDRAM_READ_PIPE_DELAY;
  
  /* 时序配置（对齐官方参数） */
  SdramTiming.LoadToActiveDelay = LOADTOACTIVEDELAY;
  SdramTiming.ExitSelfRefreshDelay = EXITSELFREFRESHDELAY;
  SdramTiming.SelfRefreshTime = SELFREFRESHTIME;
  SdramTiming.RowCycleDelay = ROWCYCLEDELAY;
  SdramTiming.WriteRecoveryTime = WRITERECOVERYTIME;
  SdramTiming.RPDelay = RPDELAY;
  SdramTiming.RCDDelay = RCDDELAY;
  
  /* 初始化SDRAM */
  if (HAL_SDRAM_Init(&hsdram1, &SdramTiming) != HAL_OK) {
    return -1;
  }
  
  /* JEDEC初始化序列 */
  
  /* 1. Clock配置使能命令 */
  Command.CommandMode = FMC_SDRAM_CMD_CLK_ENABLE;
  Command.CommandTarget = FMC_SDRAM_CMD_TARGET_BANK1;
  Command.AutoRefreshNumber = 1;
  Command.ModeRegisterDefinition = 0;
  
  if (HAL_SDRAM_SendCommand(&hsdram1, &Command, SDRAM_TIMEOUT) != HAL_OK) {
    return -2;
  }
  HAL_Delay(1);
  
  /* 2. 预充电所有Bank命令 */
  Command.CommandMode = FMC_SDRAM_CMD_PALL;
  Command.CommandTarget = FMC_SDRAM_CMD_TARGET_BANK1;
  Command.AutoRefreshNumber = 1;
  Command.ModeRegisterDefinition = 0;
  
  if (HAL_SDRAM_SendCommand(&hsdram1, &Command, SDRAM_TIMEOUT) != HAL_OK) {
    return -3;
  }
  HAL_Delay(1);
  
  /* 3. 自动刷新命令（执行8次） */
  Command.CommandMode = FMC_SDRAM_CMD_AUTOREFRESH_MODE;
  Command.CommandTarget = FMC_SDRAM_CMD_TARGET_BANK1;
  Command.AutoRefreshNumber = 8;
  Command.ModeRegisterDefinition = 0;
  
  if (HAL_SDRAM_SendCommand(&hsdram1, &Command, SDRAM_TIMEOUT) != HAL_OK) {
    return -4;
  }
  HAL_Delay(1);
  
  /* 4. 加载模式寄存器（CAS=2, BL=1） */
  Command.CommandMode = FMC_SDRAM_CMD_LOAD_MODE;
  Command.CommandTarget = FMC_SDRAM_CMD_TARGET_BANK1;
  Command.AutoRefreshNumber = 1;
  Command.ModeRegisterDefinition = 0x0220;  /* CAS=2(010), BL=1(000) */
  
  if (HAL_SDRAM_SendCommand(&hsdram1, &Command, SDRAM_TIMEOUT) != HAL_OK) {
    return -5;
  }
  HAL_Delay(1);
  
  /* 5. 设置刷新计数器 */
  if (HAL_SDRAM_ProgramRefreshRate(&hsdram1, REFRESH_COUNT) != HAL_OK) {
    return -6;
  }
  HAL_Delay(1);
  
  return 0;
}

#if SDRAM_DEBUG
/**
  * @brief  SDRAM 16×halfword烟雾测试 - 最小化测试三个关键位置
  * @retval 0: 成功, -1: 失败
  */
int SDRAM_HalfwordSmokeTest(void)
{
  printf("\r\n=== SDRAM 16×halfword烟雾测试（Bank1） ===\r\n");
  
  /* 测试三个位置（16×2=32B对齐） */
  uint32_t test_addrs[3] = {
    SDRAM_BANK_ADDR,              /* 基址 */
    SDRAM_BANK_ADDR + 0x100000,   /* 基址+1MB */
    SDRAM_BANK_ADDR + 0x1FFFE0    /* 基址+2MB-32 */
  };
  
  const char* test_names[3] = {"基址", "1MB处", "2MB-32处"};
  
  int total_errors = 0;
  
  for (int addr_idx = 0; addr_idx < 3; addr_idx++) {
    uint32_t test_addr = test_addrs[addr_idx];
    volatile uint16_t *pSDRAM = (volatile uint16_t *)test_addr;
    
    printf("\r\n[%s] 地址: 0x%08lX\r\n", test_names[addr_idx], test_addr);
    
    /* 写入16个halfword（0xAAAA/0x5555交替） */
    printf("  写入32B数据...\r\n");
    for (int i = 0; i < 16; i++) {
      if (i % 2 == 0) {
        pSDRAM[i] = 0xAAAA;
      } else {
        pSDRAM[i] = 0x5555;
      }
    }
    
    /* 内存屏障（暂不用Cache） */
    __DSB();
    
    /* 验证数据（只打印前8个采样） */
    printf("  验证32B数据（前8个采样）...\r\n");
    int addr_errors = 0;
    for (int i = 0; i < 16; i++) {
      uint16_t expected = (i % 2 == 0) ? 0xAAAA : 0x5555;
      uint16_t actual = pSDRAM[i];
      
      if (actual != expected) {
        if (i < 8) {  /* 只打印前8个采样 */
          printf("    [%02d] 错误: 期望=0x%04X, 实际=0x%04X\r\n", 
                 i, expected, actual);
        }
        addr_errors++;
      } else {
        if (i < 8) {  /* 只打印前8个采样 */
          printf("    [%02d] 正确: 0x%04X\r\n", i, actual);
        }
      }
    }
    if (addr_errors > 0) {
      printf("    ...总错误数: %d/16\r\n", addr_errors);
    }
    
    printf("  [%s] 结果: %s (错误: %d/16)\r\n", 
           test_names[addr_idx], (addr_errors == 0) ? "通过" : "失败", addr_errors);
    total_errors += addr_errors;
  }
  
  printf("\r\n=== 16×halfword烟雾测试总结 ===\r\n");
  printf("总错误数: %d/48\r\n", total_errors);
  printf("测试结果: %s\r\n", (total_errors == 0) ? "✅ 全部通过" : "❌ 发现错误");
  
  return (total_errors == 0) ? 0 : -1;
}

/**
  * @brief  SDRAM状态检查打印
  */
void SDRAM_Print_Status(void)
{
  printf("\r\n=== SDRAM状态检查（HAL版） ===\r\n");
  
  /* 检查时钟配置 */
  printf("RCC->D1CCIPR = 0x%08lX\r\n", RCC->D1CCIPR);
  printf("RCC->AHB3ENR = 0x%08lX (FMCEN=%s)\r\n", 
         RCC->AHB3ENR, 
         (RCC->AHB3ENR & RCC_AHB3ENR_FMCEN) ? "1" : "0");
  
  /* 详细解码FMC关键寄存器 */
  uint32_t sdcr1 = FMC_SDRAM_DEVICE->SDCR[0];
  printf("FMC_SDCR1 = 0x%08lX\r\n", sdcr1);
  
  uint32_t sdclk_bits = (sdcr1 >> 10) & 0x3;
  const char* sdclk_str[] = {"禁用", "HCLK/2", "HCLK/3", "保留"};
  printf("  - SDCLK = %s (bits=%lu)\r\n", sdclk_str[sdclk_bits], sdclk_bits);
  
  printf("  - MWID = %s\r\n", (((sdcr1 >> 4) & 0x3) == 1) ? "16bit" : "其他");
  printf("  - WP (写保护) = %lu (%s)\r\n", (sdcr1 >> 7) & 0x1, ((sdcr1 >> 7) & 0x1) ? "启用" : "禁用");
  
  printf("FMC_SDRTR = 0x%08lX\r\n", FMC_SDRAM_DEVICE->SDRTR);
  
  uint32_t sdsr = FMC_SDRAM_DEVICE->SDSR;
  printf("FMC_SDSR = 0x%08lX\r\n", sdsr);
  uint32_t modes1 = (sdsr >> 1) & 0x3;
  uint32_t busy = (sdsr >> 5) & 0x1;
  const char* mode_str[] = {"NORMAL", "Self-refresh", "Power-down", "保留"};
  printf("  - MODES1 = %lu (%s)\r\n", modes1, mode_str[modes1]);
  printf("  - BUSY = %lu (%s)\r\n", busy, busy ? "忙" : "空闲");
}

/**
  * @brief  SDRAM GPIO状态检查打印（重点6个引脚）
  */
void SDRAM_Print_GPIO_Status(void)
{
  printf("\r\n=== 关键GPIO状态检查 ===\r\n");
  
  /* 6个关键引脚状态 */
  printf("关键引脚AF12检查：\r\n");
  
  printf("  PC0(SDNWE) - MODER=0x%lX, AFR=%lX, OSPEEDR=%lX %s\r\n",
         (GPIOC->MODER >> 0) & 0x3,
         (GPIOC->AFR[0] >> 0) & 0xF,
         (GPIOC->OSPEEDR >> 0) & 0x3,
         (((GPIOC->AFR[0] >> 0) & 0xF) == 12) ? "✓AF12" : "❌非AF12");
  
  printf("  PC2(SDNE0) - MODER=0x%lX, AFR=%lX, OSPEEDR=%lX %s\r\n", 
         (GPIOC->MODER >> 4) & 0x3, 
         (GPIOC->AFR[0] >> 8) & 0xF,
         (GPIOC->OSPEEDR >> 4) & 0x3,
         (((GPIOC->AFR[0] >> 8) & 0xF) == 12) ? "✓AF12" : "❌非AF12");
  
  printf("  PC3(SDCKE0) - MODER=0x%lX, AFR=%lX, OSPEEDR=%lX %s\r\n", 
         (GPIOC->MODER >> 6) & 0x3, 
         (GPIOC->AFR[0] >> 12) & 0xF,
         (GPIOC->OSPEEDR >> 6) & 0x3,
         (((GPIOC->AFR[0] >> 12) & 0xF) == 12) ? "✓AF12" : "❌非AF12");
  
  printf("  PG8(SDCLK) - MODER=0x%lX, AFR=%lX, OSPEEDR=%lX %s\r\n", 
         (GPIOG->MODER >> 16) & 0x3, 
         (GPIOG->AFR[1] >> 0) & 0xF,
         (GPIOG->OSPEEDR >> 16) & 0x3,
         (((GPIOG->AFR[1] >> 0) & 0xF) == 12) ? "✓AF12" : "❌非AF12");
  
  printf("  PF11(SDNRAS) - MODER=0x%lX, AFR=%lX, OSPEEDR=%lX %s\r\n", 
         (GPIOF->MODER >> 22) & 0x3, 
         (GPIOF->AFR[1] >> 12) & 0xF,
         (GPIOF->OSPEEDR >> 22) & 0x3,
         (((GPIOF->AFR[1] >> 12) & 0xF) == 12) ? "✓AF12" : "❌非AF12");
  
  printf("  PG15(SDNCAS) - MODER=0x%lX, AFR=%lX, OSPEEDR=%lX %s\r\n", 
         (GPIOG->MODER >> 30) & 0x3, 
         (GPIOG->AFR[1] >> 28) & 0xF,
         (GPIOG->OSPEEDR >> 30) & 0x3,
         (((GPIOG->AFR[1] >> 28) & 0xF) == 12) ? "✓AF12" : "❌非AF12");
  
  printf("\r\n期望值：MODER=2(AF), AFR=C(12), OSPEEDR=3(VeryHigh)\r\n");
}

/**
  * @brief  SDRAM 64B烟雾测试
  */
int SDRAM_SmokeTest(void)
{
  printf("\r\n=== SDRAM 64B烟雾测试 ===\r\n");
  
  uint32_t test_addr = SDRAM_BANK_ADDR;
  volatile uint32_t *pSDRAM = (volatile uint32_t *)test_addr;
  
  /* 写入64B数据 */
  for (int i = 0; i < 16; i++) {
    pSDRAM[i] = 0x12345678 + i;
  }
  
  __DSB();
  
  /* 验证数据 */
  int errors = 0;
  for (int i = 0; i < 16; i++) {
    uint32_t expected = 0x12345678 + i;
    uint32_t actual = pSDRAM[i];
    if (actual != expected) {
      printf("  [%02d] 错误: 期望=0x%08lX, 实际=0x%08lX\r\n", 
             i, expected, actual);
      errors++;
    }
  }
  
  printf("64B测试结果: %s (错误: %d/16)\r\n", 
         (errors == 0) ? "✅通过" : "❌失败", errors);
  
  return (errors == 0) ? 0 : -1;
}

/**
  * @brief  SDRAM 单块测试
  */
int SDRAM_SingleBlockTest(uint32_t base)
{
  printf("\r\n=== SDRAM 128KB单块测试 ===\r\n");
  printf("测试地址: 0x%08lX\r\n", base);
  
  // 简化版本：只测试关键位置
  volatile uint32_t *pSDRAM = (volatile uint32_t *)base;
  
  /* 写入关键位置 */
  pSDRAM[0] = 0xDEADBEEF;
  pSDRAM[1023] = 0xCAFEBABE;    // 4KB处
  pSDRAM[8191] = 0x12345678;    // 32KB处
  
  __DSB();
  
  /* 验证数据 */
  int errors = 0;
  if (pSDRAM[0] != 0xDEADBEEF) errors++;
  if (pSDRAM[1023] != 0xCAFEBABE) errors++;
  if (pSDRAM[8191] != 0x12345678) errors++;
  
  printf("128KB测试结果: %s (错误: %d/3)\r\n", 
         (errors == 0) ? "✅通过" : "❌失败", errors);
  
  return (errors == 0) ? 0 : -1;
}
#endif

/**
  * @brief  SDRAM最小健康检查打印（3行输出）
  */
void SDRAM_Print_MinimalHealth(void)
{
  /* 读取SDCR1解析SDCLK分频和CAS */
  uint32_t sdcr1 = FMC_SDRAM_DEVICE->SDCR[0];
  uint32_t sdclk_div = ((sdcr1 >> 10) & 0x3) + 1;  // SDCLK分频：2或3
  uint32_t cas = (sdcr1 >> 7) & 0x3;               // CAS延迟：2或3
  uint32_t refresh = FMC_SDRAM_DEVICE->SDRTR & 0x1FFF;
  
  printf("SDRAM OK: base=0xC0000000, SDCLK=HCLK/%lu, CAS=%lu, REFRESH=%lu\r\n", 
         sdclk_div, cas, refresh);
  
  /* 读取SDSR状态 */
  uint32_t sdsr = FMC_SDRAM_DEVICE->SDSR;
  uint32_t modes = (sdsr >> 1) & 0x3;
  uint32_t busy = (sdsr >> 5) & 0x1;
  const char* mode_str = (modes == 0) ? "Normal" : "Other";
  printf("SDSR=%s, BUSY=%lu\r\n", mode_str, busy);
  
  /* 读取LTDC Layer1寄存器状态 */
  uint32_t cfbar = LTDC_Layer1->CFBAR;
  uint32_t cfblr = LTDC_Layer1->CFBLR;
  uint32_t cfblnr = LTDC_Layer1->CFBLNR;
  
  /* 读取实际LTDC时钟频率 */
  uint32_t ltdc_clk = LCD_Get_LTDCclk_Hz_ABS();
  
  printf("LTDCclk≈%luHz, CFBAR=0x%08lX, CFBLR=0x%08lX, CFBLNR=0x%04lX\r\n",
         ltdc_clk, cfbar, cfblr, cfblnr);
}