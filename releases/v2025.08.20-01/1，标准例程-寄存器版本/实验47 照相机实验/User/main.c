/**
 ****************************************************************************************************
 * @file        main.c
 * @author      ����ԭ���Ŷ�(ALIENTEK)
 * @version     V1.0
 * @date        2023-03-25
 * @brief       ����� ʵ��
 * @license     Copyright (c) 2022-2032, ������������ӿƼ����޹�˾
 ****************************************************************************************************
 * @attention
 *
 * ʵ��ƽ̨:����ԭ�� ������ H743������
 * ������Ƶ:www.yuanzige.com
 * ������̳:www.openedv.com
 * ��˾��ַ:www.alientek.com
 * �����ַ:openedv.taobao.com
 *
 ****************************************************************************************************
 */

#include "./SYSTEM/sys/sys.h"
#include "./SYSTEM/usart/usart.h"
#include "./SYSTEM/delay/delay.h"
#include "./BSP/LED/led.h"
#include "./BSP/KEY/key.h"
#include "./BSP/MPU/mpu.h"
#include "./BSP/LCD/lcd.h"
#include "./BSP/SDRAM/sdram.h"
#include "./USMART/usmart.h"
#include "./MALLOC/malloc.h"
#include "./BSP/NAND/ftl.h"
#include "./FATFS/exfuns/exfuns.h"
#include "./BSP/NORFLASH/norflash.h"
#include "./BSP/SDMMC/sdmmc_sdcard.h" 
#include "./TEXT/text.h"
#include "./PICTURE/piclib.h"
#include "./BSP/OV5640/sccb.h"
#include "./BSP/OV5640/ov5640.h"
#include "./BSP/DCMI/dcmi.h"
#include "./BSP/PCF8574/pcf8574.h"
#include "./BSP/LCD/ltdc.h"



__IO uint8_t g_bmp_request = 0;                /* bmp��������:0,��bmp��������;1,��bmp��������,��Ҫ��֡�ж�����,�ر�DCMI�ӿ�. */
uint8_t g_ov_mode = 0;                         /* bit0: 0, RGB565ģʽ;  1, JPEGģʽ */
uint16_t g_curline = 0;                        /* ����ͷ�������,��ǰ�б�� */
uint16_t g_yoffset = 0;                        /* y�����ƫ���� */

#define jpeg_buf_size       4 * 1024 * 1024    /* ����JPEG���ݻ���jpeg_buf�Ĵ�С(�ֽ�) */
#define jpeg_line_size      4 * 1024           /* ����DMA��������ʱ,һ�����ݵ����ֵ */

uint32_t *p_dcmi_line_buf[2];                  /* RGB��ʱ,����ͷ����һ��һ�ж�ȡ,�����л��� */
uint32_t *p_jpeg_data_buf;                     /* JPEG���ݻ���bufָ�� */


/* buf�е�JPEG��Ч���ݳ��� */
volatile uint32_t g_jpeg_data_len = 0;       


/**
 * JPEG���ݲɼ���ɱ�־
 * 0,����û�вɼ���;
 * 1,���ݲɼ�����,���ǻ�û����;
 * 2,�����Ѿ����������,���Կ�ʼ��һ֡����
 */
volatile uint8_t g_jpeg_data_ok = 0;

/**
 * @brief       ����JPEG����
 * @ntoe        ��DCMI_IRQHandler�жϷ��������汻����
 *              ���ɼ���һ֡JPEG���ݺ�,���ô˺���,�л�JPEG BUF.��ʼ��һ֡�ɼ�.
 *
 * @param       ��
 * @retval      ��
 */
