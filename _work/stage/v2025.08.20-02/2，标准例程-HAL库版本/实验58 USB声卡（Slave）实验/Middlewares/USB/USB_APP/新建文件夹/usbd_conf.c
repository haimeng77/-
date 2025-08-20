/**
 ****************************************************************************************************
 * @file        usbd_conf.c
 * @author      ����ԭ���Ŷ�(ALIENTEK)
 * @version     V1.0
 * @date        2022-04-20
 * @brief       usbd_conf ��������
 * @license     Copyright (c) 2020-2032, ������������ӿƼ����޹�˾
 ****************************************************************************************************
 * @attention
 *
 * ʵ��ƽ̨:����ԭ�� ������ F429������
 * ������Ƶ:www.yuanzige.com
 * ������̳:www.openedv.com
 * ��˾��ַ:www.alientek.com
 * �����ַ:openedv.taobao.com
 *
 * �޸�˵��
 * V1.0 20220420
 * ��һ�η���
 *
 ****************************************************************************************************
 */

#include "usbd_conf.h"
#include "usbd_core.h"
#include "usbd_def.h"
#include "./BSP/PCF8574/pcf8574.h"
#include "./SYSTEM/sys/sys.h"
#include "./SYSTEM/usart/usart.h"


/* PCD_USB_OTG_FS ����*/
PCD_HandleTypeDef g_pcd_usb_otg_fs;

/* USB����״̬
 * 0,û������;
 * 1,�Ѿ�����;
 */
volatile uint8_t g_device_state = 0;                                    /* Ĭ��û������ */

USBD_StatusTypeDef USBD_Get_USB_Status(HAL_StatusTypeDef hal_status);   /* ����USB��״̬ */

/**
 * @brief       ��ʼ��PCD MSP
 * @param       hpcd:PCD�ṹ��ָ��
 * @retval      ��
 */
void HAL_PCD_MspInit(PCD_HandleTypeDef *hpcd)
{
    GPIO_InitTypeDef gpio_init_struct = {0};

    if (hpcd->Instance == USB_OTG_FS)
    {
        __HAL_RCC_GPIOA_CLK_ENABLE();                        /* ʹ��GPIOAʱ�� */
        gpio_init_struct.Pin = GPIO_PIN_11 | GPIO_PIN_12;
        gpio_init_struct.Mode = GPIO_MODE_AF_PP;             /* ���� */
        gpio_init_struct.Pull = GPIO_NOPULL;                 /* ���� */
        gpio_init_struct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;  /* ���� */
        gpio_init_struct.Alternate = GPIO_AF10_OTG1_FS;      /* ����ΪOTG1_FS */
        HAL_GPIO_Init(GPIOA, &gpio_init_struct);             /* ��ʼ��PA11��PA12���� */
        pcf8574_write_bit(USB_PWR_IO, 1);                    /* ����USB HOST��Դ����C */
  
        __HAL_RCC_USB_OTG_FS_CLK_ENABLE();                   /* ʹ��OTG FSʱ�� */

        HAL_NVIC_SetPriority(OTG_FS_IRQn, 0, 3);             /* ���ȼ�����Ϊ��ռ0,�����ȼ�3 */
        HAL_NVIC_EnableIRQ(OTG_FS_IRQn);                     /* ʹ��OTG FS�ж� */
    }
    else if (hpcd->Instance == USB_OTG_HS)
    {
        /* USB OTG HS������û�õ�,�ʲ������� */
    }
}

/**
 * @brief       USB OTG �жϷ�����
 *   @note      ��������USB�ж�
 * @param       ��
 * @retval      ��
 */
void OTG_FS_IRQHandler(void)
{
    HAL_PCD_IRQHandler(&g_pcd_usb_otg_fs);
}

/**
 * @brief       USBD ���ý׶λص�����
 * @param       hpcd    : PCD�ṹ��ָ��
 * @retval      ��
 */
