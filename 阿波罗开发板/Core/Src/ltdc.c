/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    ltdc.c
  * @brief   This file provides code for the configuration
  *          of the LTDC instances.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "ltdc.h"

/* USER CODE BEGIN 0 */
#include <stdint.h>
#include <stdio.h>

/* 宏定义 */
#ifndef DISP_FB_DOMAIN
#define DISP_FB_DOMAIN AXI_D1
#endif

#ifndef DISP_VERBOSE_LOG
#define DISP_VERBOSE_LOG 0
#endif

/* 日志分级宏 - 默认关闭详细日志 */
#ifndef LOG_VERBOSE
#define LOG_VERBOSE 0
#endif

/* 帧缓冲参数 - 全局只读常量 */
static const uint32_t FB_WIDTH = 480;
static const uint32_t FB_HEIGHT = 272;
static const uint32_t BYTES_PER_PIXEL = 2;  /* RGB565: 2 bytes/pixel */
static const uint32_t STRIDE_PIXELS = 480;  /* 帧缓冲行宽，像素单位 */
// static const uint32_t FB_PIXFMT = LTDC_PIXEL_FORMAT_RGB565;  // 未使用，注释掉
static const uint32_t FB_LINE_COUNT = 272;

/* D1/AXI SRAM 放 480x272 的 RGB565 帧缓冲；32B 对齐便于 D-Cache 清理 */
__attribute__((section(".RAM_D1"), aligned(32)))
static uint16_t s_fb_480x272[480 * 272];

/* 使用CMSIS定义的DMA2D寄存器访问，避免硬编码偏移错误 */

/* DMA2D寄存器位定义 */
#define DMA2D_CR_START_BIT      (1U << 0)
#define DMA2D_CR_MODE_R2M       (3U << 16)  /* Register-to-Memory mode */
#define DMA2D_OPFCCR_RGB565     (2U << 0)   /* RGB565 format */
/* 使用CMSIS定义的DMA2D_ISR_TCIF和DMA2D_IFCR_CTCIF */

/* DMA2D寄存器级操作 - 强化初始化 */
static void DMA2D_Init(void)
{
    /* 使能DMA2D时钟 */
    RCC->AHB3ENR |= RCC_AHB3ENR_DMA2DEN;
    __DSB();  /* 数据同步屏障，确保时钟生效 */
    
    /* 软复位DMA2D */
    RCC->AHB3RSTR |= RCC_AHB3RSTR_DMA2DRST;
    __DSB();
    RCC->AHB3RSTR &= ~RCC_AHB3RSTR_DMA2DRST;
    __DSB();
    
    /* 等待复位完成 */
    for(volatile int i = 0; i < 1000; i++);
    
    /* 清除所有寄存器和中断标志 */
    DMA2D->CR = 0;
    DMA2D->IFCR = 0x3F;  /* 清除所有中断标志 */
    
    /* 等待DMA2D空闲 */
    while (DMA2D->CR & DMA2D_CR_START_BIT);
    
}


/* DMA2D快速填充彩条带 - 像素口径计算，与CFBLR解耦 */
static void DMA2D_FillColorBand_Fast(uint32_t x, uint32_t y, uint32_t width, uint32_t height, uint16_t color)
{
    /* 获取LTDC Layer1的实际帧缓冲地址 */
    uint32_t cfbar_addr = LTDC_Layer1->CFBAR;
    
    /* 等待DMA2D空闲 */
    while (DMA2D->CR & DMA2D_CR_START_BIT);
    
    /* 计算起始地址：基址 + (y*stride_px + x)*bytes_per_pixel */
    uint32_t dest_addr = cfbar_addr + (y * STRIDE_PIXELS + x) * BYTES_PER_PIXEL;
    
    /* 清除传输完成标志 */
    DMA2D->IFCR = DMA2D_IFCR_CTCIF;
    
    /* 按标准顺序配置DMA2D寄存器 - 严格像素口径 */
    DMA2D->CR = DMA2D_CR_MODE_R2M;         /* 1. R2M mode */
    DMA2D->OPFCCR = DMA2D_OPFCCR_RGB565;   /* 2. RGB565格式 */
    DMA2D->OCOLR = (uint32_t)color;        /* 3. 输出颜色 */
    DMA2D->OMAR = dest_addr;               /* 4. 输出地址 */
    DMA2D->OOR = STRIDE_PIXELS - width;    /* 5. 输出偏移：行间跳跃像素数 */
    DMA2D->NLR = (width << 16) | height;  /* 6. 宽度和高度(像素) */
    
    /* 启动传输 */
    DMA2D->CR |= DMA2D_CR_START_BIT;       /* 7. 启动 */
    
    /* 等待传输完成标志TCIF */
    while (!(DMA2D->ISR & DMA2D_ISR_TCIF));
    
    /* 清除TCIF标志 */
    DMA2D->IFCR = DMA2D_IFCR_CTCIF;
}