void jpeg_data_process(void)
{
    uint16_t i, rlen;
    uint32_t *pbuf;
    g_curline = g_yoffset;                                                        /* ������λ */

    if (g_ov_mode & 0X01)                                                         /* ֻ����JPEG��ʽ��,����Ҫ������. */
    {
        if (g_jpeg_data_ok == 0)    /* jpeg���ݻ�δ�ɼ���? */
        {
            DMA1_Stream1->CR &= ~(1 << 0);      /* ֹͣ��ǰ���� */
    
            while (DMA1_Stream1->CR & 0X01);    /* �ȴ�DMA1_Stream1������ */

            rlen = jpeg_line_size - DMA1_Stream1->NDTR; /* �õ�ʣ�����ݳ��� */
            pbuf = p_jpeg_data_buf + g_jpeg_data_len;   /* ƫ�Ƶ���Ч����ĩβ,������� */

            if (DMA1_Stream1->CR & (1 << 19))
            {
                for (i = 0; i < rlen; i++)
                {
                    pbuf[i] = p_dcmi_line_buf[1][i];    /* ��ȡbuf1�����ʣ������ */
                }
            }
            else 
            {
                for (i = 0; i < rlen; i++)
                {
                    pbuf[i] = p_dcmi_line_buf[0][i];    /* ��ȡbuf0�����ʣ������ */
                }
            }

            g_jpeg_data_len += rlen;    /* ����ʣ�೤�� */
            g_jpeg_data_ok = 1;         /* ���JPEG���ݲɼ����,�ȴ������������� */
        }

        if (g_jpeg_data_ok == 2)        /* ��һ�ε�jpeg�����Ѿ��������� */
        {
            DMA1_Stream1->NDTR = jpeg_line_size;    /* ���䳤��Ϊjpeg_buf_size*4�ֽ� */
            DMA1_Stream1->CR |= 1 << 0; /* ���´��� */
            g_jpeg_data_ok = 0;         /* �������δ�ɼ� */
            g_jpeg_data_len = 0;        /* �������¿�ʼ */
        }
    }
    else
    {
        if (g_bmp_request == 1)         /* ��bmp��������,�ر�DCMI */
        {
            dcmi_stop();                /* ֹͣDCMI */
            g_bmp_request = 0;          /* ������������ */
        }
        if (lcdltdc.pwidth == 0)
        {
            lcd_set_cursor(0, 0);
            lcd_write_ram_prepare();    /* ��ʼд��GRAM */
        }
    }
}

/**
 * @brief       JPEG���ݽ��ջص�����
 * @param       ��
 * @retval      ��
 */
void jpeg_dcmi_rx_callback(void)
{
    uint16_t i;
    uint32_t *pbuf;

    pbuf = p_jpeg_data_buf + g_jpeg_data_len;               /* ƫ�Ƶ���Ч����ĩβ */

    if (DMA1_Stream1->CR & (1 << 19))                       /* buf0����,��������buf1 */
    {
        for (i = 0; i < jpeg_line_size; i++)
        {
            pbuf[i] = p_dcmi_line_buf[0][i];                /* ��ȡbuf0��������� */
        }
        
        g_jpeg_data_len += jpeg_line_size;                  /* ƫ�� */
    }
    else                                                    /* buf1����,��������buf0 */
    {
        for (i = 0; i < jpeg_line_size; i++)
        {
            pbuf[i] = p_dcmi_line_buf[1][i];                /* ��ȡbuf1��������� */
        }
        
        g_jpeg_data_len += jpeg_line_size;                  /* ƫ�� */
    }
}
\
/**
 * @brief       RGB�����ݽ��ջص�����
 * @param       ��
 * @retval      ��
 */
void rgblcd_dcmi_rx_callback(void)
{
    uint16_t *pbuf;

    if (DMA1_Stream1->CR & (1 << 19))                       /* DMAʹ��buf1,��ȡbuf0 */
    { 
        pbuf = (uint16_t*)p_dcmi_line_buf[0]; 
    }
    else                                                    /* DMAʹ��buf0,��ȡbuf1 */
    {
        pbuf = (uint16_t*)p_dcmi_line_buf[1]; 
    }     

    ltdc_color_fill(0, g_curline, lcddev.width - 1, g_curline, pbuf);   /* DM2D��� */

    if (g_curline < lcddev.height) g_curline++;
    if (g_bmp_request == 1 && g_curline == (lcddev.height - 1))         /* ��bmp��������,�ر�DCMI */
    {
        dcmi_stop();                                        /* ֹͣDCMI */
        g_bmp_request = 0;                                  /* ������������ */
    }
}

