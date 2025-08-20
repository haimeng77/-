/**
 ****************************************************************************************************
 * @file        stm32h743i_eval_audio.h
 * @author      ����ԭ���Ŷ�(ALIENTEK)
 * @version     V1.0
 * @date        2022-09-06
 * @brief       stm32h743i_eval_audio ��������
 * @license     Copyright (c) 2020-2032, ������������ӿƼ����޹�˾
 ****************************************************************************************************
 * @attention
 *
 * ʵ��ƽ̨:����ԭ�� ������ H743������
 * ������Ƶ:www.yuanzige.com
 * ������̳:www.openedv.com
 * ��˾��ַ:www.alientek.com
 * �����ַ:openedv.taobao.com
 *
 * �޸�˵��
 * V1.0 20220906
 * ��һ�η���
 *
 ****************************************************************************************************
 */

#ifndef __STM32H743I_EVAL_AUDIO_H
#define __STM32H743I_EVAL_AUDIO_H

#ifdef __cplusplus
 extern "C" {
#endif


#include "./SYSTEM/sys/sys.h"
#include "audio.h"
#include <stdlib.h>


uint8_t BSP_AUDIO_OUT_Init(uint16_t OutputDevice, uint8_t Volume, uint32_t AudioFreq);
uint8_t BSP_AUDIO_OUT_Play(uint16_t *pBuffer, uint32_t Size);
void    BSP_AUDIO_OUT_ChangeBuffer(uint16_t *pData, uint16_t Size);
uint8_t BSP_AUDIO_OUT_Stop(uint32_t Option);
uint8_t BSP_AUDIO_OUT_SetVolume(uint8_t Volume);
uint8_t BSP_AUDIO_OUT_SetMute(uint32_t Cmd);
void    BSP_AUDIO_OUT_TransferComplete_CallBack(void);


#ifdef __cplusplus
}
#endif

#endif