void HAL_PCD_SetupStageCallback(PCD_HandleTypeDef *hpcd)
{
    USBD_LL_SetupStage((USBD_HandleTypeDef*)hpcd->pData, (uint8_t *) hpcd->Setup);
}

/**
 * @brief       USBD OUT �׶λص�����
 * @param       hpcd    : PCD�ṹ��ָ��
 * @param       epnum   : �˵��
 * @retval      ��
 */
void HAL_PCD_DataOutStageCallback(PCD_HandleTypeDef *hpcd, uint8_t epnum)
{
    USBD_LL_DataOutStage((USBD_HandleTypeDef*)hpcd->pData, epnum, hpcd->OUT_ep[epnum].xfer_buff);
}

/**
 * @brief       USBD IN �׶λص�����
 * @param       hpcd    : PCD�ṹ��ָ��
 * @param       epnum   : �˵��
 * @retval      ��
 */
void HAL_PCD_DataInStageCallback(PCD_HandleTypeDef *hpcd, uint8_t epnum)
{
    USBD_LL_DataInStage((USBD_HandleTypeDef*)hpcd->pData, epnum, hpcd->IN_ep[epnum].xfer_buff);
}

/**
 * @brief       USBD SOF �ص�����
 * @param       hpcd    : PCD�ṹ��ָ��
 * @retval      ��
 */
void HAL_PCD_SOFCallback(PCD_HandleTypeDef *hpcd)
{
    USBD_LL_SOF((USBD_HandleTypeDef*)hpcd->pData);
}

/**
 * @brief       USBD ��λ�ص�����
 * @param       hpcd    : PCD�ṹ��ָ��
 * @retval      ��
 */
void HAL_PCD_ResetCallback(PCD_HandleTypeDef *hpcd)
{
    USBD_SpeedTypeDef speed = USBD_SPEED_FULL;

    /* Set USB Current Speed */
    switch (hpcd->Init.speed)
    {
        case PCD_SPEED_HIGH:
            printf("USB Device Library  [HS]\r\n");
            speed = USBD_SPEED_HIGH;
            break;

        case PCD_SPEED_FULL:
            printf("USB Device Library  [FS]\r\n");
            speed = USBD_SPEED_FULL;
            break;

        default:
            printf("USB Device Library  [FS?]\r\n");
            speed = USBD_SPEED_FULL;
            break;
    }
    
    USBD_LL_SetSpeed((USBD_HandleTypeDef*)hpcd->pData, speed);
    USBD_LL_Reset((USBD_HandleTypeDef*)hpcd->pData); /* ��λ�豸 */
}

/**
 * @brief       USBD ����ص�����
 * @param       hpcd    : PCD�ṹ��ָ��
 * @retval      ��
 */
void HAL_PCD_SuspendCallback(PCD_HandleTypeDef *hpcd)
{
    g_device_state = 0;
    printf("Device In suspend mode.\r\n");
    USBD_LL_Suspend((USBD_HandleTypeDef*)hpcd->pData);
    __HAL_PCD_GATE_PHYCLOCK(hpcd);

    if (hpcd->Init.low_power_enable)
    {
        /* Set SLEEPDEEP bit and SleepOnExit of Cortex System Control Register. */
        SCB->SCR |= (uint32_t)((uint32_t)(SCB_SCR_SLEEPDEEP_Msk | SCB_SCR_SLEEPONEXIT_Msk));
    }
}

/**
 * @brief       USBD �ָ��ص�����
 * @param       hpcd    : PCD�ṹ��ָ��
 * @retval      ��
 */
void HAL_PCD_ResumeCallback(PCD_HandleTypeDef *hpcd)
{
    printf("Device Resumed\r\n");
    USBD_LL_Resume((USBD_HandleTypeDef*)hpcd->pData);
}

/**
 * @brief       USBD ISO OUT ������ɻص�����
 * @param       hpcd    : PCD�ṹ��ָ��
 * @param       epnum   : �˵��
 * @retval      ��
 */
void HAL_PCD_ISOOUTIncompleteCallback(PCD_HandleTypeDef *hpcd, uint8_t epnum)
{
    USBD_LL_IsoOUTIncomplete((USBD_HandleTypeDef*)hpcd->pData, epnum);
}