/* 统一的Layer配置和步幅寄存器设置 - 唯一重载点 */
static void configure_layer_with_stride(void)
{
    /* 1. 先禁用图层 */
    LTDC_Layer1->CR &= ~LTDC_LxCR_LEN;
    
    /* 2. 设置帧缓冲地址 - 使用全局常量 */
    HAL_LTDC_SetAddress(&hltdc, (uint32_t)s_fb_480x272, 0);
    
    /* 3. 固化步幅寄存器：CFBLR = (CFBP << 16) | CFBLL */
    /* 固定CFBLR=0x03C003C0: CFBP=0x3C0, CFBLL=0x3C0 (960字节stride) */
    LTDC_Layer1->CFBLR = 0x03C003C0;
    LTDC_Layer1->CFBLNR = FB_LINE_COUNT;  /* 272行 */
    
    /* 4. 触发立即重载 - 全流程唯一重载点 */
    LTDC->SRCR = LTDC_SRCR_IMR;
    
    /* 5. 等待重载完成 - 简化版本 */
    volatile uint32_t timeout = 100000;  /* 简化超时 */
    while(!(LTDC->ISR & LTDC_ISR_RRIF) && --timeout);
    if (LTDC->ISR & LTDC_ISR_RRIF) {
        LTDC->ICR = LTDC_ICR_CRRIF;  /* 清除重载完成标志 */
    }
    
    /* 6. 重新使能图层和控制器 */
    __HAL_LTDC_ENABLE(&hltdc);
    __HAL_LTDC_LAYER_ENABLE(&hltdc, 0);
}





/* DMA2D快速填充8条彩条 - 基线固化版本 */
static void FB_FillColorBars_DMA2D_Fast(void)
{
    /* 8色彩条颜色定义 */
    const uint16_t colors[8] = {
        0xF800,  /* 红 */
        0xFFE0,  /* 黄 */
        0x07E0,  /* 绿 */
        0x07FF,  /* 青 */
        0x001F,  /* 蓝 */
        0xF81F,  /* 品红 */
        0xC618,  /* 灰 */
        0xFFFF   /* 白 */
    };
    
    /* 每条彩条的宽度 - 像素计算 */
    const uint32_t band_width = FB_WIDTH / 8;  /* 480 / 8 = 60像素 */
    
    /* 使用DMA2D快速填充每条彩条 */
    for (uint32_t i = 0; i < 8; i++) {
        uint32_t x_offset_px = i * band_width;
        uint32_t rect_width_px = (i == 7) ? (FB_WIDTH - x_offset_px) : band_width;
        
        DMA2D_FillColorBand_Fast(x_offset_px, 0, rect_width_px, FB_HEIGHT, colors[i]);
    }
    
    /* 所有彩条填充完成后，统一Cache Invalidate */
    uintptr_t adr = (uintptr_t)s_fb_480x272;
    uintptr_t end = adr + sizeof(s_fb_480x272);
    adr &= ~((uintptr_t)31);                /* 起始向下对齐 32B */
    end  = (end + 31) & ~((uintptr_t)31);   /* 结束向上对齐 32B */
    SCB_InvalidateDCache_by_Addr((uint32_t*)adr, (int32_t)(end - adr));
}

