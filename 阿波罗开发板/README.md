# STM32H743 LTDC显示驱动项目

## 开发工作流程
1. **问题分析**：向Claude描述问题，获取技术分析
2. **方案制定**：使用ChatGPT结合技术手册生成任务卡
3. **任务执行**：Claude根据任务卡直接修改代码
4. **验证测试**：编译烧录验证结果

## 项目概述
基于STM32H743的LTDC（LCD-TFT Display Controller）显示驱动，实现480x272 RGB565显示屏的8色彩条测试。

## 硬件配置
- **MCU**: STM32H743VIT6
- **显示屏**: 480x272 RGB565 TFT LCD
- **时钟**: HSE 25MHz → PLL3R 10MHz像素时钟
- **帧缓冲**: D1 SRAM @ 0x24000080
- **SDRAM**: W9825G6KH (32MB, 16-bit) @ 0xC0000000

## 关键技术参数
| 参数 | 值 | 说明 |
|-----|-----|-----|
| FB_WIDTH | 480 | 显示宽度（像素） |
| FB_HEIGHT | 272 | 显示高度（像素） |
| BYTES_PER_PIXEL | 2 | RGB565格式 |
| STRIDE_PIXELS | 480 | 行宽（像素） |
| CFBLR | 0x03C003C0 | 步幅寄存器（960字节） |
| CFBLNR | 0x00000110 | 行数（272） |

## 核心功能
### DMA2D硬件加速
- 使用R2M（Register-to-Memory）模式
- 像素口径计算：
  - OMAR = CFBAR + (y * STRIDE_PIXELS + x) * BYTES_PER_PIXEL
  - OOR = STRIDE_PIXELS - width（像素单位）
  - NLR = (width << 16) | height（像素单位）

### 时钟配置
```
PLL3: M=25, N=160, R=16 → 10MHz LTDC像素时钟
```

## 验证指标
- ✅ 启动时间 ≤200ms
- ✅ LTDCclk = 10000000 Hz
- ✅ 8条垂直彩条，无倾斜
- ✅ DMA2D传输：TCIF=1, CEIF=0
- ✅ 彩条取样：[0]=F800, [60]=FFE0, [120]=07E0, [180]=07FF

## 编译与烧录
```bash
# 使用STM32CubeIDE编译
# 烧录到目标板
# 串口查看调试信息（115200 8N1）
```

## SDRAM配置
- **型号**: W9825G6KH (32MB, 16-bit)
- **基址**: 0xC0000000 (Bank1)
- **参数**: CAS=2, SDCLK=HCLK/2, REFRESH=760
- **初始化**: 使用BSP模块（bsp_fmc_sdram.c），不修改.ioc

### SDRAM_DEBUG调试宏
- **位置**: `Core/Inc/bsp_fmc_sdram.h`
- **默认值**: 0（关闭详细日志）
- **功能**: 控制测试函数和详细打印的编译

### 运行时输出（极简4行）
```
SDRAM OK: base=0xC0000000, SDCLK=HCLK/2, CAS=2, REFRESH=760
SDSR=Normal, BUSY=0
LTDCclk≈10000000Hz, CFBAR=0xC0000000, CFBLR=0x03C003C0, CFBLNR=0x0110
Boot: 24 ms
```

## 文件结构
- `/Core/Src/ltdc.c` - LTDC和DMA2D核心实现
- `/Core/Src/main.c` - 主程序入口
- `/Core/Src/bsp_fmc_sdram.c` - SDRAM BSP模块
- `/Core/Inc/bsp_fmc_sdram.h` - SDRAM配置和调试宏
- `/Core/Inc/stm32h7xx_hal_conf.h` - HAL配置
- `/CLAUDE.md` - Claude AI工作指南
- `/README.md` - 项目文档

## 常见问题解决
1. **hsdram1重复定义**：确保BSP模块使用extern声明，仅在fmc.c中定义
2. **编译警告**：检查函数是否需要导出到.h文件声明
3. **SDRAM访问失败**：检查MPU是否屏蔽0xC0000000访问

## 注意事项
1. D-Cache需要正确处理（Invalidate after DMA2D）
2. CFBLR寄存器值固定，不要修改
3. 保持唯一重载点原则
4. 使用任务卡方式进行开发迭代
5. 健康检查必须从寄存器实时读取，禁止硬编码