/**
 * @brief       USBD ISO IN ������ɻص�����
 * @param       hpcd    : PCD�ṹ��ָ��
 * @param       epnum   : �˵��
 * @retval      ��
 */
void HAL_PCD_ISOINIncompleteCallback(PCD_HandleTypeDef *hpcd, uint8_t epnum)
{
    USBD_LL_IsoINIncomplete((USBD_HandleTypeDef*)hpcd->pData, epnum);
}

/**
 * @brief       USBD ���ӳɹ��ص�����
 * @param       hpcd    : PCD�ṹ��ָ��
 * @retval      ��
 */
void HAL_PCD_ConnectCallback(PCD_HandleTypeDef *hpcd)
{
    g_device_state = 1;
    USBD_LL_DevConnected((USBD_HandleTypeDef*)hpcd->pData);
}

/**
 * @brief       USBD �Ͽ����ӻص�����
 * @param       hpcd    : PCD�ṹ��ָ��
 * @retval      ��
 */
void HAL_PCD_DisconnectCallback(PCD_HandleTypeDef *hpcd)
{
    g_device_state = 0;
    printf("USB Device Disconnected.\r\n");
    USBD_LL_DevDisconnected((USBD_HandleTypeDef*)hpcd->pData);
}

/******************************************************************************************/
/* ������: USBD LL �����ӿں���(PCD->USB Device Library) */

/**
 * @brief       USBD �ײ��ʼ������
 * @param       pdev    : USBD���ָ��
 * @retval      USB״̬
 *   @arg       USBD_OK(0)   , ����;
 *   @arg       USBD_BUSY(1) , æ;
 *   @arg       USBD_FAIL(2) , ʧ��;
 */
USBD_StatusTypeDef USBD_LL_Init(USBD_HandleTypeDef *pdev)
{
        g_pcd_usb_otg_fs.pData = pdev;                          /* g_pcd_usb_otg_fs��pDataָ��pdev */
        pdev->pData = &g_pcd_usb_otg_fs;                        /* pdev��pDataָ��g_pcd_usb_otg_fs */

        g_pcd_usb_otg_fs.Instance = USB_OTG_FS;                 /* ʹ��USB OTG FS */
        g_pcd_usb_otg_fs.Init.dev_endpoints = 4;                /* �˵���Ϊ4 */
        g_pcd_usb_otg_fs.Init.speed = PCD_SPEED_FULL;           /* USBȫ��(12Mbps) */
        g_pcd_usb_otg_fs.Init.dma_enable = DISABLE;             /* ��ʹ��DMA */
        g_pcd_usb_otg_fs.Init.phy_itface = PCD_PHY_EMBEDDED;    /* ʹ���ڲ�PHY */
        g_pcd_usb_otg_fs.Init.Sof_enable = DISABLE;             /* ��ʹ��SOF�ж� */
        g_pcd_usb_otg_fs.Init.low_power_enable = DISABLE;       /* ��ʹ�ܵ͹���ģʽ */
        g_pcd_usb_otg_fs.Init.lpm_enable = DISABLE;             /* ��ʹ�����ӵ�Դ���� */
        g_pcd_usb_otg_fs.Init.vbus_sensing_enable = DISABLE;    /* ��ʹ��VBUS��� */
        g_pcd_usb_otg_fs.Init.use_dedicated_ep1 = DISABLE;      /* ��ֹEP1 dedicated�ж� */

        HAL_PCD_Init(&g_pcd_usb_otg_fs);

        HAL_PCDEx_SetRxFiFo(&g_pcd_usb_otg_fs, 0x80);           /* ���ý���FIFO��СΪ0X80(128�ֽ�) */
        HAL_PCDEx_SetTxFiFo(&g_pcd_usb_otg_fs, 0, 0x40);        /* ���÷���FIFO 0�Ĵ�СΪ0X40(64�ֽ�) */
        HAL_PCDEx_SetTxFiFo(&g_pcd_usb_otg_fs, 1, 0x80);        /* ���÷���FIFO 1�Ĵ�СΪ0X80(128�ֽ�) */

    return USBD_OK;
}

