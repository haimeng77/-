// Core/Src/retarget.c
#include "main.h"    // 里面应包含 stm32h7xx_hal.h（第1步已确认）
#include "usart.h"   // 提供 extern UART_HandleTypeDef huart1;
#include <unistd.h>

int _write(int file, char *ptr, int len)
{
  // 这里用 USART1（PA9/PA10）
  if (len > 0) HAL_UART_Transmit(&huart1, (uint8_t*)ptr, len, HAL_MAX_DELAY);
  return len;
}