/* == RCC 绝对地址宏（避免结构体映射不一致） == */
#ifndef __IO
#define __IO volatile
#endif
#define RCC_BASE_ADDR     (0x58024400UL)
#define RCC_REG32(off)   (*(__IO uint32_t *)(RCC_BASE_ADDR + (off)))
#define RCC_D1CCIPR_ABS   RCC_REG32(0x4CUL)
#define RCC_D3CCIPR_ABS   RCC_REG32(0x50UL)

/* 返回当前 LTDC 内核时钟Hz —— 仅用"绝对地址"判源并计算 */
uint32_t LCD_Get_LTDCclk_Hz_ABS(void)
{
  /* 读取两个寄存器 */
  uint32_t d1 = RCC_D1CCIPR_ABS;
  uint32_t d3 = RCC_D3CCIPR_ABS;
  
  /* 检查三个可能的位置 */
  uint32_t sel_d3 = (d3 >> 20) & 0x3U;  /* D3CCIPR[21:20] */
  uint32_t sel_d1_20 = (d1 >> 20) & 0x3U;  /* D1CCIPR[21:20] */
  uint32_t sel_d1_16 = (d1 >> 16) & 0x3U;  /* D1CCIPR[17:16] */
  
  /* 优先级：D3 > D1[21:20] > D1[17:16] */
  uint32_t sel = 0;
  if (sel_d3 != 0) sel = sel_d3;
  else if (sel_d1_20 != 0) sel = sel_d1_20;
  else if (sel_d1_16 != 0) sel = sel_d1_16;

  if (sel == 0U) {
    /* ck_per：取 D1[1:0] 的 CKPERSEL（0=HSI64M,1=CSI4M,2=HSE25M） */
    uint32_t ckper_sel = (d1 >> 0) & 0x3U;
    if (ckper_sel == 0U) return 64000000UL;
    if (ckper_sel == 1U) return 4000000UL;
    if (ckper_sel == 2U) return 25000000UL;
    return 0U;
  } else if (sel == 2U) {
    /* PLL3R：你当前代码已配置 M=25,N=160,R=16 → 10MHz */
    return 10000000UL;
  } else if (sel == 1U) {
    /* 某些 HAL 版本：1=PLL3R (而非 PLL2R) */
    return 10000000UL;
  }
  return 0U;
}

/* USER CODE END 0 */

LTDC_HandleTypeDef hltdc;