/**
 * @brief       USBD �ײ�ȡ����ʼ��(�ظ�Ĭ�ϸ�λ״̬)����
 * @param       pdev    : USBD���ָ��
 * @retval      USB״̬
 *   @arg       USBD_OK(0)   , ����;
 *   @arg       USBD_BUSY(1) , æ;
 *   @arg       USBD_FAIL(2) , ʧ��;
 */
USBD_StatusTypeDef USBD_LL_DeInit(USBD_HandleTypeDef *pdev)
{
    HAL_StatusTypeDef hal_status = HAL_OK;
    USBD_StatusTypeDef usb_status = USBD_OK;

    hal_status = HAL_PCD_DeInit(pdev->pData);

    usb_status =  USBD_Get_USB_Status(hal_status);

    return usb_status;
}

/**
 * @brief       USBD �ײ�������ʼ����
 * @param       pdev    : USBD���ָ��
 * @retval      USB״̬
 *   @arg       USBD_OK(0)   , ����;
 *   @arg       USBD_BUSY(1) , æ;
 *   @arg       USBD_FAIL(2) , ʧ��;
 */
USBD_StatusTypeDef USBD_LL_Start(USBD_HandleTypeDef *pdev)
{
    HAL_StatusTypeDef hal_status = HAL_OK;
    USBD_StatusTypeDef usb_status = USBD_OK;

    hal_status = HAL_PCD_Start(pdev->pData);

    usb_status =  USBD_Get_USB_Status(hal_status);

    return usb_status;
}

/**
 * @brief       USBD �ײ�����ֹͣ����
 * @param       pdev    : USBD���ָ��
 * @retval      USB״̬
 *   @arg       USBD_OK(0)   , ����;
 *   @arg       USBD_BUSY(1) , æ;
 *   @arg       USBD_FAIL(2) , ʧ��;
 */
USBD_StatusTypeDef USBD_LL_Stop(USBD_HandleTypeDef *pdev)
{
    HAL_StatusTypeDef hal_status = HAL_OK;
    USBD_StatusTypeDef usb_status = USBD_OK;

    hal_status = HAL_PCD_Stop(pdev->pData);

    usb_status =  USBD_Get_USB_Status(hal_status);

    return usb_status;
}

/**
 * @brief       USBD ��ʼ��(��)ĳ���˵�
 * @param       pdev    : USBD���ָ��
 * @param       ep_addr : �˵��
 * @param       ep_type : �˵�����
 * @param       ep_mps  : �˵���������(�ֽ�)
 * @retval      USB״̬
 *   @arg       USBD_OK(0)   , ����;
 *   @arg       USBD_BUSY(1) , æ;
 *   @arg       USBD_FAIL(2) , ʧ��;
 */
USBD_StatusTypeDef USBD_LL_OpenEP(USBD_HandleTypeDef *pdev, uint8_t ep_addr, uint8_t ep_type, uint16_t ep_mps)
{
    HAL_StatusTypeDef hal_status = HAL_OK;
    USBD_StatusTypeDef usb_status = USBD_OK;

    hal_status = HAL_PCD_EP_Open(pdev->pData, ep_addr, ep_mps, ep_type);

    usb_status =  USBD_Get_USB_Status(hal_status);

    return usb_status;
}

/**
 * @brief       USBD ȡ����ʼ��(�ر�)ĳ���˵�
 * @param       pdev    : USBD���ָ��
 * @param       ep_addr : �˵��
 * @retval      USB״̬
 *   @arg       USBD_OK(0)   , ����;
 *   @arg       USBD_BUSY(1) , æ;
 *   @arg       USBD_FAIL(2) , ʧ��;
 */
