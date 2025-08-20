/**
 ****************************************************************************************************
 * @file        stm32h743i_eval_audio.c
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

#include "stm32h743i_eval_audio.h"
#include "./SYSTEM/usart/usart.h"
#include "./SYSTEM/delay/delay.h"
#include "./BSP/SAI/sai.h"
#include "./BSP/ES8388/es8388.h"
#include "./MALLOC/malloc.h"
#include "usbd_audio.h"

uint8_t g_volume = 0;                               /* ��ǰ���� */
extern uint8_t usbd_audio_dataout_en;               /* USB�������״̬,��usbd_audio.c���涨�� */


/**
 * @brief       ����ʱ,SAI DMA����ص�����
 * @param       ��
 * @retval      ��
 */
void audio_sai_dma_callback(void) 
{      
    BSP_AUDIO_OUT_TransferComplete_CallBack();  /* ִ�лص�����,�������� */
}   

/**
 * @brief  ������Ƶ�ӿ�
 * @param  OutputDevice :����豸ѡ��,δ�õ�.
 * @param  Volume       :������С, 0~100
 * @param  AudioFreq    :��Ƶ������
 * @retval               �������
 *   @arg                0    , �ɹ�
 *   @arg                ���� , �������
 */
uint8_t BSP_AUDIO_OUT_Init(uint16_t OutputDevice, uint8_t Volume, uint32_t AudioFreq)
{ 
    es8388_sai_cfg(0, 3);                           /* �����ֱ�׼,16λ���ݳ��� */
    sai1_saia_init(SAI_MODEMASTER_TX, SAI_CLOCKSTROBING_RISINGEDGE, SAI_DATASIZE_16); /* �����ֱ�׼,��������,ʱ�Ӹߵ�ƽ��Ч,16λ��չ֡���� */
    sai1_samplerate_set(AudioFreq);                 /* ���ò����� */
    BSP_AUDIO_OUT_SetVolume(Volume);                /* �������� */
    sai1_tx_callback = audio_sai_dma_callback;      /* �ص�����ָ bsp_audio_sai_dma_callback */
    
    printf("EVAL_AUDIO_Init:%d,%d\r\n", Volume, AudioFreq);
    
    return 0;
}

/**
 * @brief  ��ʼ������Ƶ����
 * @param  pBuffer  : ��Ƶ�������׵�ַָ��
 * @param  Size     : ��������С(��λ:�ֽ�)
 * @retval            �������
 *   @arg             0    , �ɹ�
 *   @arg             ���� , �������
 */
uint8_t BSP_AUDIO_OUT_Play(uint16_t* pBuffer, uint32_t Size)
{
    sai1_tx_dma_init((uint8_t *) pBuffer, 0, Size / 2, 1);   /* ����SAI TX DMA */
    SAI1_TX_DMASx->CR &= ~(1 << 8);                          /* ����ģʽ */
    SAI1_TX_DMASx->CR &= ~(1 << 18);                         /* �ĳɵ�����ģʽ */
    sai1_play_start();                                       /* ����DMA */
    
    printf("EVAL_AUDIO_Play:%x,%d\r\n", (int)pBuffer, Size);
    return 0;
}

/**
 * @brief  �л���Ƶ������buffer,ÿ��DMA�ж���ɻ���øú���
 * @param  pData    : ��Ƶ�����������׵�ַ
 * @param  Size     : ��Ƶ��������С(��λ:harf word,Ҳ����2���ֽ�)
 * @retval          : ��
  */
void BSP_AUDIO_OUT_ChangeBuffer(uint16_t *pData, uint16_t Size)
{ 
    if (usbd_audio_dataout_en == 1)       /* USB����Ƶ������� */
    {
        usbd_audio_dataout_en = 0;        /* ������λ */
    }
    else if (usbd_audio_dataout_en == 0)  /* û���յ��κ�����,ȫ�����0 */
    {
        my_mem_set(pData, 0, AUDIO_TOTAL_BUF_SIZE);
        usbd_audio_dataout_en = 2;        /* ����Ѿ�������buffer,ֹͣ������� */
    }

    if (Size != AUDIO_TOTAL_BUF_SIZE)
    {
        // printf("chgbuf:%x,%d\r\n", pData, Size);  /* ��۲�����,����printf,���ǻ�����΢��Ӱ������ */
    }

    SAI1_TX_DMASx->CR &= ~(1 << 0);      /* �ر�DMA���� */

    while (SAI1_TX_DMASx->CR & 0X1);     /* ȷ��DMA���Ա����� */

    SAI1_TX_DMASx->NDTR = Size / 2;      /* ���ô��䳤�� */
    SAI1_TX_DMASx->CR |= 1 << 0;         /* ����DMA���� */
}


/**
 * @brief  ֹͣ����
 * @param             ��
 * @retval            �������
 *   @arg             0    , �ɹ�
 *   @arg             ���� , �������
 */
uint8_t BSP_AUDIO_OUT_Pause(void)
{
    printf("EVAL_AUDIO_Stop\r\n");
    return 0;
}

/**
 * @brief  ֹͣ����
 * @param  Option   : δ�õ�
 * @retval            �������
 *   @arg             0    , �ɹ�
 *   @arg             ���� , �������
 */
uint8_t BSP_AUDIO_OUT_Stop(uint32_t Option)
{
    printf("EVAL_AUDIO_Stop\r\n");
    return 0;
}

/**
 * @brief  ��������
 * @param  Volume: ������С, 0~100
 * @retval      �������
 *   @arg       0    , �ɹ�
 *   @arg       ���� , �������
 */
uint8_t BSP_AUDIO_OUT_SetVolume(uint8_t Volume)
{
    g_volume = Volume;
    es8388_hpvol_set(g_volume * 0.3);
    es8388_spkvol_set(g_volume * 0.3);
    return 0;
}


/**
 * @brief  ��������
 * @param  Cmd      : 0,������; 1,����
 * @retval            �������
 *   @arg             0    , �ɹ�
 *   @arg             ���� , �������
 */
uint8_t BSP_AUDIO_OUT_SetMute(uint32_t Cmd)
{
    if (Cmd)
    {
        es8388_output_cfg(0, 0);    /* �ر���� ,���Ǿ��� */
    }
    else
    {
        es8388_output_cfg(1, 1);    /* ����� ,ȡ������ */
    }
    
    printf("BSP_AUDIO_OUT_SetMute:%d\r\n", Cmd);
    return 0;
}