/* LTDC init function */
void MX_LTDC_Init(void)
{

  /* USER CODE BEGIN LTDC_Init 0 */

  /* USER CODE END LTDC_Init 0 */

  LTDC_LayerCfgTypeDef pLayerCfg = {0};

  /* USER CODE BEGIN LTDC_Init 1 */
  // LCD_ClockInit_UsingPLL3_10MHz();  // 注释掉：HAL_LTDC_MspInit已配置PLL3，避免重复
  /* USER CODE END LTDC_Init 1 */
  hltdc.Instance = LTDC;
  hltdc.Init.HSPolarity = LTDC_HSPOLARITY_AL;
  hltdc.Init.VSPolarity = LTDC_VSPOLARITY_AL;
  hltdc.Init.DEPolarity = LTDC_DEPOLARITY_AH;
  hltdc.Init.PCPolarity = LTDC_PCPOLARITY_IPC;
  hltdc.Init.HorizontalSync = 3;
  hltdc.Init.VerticalSync = 3;
  hltdc.Init.AccumulatedHBP = 46;
  hltdc.Init.AccumulatedVBP = 15;
  hltdc.Init.AccumulatedActiveW = 526;
  hltdc.Init.AccumulatedActiveH = 287;
  hltdc.Init.TotalWidth = 534;
  hltdc.Init.TotalHeigh = 295;
  hltdc.Init.Backcolor.Blue = 0;
  hltdc.Init.Backcolor.Green = 0;
  hltdc.Init.Backcolor.Red = 0;
  if (HAL_LTDC_Init(&hltdc) != HAL_OK)
  {
    Error_Handler();
  }
  pLayerCfg.WindowX0 = 0;
  pLayerCfg.WindowX1 = 0;
  pLayerCfg.WindowY0 = 0;
  pLayerCfg.WindowY1 = 0;
  pLayerCfg.PixelFormat = LTDC_PIXEL_FORMAT_RGB565;
  pLayerCfg.Alpha = 0;
  pLayerCfg.Alpha0 = 0;
  pLayerCfg.BlendingFactor1 = LTDC_BLENDING_FACTOR1_CA;
  pLayerCfg.BlendingFactor2 = LTDC_BLENDING_FACTOR2_CA;
  pLayerCfg.FBStartAdress = 0x24000000;
  pLayerCfg.ImageWidth = 240;
  pLayerCfg.ImageHeight = 160;
  pLayerCfg.Backcolor.Blue = 0;
  pLayerCfg.Backcolor.Green = 0;
  pLayerCfg.Backcolor.Red = 0;
  if (HAL_LTDC_ConfigLayer(&hltdc, &pLayerCfg, 0) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN LTDC_Init 2 */
  /* 初始化DMA2D */
  DMA2D_Init();
  
  /* 统一Layer配置和重载 - 唯一重载点 */
  configure_layer_with_stride();
  
  /* 填充8条彩条 */
  FB_FillColorBars_DMA2D_Fast();
  
  
#if LOG_VERBOSE
  /* 详细寄存器日志 - 默认关闭 */
  printf("详细Layer1配置:\n");
  printf("  PFCR=%08lX WHPCR=%08lX WVPCR=%08lX CR=%08lX\n",
         (unsigned long)LTDC_Layer1->PFCR, (unsigned long)LTDC_Layer1->WHPCR, 
         (unsigned long)LTDC_Layer1->WVPCR, (unsigned long)LTDC_Layer1->CR);
#endif

  /* USER CODE END LTDC_Init 2 */

}

void HAL_LTDC_MspInit(LTDC_HandleTypeDef* ltdcHandle)
{

  GPIO_InitTypeDef GPIO_InitStruct = {0};
  RCC_PeriphCLKInitTypeDef PeriphClkInitStruct = {0};
  if(ltdcHandle->Instance==LTDC)
  {
  /* USER CODE BEGIN LTDC_MspInit 0 */

  /* USER CODE END LTDC_MspInit 0 */

  /** Initializes the peripherals clock
  */
    PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_LTDC;
    PeriphClkInitStruct.PLL3.PLL3M = 25;
    PeriphClkInitStruct.PLL3.PLL3N = 160;
    PeriphClkInitStruct.PLL3.PLL3P = 2;
    PeriphClkInitStruct.PLL3.PLL3Q = 2;
    PeriphClkInitStruct.PLL3.PLL3R = 16;
    PeriphClkInitStruct.PLL3.PLL3RGE = RCC_PLL3VCIRANGE_0;
    PeriphClkInitStruct.PLL3.PLL3VCOSEL = RCC_PLL3VCOMEDIUM;
    PeriphClkInitStruct.PLL3.PLL3FRACN = 0;
    if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK)
    {
      Error_Handler();
    }

    /* LTDC clock enable */
    __HAL_RCC_LTDC_CLK_ENABLE();

    __HAL_RCC_GPIOI_CLK_ENABLE();
    __HAL_RCC_GPIOF_CLK_ENABLE();
    __HAL_RCC_GPIOH_CLK_ENABLE();
    __HAL_RCC_GPIOG_CLK_ENABLE();
    /**LTDC GPIO Configuration
    PI9     ------> LTDC_VSYNC
    PI10     ------> LTDC_HSYNC
    PF10     ------> LTDC_DE
    PH9     ------> LTDC_R3
    PH10     ------> LTDC_R4
    PH11     ------> LTDC_R5
    PH12     ------> LTDC_R6
    PG6     ------> LTDC_R7
    PG7     ------> LTDC_CLK
    PH13     ------> LTDC_G2
    PH14     ------> LTDC_G3
    PH15     ------> LTDC_G4
    PI0     ------> LTDC_G5
    PI1     ------> LTDC_G6
    PI2     ------> LTDC_G7
    PG11     ------> LTDC_B3
    PI4     ------> LTDC_B4
    PI5     ------> LTDC_B5
    PI6     ------> LTDC_B6
    PI7     ------> LTDC_B7
    */
    GPIO_InitStruct.Pin = GPIO_PIN_9|GPIO_PIN_10|GPIO_PIN_0|GPIO_PIN_1
                          |GPIO_PIN_2|GPIO_PIN_4|GPIO_PIN_5|GPIO_PIN_6
                          |GPIO_PIN_7;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.Alternate = GPIO_AF14_LTDC;
    HAL_GPIO_Init(GPIOI, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = GPIO_PIN_10;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.Alternate = GPIO_AF14_LTDC;
    HAL_GPIO_Init(GPIOF, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = GPIO_PIN_9|GPIO_PIN_10|GPIO_PIN_11|GPIO_PIN_12
                          |GPIO_PIN_13|GPIO_PIN_14|GPIO_PIN_15;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.Alternate = GPIO_AF14_LTDC;
    HAL_GPIO_Init(GPIOH, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = GPIO_PIN_6|GPIO_PIN_7|GPIO_PIN_11;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.Alternate = GPIO_AF14_LTDC;
    HAL_GPIO_Init(GPIOG, &GPIO_InitStruct);

  /* USER CODE BEGIN LTDC_MspInit 1 */

  /* USER CODE END LTDC_MspInit 1 */
  }
}

void HAL_LTDC_MspDeInit(LTDC_HandleTypeDef* ltdcHandle)
{

  if(ltdcHandle->Instance==LTDC)
  {
  /* USER CODE BEGIN LTDC_MspDeInit 0 */

  /* USER CODE END LTDC_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_LTDC_CLK_DISABLE();

    /**LTDC GPIO Configuration
    PI9     ------> LTDC_VSYNC
    PI10     ------> LTDC_HSYNC
    PF10     ------> LTDC_DE
    PH9     ------> LTDC_R3
    PH10     ------> LTDC_R4
    PH11     ------> LTDC_R5
    PH12     ------> LTDC_R6
    PG6     ------> LTDC_R7
    PG7     ------> LTDC_CLK
    PH13     ------> LTDC_G2
    PH14     ------> LTDC_G3
    PH15     ------> LTDC_G4
    PI0     ------> LTDC_G5
    PI1     ------> LTDC_G6
    PI2     ------> LTDC_G7
    PG11     ------> LTDC_B3
    PI4     ------> LTDC_B4
    PI5     ------> LTDC_B5
    PI6     ------> LTDC_B6
    PI7     ------> LTDC_B7
    */
    HAL_GPIO_DeInit(GPIOI, GPIO_PIN_9|GPIO_PIN_10|GPIO_PIN_0|GPIO_PIN_1
                          |GPIO_PIN_2|GPIO_PIN_4|GPIO_PIN_5|GPIO_PIN_6
                          |GPIO_PIN_7);

    HAL_GPIO_DeInit(GPIOF, GPIO_PIN_10);

    HAL_GPIO_DeInit(GPIOH, GPIO_PIN_9|GPIO_PIN_10|GPIO_PIN_11|GPIO_PIN_12
                          |GPIO_PIN_13|GPIO_PIN_14|GPIO_PIN_15);

    HAL_GPIO_DeInit(GPIOG, GPIO_PIN_6|GPIO_PIN_7|GPIO_PIN_11);

  /* USER CODE BEGIN LTDC_MspDeInit 1 */

  /* USER CODE END LTDC_MspDeInit 1 */
  }
}

/* USER CODE BEGIN 1 */

/* USER CODE END 1 */