/**
 * @brief       �л�ΪOV5640ģʽ
 * @note        �л�PC8/PC9/PC11ΪDCMI���ù���(AF13)
 * @param       ��
 * @retval      ��
 */
void sw_ov5640_mode(void)
{
    ov5640_write_reg(0X3017, 0XFF);             /* ����OV5650���(����������ʾ) */
    ov5640_write_reg(0X3018, 0XFF); 
    
    sys_gpio_af_set(GPIOC, SYS_GPIO_PIN8, 13);  /* PC8,AF13  DCMI_D2 */
    sys_gpio_af_set(GPIOC, SYS_GPIO_PIN9, 13);  /* PC9,AF13  DCMI_D3 */
    sys_gpio_af_set(GPIOC, SYS_GPIO_PIN11, 13); /* PC11,AF13 DCMI_D4 */
}

/**
 * @brief       �л�ΪSD��ģʽ
 * @note        �л�PC8/PC9/PC11ΪSDMMC���ù���(AF12)
 * @param       ��
 * @retval      ��
 */
void sw_sdcard_mode(void)
{
    ov5640_write_reg(0X3017, 0X00);             /* �ر�OV5650���(����������ʾ) */
    ov5640_write_reg(0X3018, 0X00); 

    sys_gpio_af_set(GPIOC, SYS_GPIO_PIN8, 12);  /* PC8,AF12  SDIO_D0 */
    sys_gpio_af_set(GPIOC, SYS_GPIO_PIN9, 12);  /* PC9,AF12  SDIO_D1 */
    sys_gpio_af_set(GPIOC, SYS_GPIO_PIN11, 12); /* PC11,AF12 SDIO_D3 */
}

/**
 * @brief       �ļ������������⸲�ǣ�
 * @note        bmp��ϳ�: ���� "0:PHOTO/PIC13141.bmp" ���ļ���
 *              jpg��ϳ�: ���� "0:PHOTO/PIC13141.jpg" ���ļ���
 * @param       pname : ��Ч���ļ���
 * @param       mode  : 0, ����.bmp�ļ�;  1, ����.jpg�ļ�;
 * @retval      ��
 */
void camera_new_pathname(uint8_t *pname, uint8_t mode)
{
    uint8_t res;
    uint16_t index = 0;
    FIL *ftemp;

    ftemp = (FIL *)mymalloc(SRAMIN, sizeof(FIL));           /* ����FIL�ֽڵ��ڴ����� */

    while (index < 0XFFFF)
    {
        if (mode == 0)                                      /* ����.bmp�ļ��� */
        {
            sprintf((char *)pname, "0:PHOTO/PIC%05d.bmp", index);
        }
        else                                                /* ����.jpg�ļ��� */
        {
            sprintf((char *)pname, "0:PHOTO/PIC%05d.jpg", index);
        }
        
        res = f_open(ftemp, (const TCHAR *)pname, FA_READ); /* ���Դ�����ļ� */

        if (res == FR_NO_FILE)
        {
            break;                        /* ���ļ���������, ����������Ҫ�� */
        }

        index++;
    }
    myfree(SRAMIN, ftemp);
}
/**
 * @brief       OV5640����jpgͼƬ
 * @param       pname : Ҫ������jpg�ļ���(��·��)
 * @retval      0, �ɹ�; ����,�������;
 */
