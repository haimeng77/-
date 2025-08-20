#include "stm32h743i_eval_audio.h"
#include "./SYSTEM/usart/usart.h"
#include "./SYSTEM/delay/delay.h"
#include "./BSP/SAI/sai.h"
#include "./BSP/ES8388/es8388.h"
#include "./MALLOC/malloc.h"
#include "usbd_audio.h"
//////////////////////////////////////////////////////////////////////////////////	 
//本程序只供学习使用，未经作者许可，不得用于其它任何用途
//ALIENTEK STM32H7开发板
//USB声卡底层接口函数 驱动代码	   
//正点原子@ALIENTEK
//技术论坛:www.openedv.com
//创建日期:2018/8/31
//版本：V1.0
//版权所有，盗版必究。
//Copyright(C) 广州市星翼电子科技有限公司 2009-2019
//All rights reserved									  
//*******************************************************************************
//修改信息
//无
////////////////////////////////////////////////////////////////////////////////// 	
uint8_t g_volume=0;									//当前音量   
extern uint8_t usbd_audio_dataout_en;				//USB数据输出状态,在usbd_audio.c里面定义


//音频数据SAI DMA传输回调函数
void audio_sai_dma_callback(void) 
{      
	BSP_AUDIO_OUT_TransferComplete_CallBack();	//执行回调函数,更新数据
}   

//配置音频接口
//OutputDevice:输出设备选择,未用到.
//Volume:音量大小,0~100
//AudioFreq:音频采样率  
uint8_t BSP_AUDIO_OUT_Init(uint16_t OutputDevice, uint8_t Volume, uint32_t AudioFreq)
{ 
     es8388_sai_cfg(0, 3);                           /* 飞利浦标准,16位数据长度 */
    saia_init(SAI_MODEMASTER_TX, SAI_CLOCKSTROBING_FALLINGEDGE, SAI_DATASIZE_16); /* 飞利浦标准,主机发送,时钟低电平有效,16位扩展帧长度 */
    saia_samplerate_set(AudioFreq);                 /* 设置采样率 */
    BSP_AUDIO_OUT_SetVolume(Volume);                /* 设置音量 */
    sai_tx_callback = audio_sai_dma_callback;       /* 回调函数指 bsp_audio_sai_dma_callback */
    
    printf("EVAL_AUDIO_Init:%d,%d\r\n", Volume, AudioFreq);
    
    return 0;
}

//开始播放音频数据
//pBuffer:音频数据流首地址指针
//Size:数据流大小(单位:字节)
uint8_t BSP_AUDIO_OUT_Play(uint16_t* pBuffer, uint32_t Size)
{	
    saia_tx_dma_init((uint8_t *) pBuffer, 0, Size, 1);   /* 配置SAI TX DMA */
    DMA2_Stream3->CR &= ~(1 << 8);                      /* 单次模式 */
    DMA2_Stream3->CR &= ~(1 << 18);                     /* 改成单缓冲模式 */
    sai_play_start();                                    /* 开启DMA */
    
    printf("EVAL_AUDIO_Play:%x,%d\r\n", (int)pBuffer, Size);
	return 0;
}

//切换音频数据流buffer,每次DMA中断完成会调用该函数
//Addr:音频数据流缓存首地址
//Size:音频数据流大小(单位:harf word,也就是2个字节)
void BSP_AUDIO_OUT_ChangeBuffer(uint16_t *pData, uint16_t Size)
{ 
    if (usbd_audio_dataout_en == 1)       /* USB有音频数据输出 */
    {
        usbd_audio_dataout_en = 0;        /* 清除标记位 */
    }
    else if (usbd_audio_dataout_en == 0)  /* 没有收到任何数据,全部填充0 */
    {
        my_mem_set(pData, 0, AUDIO_TOTAL_BUF_SIZE);
        usbd_audio_dataout_en = 2;        /* 标记已经清零了buffer,停止数据输出 */
    }

    if (Size != AUDIO_TOTAL_BUF_SIZE)
    {
        // printf("chgbuf:%x,%d\r\n", pData, Size);  /* 想观察数据,可以printf,但是会有轻微的影响音质 */
    }

    DMA2_Stream3->CR &= ~(1 << 0);          /* 关闭DMA传输 */

    while (DMA2_Stream3->CR & 0X1);         /* 确保DMA可以被设置 */

    DMA2_Stream3->NDTR = Size;              /* 设置传输长度 */
    DMA2_Stream3->CR |= 1 << 0;             /* 开启DMA传输 */
}


/**
 * @brief  停止播放
 * @retval            操作结果
 *   @arg             0    , 成功
 *   @arg             其他 , 错误代码
 */
uint8_t BSP_AUDIO_OUT_Pause(void)
{
    printf("EVAL_AUDIO_Stop\r\n");
    return 0;
}

/**
 * @brief  停止播放
 * @param  Option   : 未用到
 * @retval            操作结果
 *   @arg             0    , 成功
 *   @arg             其他 , 错误代码
 */
uint8_t BSP_AUDIO_OUT_Stop(uint32_t Option)
{
    printf("EVAL_AUDIO_Stop\r\n");
    return 0;
}

/**
 * @brief  音量设置
 * @param  Volume: 音量大小, 0~100
 * @retval      操作结果
 *   @arg       0    , 成功
 *   @arg       其他 , 错误代码
 */
uint8_t BSP_AUDIO_OUT_SetVolume(uint8_t Volume)
{
    g_volume = Volume;
    es8388_hpvol_set(g_volume * 0.3);
    es8388_spkvol_set(g_volume * 0.3);
    return 0;
}


/**
 * @brief  静音设置
 * @param  Cmd      : 0,不静音; 1,静音
 * @retval            操作结果
 *   @arg             0    , 成功
 *   @arg             其他 , 错误代码
 */
uint8_t BSP_AUDIO_OUT_SetMute(uint32_t Cmd)
{
    if (Cmd)
    {
        es8388_output_cfg(0, 0);    /* 关闭输出 ,就是静音 */
    }
    else
    {
        es8388_output_cfg(1, 1);    /* 打开输出 ,取消静音 */
    }
    
    printf("BSP_AUDIO_OUT_SetMute:%d\r\n", Cmd);
    return 0;
}