USBD_StatusTypeDef USBD_LL_CloseEP(USBD_HandleTypeDef *pdev, uint8_t ep_addr)
{
    HAL_StatusTypeDef hal_status = HAL_OK;
    USBD_StatusTypeDef usb_status = USBD_OK;

    hal_status = HAL_PCD_EP_Close(pdev->pData, ep_addr);

    usb_status =  USBD_Get_USB_Status(hal_status);

    return usb_status;
}

/**
 * @brief       USBD ���ĳ���˵������
 * @param       pdev    : USBD���ָ��
 * @param       ep_addr : �˵��
 * @retval      USB״̬
 *   @arg       USBD_OK(0)   , ����;
 *   @arg       USBD_BUSY(1) , æ;
 *   @arg       USBD_FAIL(2) , ʧ��;
 */
USBD_StatusTypeDef USBD_LL_FlushEP(USBD_HandleTypeDef *pdev, uint8_t ep_addr)
{
    HAL_StatusTypeDef hal_status = HAL_OK;
    USBD_StatusTypeDef usb_status = USBD_OK;

    hal_status = HAL_PCD_EP_Flush(pdev->pData, ep_addr);

    usb_status =  USBD_Get_USB_Status(hal_status);

    return usb_status;
}

/**
 * @brief       USBD ��ĳ���˵�����һ����ͣ״̬
 * @param       pdev    : USBD���ָ��
 * @param       ep_addr : �˵��
 * @retval      USB״̬
 *   @arg       USBD_OK(0)   , ����;
 *   @arg       USBD_BUSY(1) , æ;
 *   @arg       USBD_FAIL(2) , ʧ��;
 */
USBD_StatusTypeDef USBD_LL_StallEP(USBD_HandleTypeDef *pdev, uint8_t ep_addr)
{
    HAL_StatusTypeDef hal_status = HAL_OK;
    USBD_StatusTypeDef usb_status = USBD_OK;

    hal_status = HAL_PCD_EP_SetStall(pdev->pData, ep_addr);

    usb_status =  USBD_Get_USB_Status(hal_status);

    return usb_status;
}

/**
 * @brief       USBD ȡ��ĳ���˵����ͣ״̬
 * @param       pdev    : USBD���ָ��
 * @param       ep_addr : �˵��
 * @retval      USB״̬
 *   @arg       USBD_OK(0)   , ����;
 *   @arg       USBD_BUSY(1) , æ;
 *   @arg       USBD_FAIL(2) , ʧ��;
 */
USBD_StatusTypeDef USBD_LL_ClearStallEP(USBD_HandleTypeDef *pdev, uint8_t ep_addr)
{
    HAL_StatusTypeDef hal_status = HAL_OK;
    USBD_StatusTypeDef usb_status = USBD_OK;

    hal_status = HAL_PCD_EP_ClrStall(pdev->pData, ep_addr);

    usb_status =  USBD_Get_USB_Status(hal_status);

    return usb_status;
}

/**
 * @brief       USBD �����Ƿ�����ͣ״̬
 * @param       pdev    : USBD���ָ��
 * @param       ep_addr : �˵��
 * @retval      0, ����ͣ; 1, ��ͣ;
 */
uint8_t USBD_LL_IsStallEP(USBD_HandleTypeDef *pdev, uint8_t ep_addr)
{
    PCD_HandleTypeDef *hpcd = (PCD_HandleTypeDef *) pdev->pData;

    if ((ep_addr & 0x80) == 0x80)
    {
        return hpcd->IN_ep[ep_addr & 0x7F].is_stall;
    }
    else
    {
        return hpcd->OUT_ep[ep_addr & 0x7F].is_stall;
    }
}

/**
 * @brief       USBD Ϊ�豸ָ���µ�USB��ַ
 * @param       pdev    : USBD���ָ��
 * @param       dev_addr: �µ��豸��ַ,USB1_OTG_HS/USB2_OTG_HS
 * @retval      USB״̬
 *   @arg       USBD_OK(0)   , ����;
 *   @arg       USBD_BUSY(1) , æ;
 *   @arg       USBD_FAIL(2) , ʧ��;
 */