uint8_t ov5640_jpg_photo(uint8_t *pname)
{
    FIL *f_jpg;
    uint8_t res = 0, headok = 0;
    uint32_t bwr;
    uint32_t i, jpgstart, jpglen;
    uint8_t *pbuf;
    
    uint16_t datasize = 0;          /* ����д�������� */
    uint32_t datalen = 0;           /* ��д�������� */
    uint8_t  *databuf;              /* ���ݻ��棬����ֱ��д�ⲿSRAM���ݵ�SD��������д��������� */

    f_jpg = (FIL *)mymalloc(SRAMIN, sizeof(FIL));   /* ����FIL�ֽڵ��ڴ����� */
    databuf = mymalloc(SRAMIN, 4096);   /* ����4K�ڴ� */

    if (databuf == NULL)
    {
        myfree(SRAMIN, f_jpg);      /* �ͷ�f_jpg�ڴ� */
        return 0XFF;                /* �ڴ�����ʧ�� */
    }

    g_ov_mode = 1;
    g_jpeg_data_ok = 0;
    
    sw_ov5640_mode();                               /* �л�ΪOV5640ģʽ */
    ov5640_jpeg_mode();                             /* JPEGģʽ */
    ov5640_outsize_set(16, 4, 2592, 1944);          /* ��������ߴ�(500W) */

    dcmi_rx_callback = jpeg_dcmi_rx_callback;       /* JPEG�������ݻص����� */
    dcmi_dma_init((uint32_t)p_dcmi_line_buf[0], (uint32_t)p_dcmi_line_buf[1], jpeg_line_size, 2, 1);    /* DCMI DMA���� */
    
    dcmi_start();                                   /* �������� */
    while (g_jpeg_data_ok != 1);                    /* �ȴ���һ֡ͼƬ�ɼ��� */
    g_jpeg_data_ok = 2;                             /* ���Ա�֡ͼƬ,������һ֡�ɼ� */
    while (g_jpeg_data_ok != 1);                    /* �ȴ��ڶ�֡ͼƬ�ɼ���,�ڶ�֡,�ű��浽SD��ȥ */

    dcmi_stop();                                    /* ֹͣDMA���� */
    g_ov_mode = 0;
    sw_sdcard_mode();                               /* �л�ΪSD��ģʽ */
    

    printf("jpeg data size:%d\r\n", g_jpeg_data_len * 4);   /* ���ڴ�ӡJPEG�ļ���С */
    pbuf = (uint8_t *)p_jpeg_data_buf;
    jpglen = 0;                                     /* ����jpg�ļ���СΪ0 */
    headok = 0;                                     /* ���jpgͷ��� */

    for (i = 0; i < g_jpeg_data_len * 4; i++)       /* ����0XFF,0XD8��0XFF,0XD9,��ȡjpg�ļ���С */
    {
        if ((pbuf[i] == 0XFF) && (pbuf[i + 1] == 0XD8))     /* �ҵ�FF D8 */
        {
            jpgstart = i;
            headok = 1;                             /* ����ҵ�jpgͷ(FF D8) */
        }

        if ((pbuf[i] == 0XFF) && (pbuf[i + 1] == 0XD9) && headok)   /* �ҵ�ͷ�Ժ�,����FF D9 */
        {
            jpglen = i - jpgstart + 2;
            break;
        }
    }

    if (jpglen)                     /* ������jpeg���� */
    {
        res = f_open(f_jpg, (const TCHAR *)pname, FA_WRITE | FA_CREATE_NEW);    /* ģʽ0,���߳��Դ�ʧ��,�򴴽����ļ� */

        if (res == 0)
        {
            pbuf += jpgstart;       /* ƫ�Ƶ�0XFF,0XD8�� */
            
            while(datalen < jpglen) /* ѭ��д�룡����ֱ��д�ⲿSRAM���ݵ�SDIO�������������FIFO������� */
            {
                if((jpglen - datalen) > 4096)
                {
                    datasize = 4096;
                }else
                {
                    datasize = jpglen - datalen;    /* �������� */
                }

                my_mem_copy(databuf, pbuf, datasize);
                res = f_write(f_jpg, databuf, datasize, (UINT *)&bwr); /* д������ */
                pbuf += datasize;
                jpglen -= datasize;

                if (res)break;
            }
        }

        f_close(f_jpg);
    }
    else
    {
        res = 0XFD;
    }
    
    g_jpeg_data_len = 0;
    sw_ov5640_mode();           /* �л�ΪOV5640ģʽ */
    ov5640_rgb565_mode();       /* RGB565ģʽ */

    if (lcdltdc.pwidth != 0)    /* RGB�� */
    {
        dcmi_rx_callback = rgblcd_dcmi_rx_callback; /* RGB���������ݻص����� */
        dcmi_dma_init((uint32_t)p_dcmi_line_buf[0], (uint32_t)p_dcmi_line_buf[1], lcddev.width / 2, 1, 1);  /* DCMI DMA���� */
    }
    else                                            /* MCU �� */
    {
        dcmi_dma_init((uint32_t)&LCD->LCD_RAM, 0, 1, 1, 0); /* DCMI DMA����,MCU��,���� */
    }
    
    myfree(SRAMIN, f_jpg);
    myfree(SRAMIN, databuf);

    return res;
}