USBD_StatusTypeDef USBD_LL_SetUSBAddress(USBD_HandleTypeDef *pdev, uint8_t dev_addr)
{
    g_device_state = 1;
    HAL_StatusTypeDef hal_status = HAL_OK;
    USBD_StatusTypeDef usb_status = USBD_OK;

    hal_status = HAL_PCD_SetAddress(pdev->pData, dev_addr);

    usb_status =  USBD_Get_USB_Status(hal_status);

    return usb_status;
}

/**
 * @brief       USBD ͨ���˵㷢������
 * @param       pdev    : USBD���ָ��
 * @param       ep_addr : �˵��
 * @param       pbuf    : ���ݻ������׵�ַ
 * @param       size    : Ҫ���͵����ݴ�С
 * @retval      USB״̬
 *   @arg       USBD_OK(0)   , ����;
 *   @arg       USBD_BUSY(1) , æ;
 *   @arg       USBD_FAIL(2) , ʧ��;
 */
USBD_StatusTypeDef USBD_LL_Transmit(USBD_HandleTypeDef *pdev, uint8_t ep_addr, uint8_t *pbuf, uint32_t size)
{
    HAL_StatusTypeDef hal_status = HAL_OK;
    USBD_StatusTypeDef usb_status = USBD_OK;

    hal_status = HAL_PCD_EP_Transmit(pdev->pData, ep_addr, pbuf, size);

    usb_status =  USBD_Get_USB_Status(hal_status);

    return usb_status;
}

/**
 * @brief       USBD ׼��һ���˵��������
 * @param       pdev    : USBD���ָ��
 * @param       ep_addr : �˵��
 * @param       pbuf    : ���ݻ������׵�ַ
 * @param       size    : Ҫ���յ����ݴ�С
 * @retval      USB״̬
 *   @arg       USBD_OK(0)   , ����;
 *   @arg       USBD_BUSY(1) , æ;
 *   @arg       USBD_FAIL(2) , ʧ��;
 */
USBD_StatusTypeDef USBD_LL_PrepareReceive(USBD_HandleTypeDef *pdev, uint8_t ep_addr, uint8_t *pbuf, uint32_t size)
{
    HAL_StatusTypeDef hal_status = HAL_OK;
    USBD_StatusTypeDef usb_status = USBD_OK;

    hal_status = HAL_PCD_EP_Receive(pdev->pData, ep_addr, pbuf, size);

    usb_status =  USBD_Get_USB_Status(hal_status);

    return usb_status;
}

/**
 * @brief       USBD ��ȡ���һ��������Ĵ�С
 * @param       pdev    : USBD���ָ��
 * @param       ep_addr : �˵��
 * @retval      ����С
 */
uint32_t USBD_LL_GetRxDataSize(USBD_HandleTypeDef *pdev, uint8_t ep_addr)
{
    return HAL_PCD_EP_GetRxCount((PCD_HandleTypeDef *) pdev->pData, ep_addr);
}

/**
 * @brief       USBD ��ʱ����(��msΪ��λ)
 * @param       Delay   : ��ʱ��ms��
 * @retval      ��
 */
void USBD_LL_Delay(uint32_t Delay)
{
    HAL_Delay(Delay);
}


/**
 * @brief       ����USB��״̬(HAL״̬)
 * @param       hal_status : ��ǰ��״̬
 * @retval      ��
 */
USBD_StatusTypeDef USBD_Get_USB_Status(HAL_StatusTypeDef hal_status)
{
    USBD_StatusTypeDef usb_status = USBD_OK;

    switch (hal_status)
    {
        case HAL_OK :
            usb_status = USBD_OK;
            break;

        case HAL_ERROR :
            usb_status = USBD_FAIL;
            break;

        case HAL_BUSY :
            usb_status = USBD_BUSY;
            break;

        case HAL_TIMEOUT :
            usb_status = USBD_FAIL;
            break;

        default :
            usb_status = USBD_FAIL;
            break;
    }

    return usb_status;
}