int main(void)
{
    uint8_t res;
    float fac;
    uint8_t *pname;                         /* ��·�����ļ��� */
    uint8_t key;                            /* ��ֵ */
    uint8_t i;
    uint8_t sd_ok = 1;                      /* 0,sd��������;1,SD������ */
    uint8_t scale = 1;                      /* Ĭ����ȫ�ߴ����� */
    uint8_t msgbuf[15];                     /* ��Ϣ������ */
    uint16_t outputheight = 0;

    sys_stm32_clock_init(160, 5, 2, 4);     /* ����ʱ��, 400Mhz */
    delay_init(400);                        /* ��ʱ��ʼ�� */
    usart_init(100, 115200);                /* ��ʼ��USART */
    usmart_dev.init(100);                   /* ��ʼ��USMART */
    led_init();                             /* ��ʼ��LED */
    mpu_memory_protection();                /* ������ش洢���� */
    sdram_init();                           /* ��ʼ��SDRAM */
    lcd_init();                             /* ��ʼ��LCD */
    norflash_init();                        /* ��ʼ��W25Q256 */
    key_init();                             /* ��ʼ������ */
    ov5640_init();                          /* ��ʼ��OV5640 */
    sw_sdcard_mode();                       /* �����л�ΪOV5640ģʽ */
    pcf8574_init();                         /* ��ʼ��PCF8574 */
    piclib_init();                          /* ��ʼ����ͼ */
    
    my_mem_init(SRAMIN);                    /* ��ʼ���ڲ��ڴ��(AXI) */
    my_mem_init(SRAMEX);                    /* ��ʼ���ⲿ�ڴ��(SDRAM) */
    my_mem_init(SRAM12);                    /* ��ʼ��SRAM12�ڴ��(SRAM1+SRAM2) */
    my_mem_init(SRAM4);                     /* ��ʼ��SRAM4�ڴ��(SRAM4) */
    my_mem_init(SRAMDTCM);                  /* ��ʼ��DTCM�ڴ��(DTCM) */
    my_mem_init(SRAMITCM);                  /* ��ʼ��ITCM�ڴ��(ITCM) */

    exfuns_init();                          /* Ϊfatfs��ر��������ڴ� */

    f_mount(fs[0], "0:", 1);                /* ����SD�� */
    f_mount(fs[1], "1:", 1);                /* ����SPI FLASH */
    f_mount(fs[2], "2:", 1);                /* ����NAND FLASH */

     while (fonts_init())                   /* ����ֿ� */
    {
        lcd_show_string(30, 90, 200, 16, 16, "Font error!", RED);
        delay_ms(200);
        lcd_fill(20, 90, 200 + 20, 90 + 16, WHITE);
        delay_ms(200);
    }

    text_show_string(30, 50, 200, 16, "����ԭ��STM32������",16,0, RED);
    text_show_string(30, 70, 200, 16, "�����ʵ��", 16, 0, RED);
    text_show_string(30, 90, 200, 16, "KEY0:����(bmp��ʽ)", 16, 0, RED);
    text_show_string(30, 110, 200, 16, "KEY1:����(jpg��ʽ)", 16, 0, RED);
    text_show_string(30, 130, 200, 16, "KEY2:�Զ��Խ�", 16, 0, RED);
    text_show_string(30, 150, 200, 16, "WK_UP:FullSize/Scale", 16, 0, RED);

    res = f_mkdir("0:/PHOTO");                                  /* ����PHOTO�ļ��� */
    
    if (res != FR_EXIST && res != FR_OK)                        /* �����˴��� */
    {
        res = f_mkdir("0:/PHOTO");                              /* ����PHOTO�ļ��� */
        text_show_string(30, 150, 240, 16, "SD������!", 16, 0, RED);
        delay_ms(200);
        text_show_string(30, 150, 240, 16, "���չ��ܽ�������!", 16, 0, RED);
        delay_ms(200);
        sd_ok = 0;
    }

    p_dcmi_line_buf[0] = mymalloc(SRAMIN, jpeg_line_size * 4);  /* Ϊjpeg dma���������ڴ� */
    p_dcmi_line_buf[1] = mymalloc(SRAMIN, jpeg_line_size * 4);  /* Ϊjpeg dma���������ڴ� */
    p_jpeg_data_buf = mymalloc(SRAMEX, jpeg_buf_size);          /* Ϊjpeg�ļ������ڴ� */
    pname = mymalloc(SRAMIN, 30);                               /* Ϊ��·�����ļ�������30���ֽڵ��ڴ� */

    while (pname == NULL || !p_dcmi_line_buf[0] || !p_dcmi_line_buf[1] || !p_jpeg_data_buf) /* �ڴ������� */
    {
        text_show_string(30, 170, 240, 16, "�ڴ����ʧ��!", 16, 0, RED);
        delay_ms(200);
        lcd_fill(30, 170, 240, 146, WHITE);                     /* �����ʾ */
        delay_ms(200);
    }

    while (ov5640_init())                           /* ��ʼ��OV5640 */
    {
        text_show_string(30, 190, 240, 16, "OV5640 ����!", 16, 0, RED);
        delay_ms(200);
        lcd_fill(30, 190, 239, 206, WHITE);
        delay_ms(200);
    }

    text_show_string(30, 190, 230, 16, "OV5640 ����", 16, 0, RED);
    
    ov5640_rgb565_mode();                           /* RGB565ģʽ */
    ov5640_focus_init(); 
    ov5640_light_mode(0);                           /* �Զ�ģʽ */
    ov5640_color_saturation(3);                     /* ɫ�ʱ��Ͷ�0 */
    ov5640_brightness(4);                           /* ����0 */
    ov5640_contrast(3);                             /* �Աȶ�0 */
    ov5640_sharpness(33);                           /* �Զ���� */
    ov5640_focus_constant();                        /* ���������Խ� */
    dcmi_init();                                    /* DCMI���� */

    if (lcdltdc.pwidth != 0)                        /* RGB�� */
    {
        dcmi_rx_callback = rgblcd_dcmi_rx_callback; /* RGB���������ݻص����� */
        dcmi_dma_init((uint32_t)p_dcmi_line_buf[0], (uint32_t)p_dcmi_line_buf[1], lcddev.width / 2, 1, 1);  /* DCMI DMA���� */
    }
    else                                            /* MCU �� */
    {
        dcmi_dma_init((uint32_t)&LCD->LCD_RAM, 0, 1, 1, 0); /* DCMI DMA����,MCU��,���� */
    }
    if (lcddev.height == 1024)
    {
        g_yoffset = (lcddev.height - 800) / 2;
        outputheight = 800;
        ov5640_write_reg(0x3035, 0X51);             /* �������֡�ʣ�������ܶ��� */
    }
    else if (lcddev.height == 1280)
    {
        g_yoffset = (lcddev.height - 600) / 2;
        outputheight = 600;
        ov5640_write_reg(0x3035, 0X51);             /* �������֡�ʣ�������ܶ��� */
    }
    else 
    {
        g_yoffset = 0;
        outputheight = lcddev.height;
    }
    g_curline = g_yoffset;                                 /* ������λ */
    ov5640_outsize_set(16, 4, lcddev.width, outputheight); /* ����������ʾ */
    dcmi_start();                                          /* �������� */
    lcd_clear(BLACK);

    while (1)
    {
        key = key_scan(0);

        if (key)
        { 
            if (key != KEY2_PRES)
            {
                if (key == KEY0_PRES)               /* �����BMP����,��ȴ�1����,ȥ����,�Ի���ȶ���bmp��Ƭ */
                {
                    delay_ms(300);
                    g_bmp_request = 1;              /* ����ر�DCMI */
                    while (g_bmp_request);          /* �ȴ���������� */
                 }
                else
                {
                    dcmi_stop();
                }
            }
            if (key == WKUP_PRES)                   /* ���Ŵ��� */
            {
                scale =! scale;  
                if (scale == 0)
                {
                    fac = (float)800 / outputheight;/* �õ��������� */
                    ov5640_outsize_set((1280 - fac * lcddev.width) / 2, (800 - fac * outputheight) / 2, lcddev.width, outputheight);
                    sprintf((char *)msgbuf, "Full Size 1:1");
                }
                else 
                {
                    ov5640_outsize_set(16, 4, lcddev.width, outputheight);
                    sprintf((char *)msgbuf, "Scale");
                }
                delay_ms(800);     
            }
            else if (key == KEY2_PRES)
            {
                ov5640_focus_single();                 /* �ֶ������Զ��Խ� */
            }
            else if (sd_ok)                            /* SD�������ſ������� */
            {    
                sw_sdcard_mode();                      /* �л�ΪSD��ģʽ */
                if (key == KEY0_PRES)                  /* BMP���� */
                {
                    camera_new_pathname(pname, 0);     /* �õ��ļ��� */
                    res = bmp_encode(pname, 0, g_yoffset, lcddev.width, outputheight, 0);
                    sw_ov5640_mode();                  /* �л�ΪOV5640ģʽ */
                }
                else if (key == KEY1_PRES)             /* JPG���� */
                {
                    camera_new_pathname(pname, 1);     /* �õ��ļ��� */
                    res = ov5640_jpg_photo(pname);
                    if (scale == 0)
                    {
                        fac = 800 / lcddev.height;     /* �õ��������� */
                        ov5640_outsize_set((1280 - fac * lcddev.width) / 2, (800 - fac * lcddev.height) / 2, lcddev.width, lcddev.height);      
                     }
                    else 
                    {
                        ov5640_outsize_set(16, 4, lcddev.width, outputheight);
                     }
                    if (lcddev.height > 800)
                    {
                        ov5640_write_reg(0x3035, 0X51); /* �������֡�ʣ�������ܶ��� */
                    }
                }
                if (res)                                /* �������� */
                {
                    text_show_string(30, 130, 240, 16, "д���ļ�����!", 16, 0, RED);
                }
                else 
                {
                    text_show_string(30, 130, 240, 16, "���ճɹ�!", 16, 0, RED);
                    text_show_string(30, 150, 240, 16, "����Ϊ:", 16, 0, RED);
                    text_show_string(30 + 56, 150, 240, 16, (char *)pname, 16, 0, RED);
                    pcf8574_write_bit(BEEP_IO, 0);    /* �������̽У���ʾ������� */
                    delay_ms(100);
                    pcf8574_write_bit(BEEP_IO, 1);    /* �رշ����� */
                }  
                delay_ms(1000);                       /* �ȴ�1���� */
                dcmi_start();                         /* ������ʹ��dcmi,Ȼ�������ر�DCMI,�����ٿ���DCMI,���Է�ֹRGB���Ĳ�������. */
                dcmi_stop();
            }
            else                                      /* ��ʾSD������ */
            {
                text_show_string(30, 130, 240, 16, "SD������!", 16, 0, RED);
                text_show_string(30, 150, 240, 16, "���չ��ܲ�����!", 16, 0, RED);
            }           
            if (key != KEY2_PRES)
            {
                dcmi_start();                         /* ��ʼ��ʾ */
            }
        } 

        delay_ms(10);
        i++;

        if (i == 20)                                  /* DS0��˸.*/
        {
            i = 0;
            LED0_TOGGLE();
         }
    }
}





