/**
 ****************************************************************************************************
 * @file        main.c
 * @author      正点原子团队(ALIENTEK)
 * @version     V1.0
 * @date        2023-03-25
 * @brief       照相机 实验
 * @license     Copyright (c) 2022-2032, 广州市星翼电子科技有限公司
 ****************************************************************************************************
 * @attention
 *
 * 实验平台:正点原子 阿波罗 H743开发板
 * 在线视频:www.yuanzige.com
 * 技术论坛:www.openedv.com
 * 公司网址:www.alientek.com
 * 购买地址:openedv.taobao.com
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



__IO uint8_t g_bmp_request = 0;                /* bmp拍照请求:0,无bmp拍照请求;1,有bmp拍照请求,需要在帧中断里面,关闭DCMI接口. */
uint8_t g_ov_mode = 0;                         /* bit0: 0, RGB565模式;  1, JPEG模式 */
uint16_t g_curline = 0;                        /* 摄像头输出数据,当前行编号 */
uint16_t g_yoffset = 0;                        /* y方向的偏移量 */

#define jpeg_buf_size       4 * 1024 * 1024    /* 定义JPEG数据缓存jpeg_buf的大小(字节) */
#define jpeg_line_size      4 * 1024           /* 定义DMA接收数据时,一行数据的最大值 */

uint32_t *p_dcmi_line_buf[2];                  /* RGB屏时,摄像头采用一行一行读取,定义行缓存 */
uint32_t *p_jpeg_data_buf;                     /* JPEG数据缓存buf指针 */


/* buf中的JPEG有效数据长度 */
volatile uint32_t g_jpeg_data_len = 0;       


/**
 * JPEG数据采集完成标志
 * 0,数据没有采集完;
 * 1,数据采集完了,但是还没处理;
 * 2,数据已经处理完成了,可以开始下一帧接收
 */
volatile uint8_t g_jpeg_data_ok = 0;

/**
 * @brief       处理JPEG数据
 * @ntoe        在DCMI_IRQHandler中断服务函数里面被调用
 *              当采集完一帧JPEG数据后,调用此函数,切换JPEG BUF.开始下一帧采集.
 *
 * @param       无
 * @retval      无
 */
void jpeg_data_process(void)
{
    uint16_t i, rlen;
    uint32_t *pbuf;
    g_curline = g_yoffset;                                                        /* 行数复位 */

    if (g_ov_mode & 0X01)                                                         /* 只有在JPEG格式下,才需要做处理. */
    {
        if (g_jpeg_data_ok == 0)    /* jpeg数据还未采集完? */
        {
            DMA1_Stream1->CR &= ~(1 << 0);      /* 停止当前传输 */
    
            while (DMA1_Stream1->CR & 0X01);    /* 等待DMA1_Stream1可配置 */

            rlen = jpeg_line_size - DMA1_Stream1->NDTR; /* 得到剩余数据长度 */
            pbuf = p_jpeg_data_buf + g_jpeg_data_len;   /* 偏移到有效数据末尾,继续添加 */

            if (DMA1_Stream1->CR & (1 << 19))
            {
                for (i = 0; i < rlen; i++)
                {
                    pbuf[i] = p_dcmi_line_buf[1][i];    /* 读取buf1里面的剩余数据 */
                }
            }
            else 
            {
                for (i = 0; i < rlen; i++)
                {
                    pbuf[i] = p_dcmi_line_buf[0][i];    /* 读取buf0里面的剩余数据 */
                }
            }

            g_jpeg_data_len += rlen;    /* 加上剩余长度 */
            g_jpeg_data_ok = 1;         /* 标记JPEG数据采集完成,等待其他函数处理 */
        }

        if (g_jpeg_data_ok == 2)        /* 上一次的jpeg数据已经被处理了 */
        {
            DMA1_Stream1->NDTR = jpeg_line_size;    /* 传输长度为jpeg_buf_size*4字节 */
            DMA1_Stream1->CR |= 1 << 0; /* 重新传输 */
            g_jpeg_data_ok = 0;         /* 标记数据未采集 */
            g_jpeg_data_len = 0;        /* 数据重新开始 */
        }
    }
    else
    {
        if (g_bmp_request == 1)         /* 有bmp拍照请求,关闭DCMI */
        {
            dcmi_stop();                /* 停止DCMI */
            g_bmp_request = 0;          /* 标记请求处理完成 */
        }
        if (lcdltdc.pwidth == 0)
        {
            lcd_set_cursor(0, 0);
            lcd_write_ram_prepare();    /* 开始写入GRAM */
        }
    }
}

/**
 * @brief       JPEG数据接收回调函数
 * @param       无
 * @retval      无
 */
void jpeg_dcmi_rx_callback(void)
{
    uint16_t i;
    uint32_t *pbuf;

    pbuf = p_jpeg_data_buf + g_jpeg_data_len;               /* 偏移到有效数据末尾 */

    if (DMA1_Stream1->CR & (1 << 19))                       /* buf0已满,正常处理buf1 */
    {
        for (i = 0; i < jpeg_line_size; i++)
        {
            pbuf[i] = p_dcmi_line_buf[0][i];                /* 读取buf0里面的数据 */
        }
        
        g_jpeg_data_len += jpeg_line_size;                  /* 偏移 */
    }
    else                                                    /* buf1已满,正常处理buf0 */
    {
        for (i = 0; i < jpeg_line_size; i++)
        {
            pbuf[i] = p_dcmi_line_buf[1][i];                /* 读取buf1里面的数据 */
        }
        
        g_jpeg_data_len += jpeg_line_size;                  /* 偏移 */
    }
}
\
/**
 * @brief       RGB屏数据接收回调函数
 * @param       无
 * @retval      无
 */
void rgblcd_dcmi_rx_callback(void)
{
    uint16_t *pbuf;

    if (DMA1_Stream1->CR & (1 << 19))                       /* DMA使用buf1,读取buf0 */
    { 
        pbuf = (uint16_t*)p_dcmi_line_buf[0]; 
    }
    else                                                    /* DMA使用buf0,读取buf1 */
    {
        pbuf = (uint16_t*)p_dcmi_line_buf[1]; 
    }     

    ltdc_color_fill(0, g_curline, lcddev.width - 1, g_curline, pbuf);   /* DM2D填充 */

    if (g_curline < lcddev.height) g_curline++;
    if (g_bmp_request == 1 && g_curline == (lcddev.height - 1))         /* 有bmp拍照请求,关闭DCMI */
    {
        dcmi_stop();                                        /* 停止DCMI */
        g_bmp_request = 0;                                  /* 标记请求处理完成 */
    }
}

/**
 * @brief       切换为OV5640模式
 * @note        切换PC8/PC9/PC11为DCMI复用功能(AF13)
 * @param       无
 * @retval      无
 */
void sw_ov5640_mode(void)
{
    ov5640_write_reg(0X3017, 0XFF);             /* 开启OV5650输出(可以正常显示) */
    ov5640_write_reg(0X3018, 0XFF); 
    
    sys_gpio_af_set(GPIOC, SYS_GPIO_PIN8, 13);  /* PC8,AF13  DCMI_D2 */
    sys_gpio_af_set(GPIOC, SYS_GPIO_PIN9, 13);  /* PC9,AF13  DCMI_D3 */
    sys_gpio_af_set(GPIOC, SYS_GPIO_PIN11, 13); /* PC11,AF13 DCMI_D4 */
}

/**
 * @brief       切换为SD卡模式
 * @note        切换PC8/PC9/PC11为SDMMC复用功能(AF12)
 * @param       无
 * @retval      无
 */
void sw_sdcard_mode(void)
{
    ov5640_write_reg(0X3017, 0X00);             /* 关闭OV5650输出(可以正常显示) */
    ov5640_write_reg(0X3018, 0X00); 

    sys_gpio_af_set(GPIOC, SYS_GPIO_PIN8, 12);  /* PC8,AF12  SDIO_D0 */
    sys_gpio_af_set(GPIOC, SYS_GPIO_PIN9, 12);  /* PC9,AF12  SDIO_D1 */
    sys_gpio_af_set(GPIOC, SYS_GPIO_PIN11, 12); /* PC11,AF12 SDIO_D3 */
}

/**
 * @brief       文件名自增（避免覆盖）
 * @note        bmp组合成: 形如 "0:PHOTO/PIC13141.bmp" 的文件名
 *              jpg组合成: 形如 "0:PHOTO/PIC13141.jpg" 的文件名
 * @param       pname : 有效的文件名
 * @param       mode  : 0, 创建.bmp文件;  1, 创建.jpg文件;
 * @retval      无
 */
void camera_new_pathname(uint8_t *pname, uint8_t mode)
{
    uint8_t res;
    uint16_t index = 0;
    FIL *ftemp;

    ftemp = (FIL *)mymalloc(SRAMIN, sizeof(FIL));           /* 开辟FIL字节的内存区域 */

    while (index < 0XFFFF)
    {
        if (mode == 0)                                      /* 创建.bmp文件名 */
        {
            sprintf((char *)pname, "0:PHOTO/PIC%05d.bmp", index);
        }
        else                                                /* 创建.jpg文件名 */
        {
            sprintf((char *)pname, "0:PHOTO/PIC%05d.jpg", index);
        }
        
        res = f_open(ftemp, (const TCHAR *)pname, FA_READ); /* 尝试打开这个文件 */

        if (res == FR_NO_FILE)
        {
            break;                        /* 该文件名不存在, 正是我们需要的 */
        }

        index++;
    }
    myfree(SRAMIN, ftemp);
}
/**
 * @brief       OV5640拍照jpg图片
 * @param       pname : 要创建的jpg文件名(含路径)
 * @retval      0, 成功; 其他,错误代码;
 */
uint8_t ov5640_jpg_photo(uint8_t *pname)
{
    FIL *f_jpg;
    uint8_t res = 0, headok = 0;
    uint32_t bwr;
    uint32_t i, jpgstart, jpglen;
    uint8_t *pbuf;
    
    uint16_t datasize = 0;          /* 单次写入数据量 */
    uint32_t datalen = 0;           /* 总写入数据量 */
    uint8_t  *databuf;              /* 数据缓存，避免直接写外部SRAM数据到SD卡，导致写入下溢错误 */

    f_jpg = (FIL *)mymalloc(SRAMIN, sizeof(FIL));   /* 开辟FIL字节的内存区域 */
    databuf = mymalloc(SRAMIN, 4096);   /* 申请4K内存 */

    if (databuf == NULL)
    {
        myfree(SRAMIN, f_jpg);      /* 释放f_jpg内存 */
        return 0XFF;                /* 内存申请失败 */
    }

    g_ov_mode = 1;
    g_jpeg_data_ok = 0;
    
    sw_ov5640_mode();                               /* 切换为OV5640模式 */
    ov5640_jpeg_mode();                             /* JPEG模式 */
    ov5640_outsize_set(16, 4, 2592, 1944);          /* 设置输出尺寸(500W) */

    dcmi_rx_callback = jpeg_dcmi_rx_callback;       /* JPEG接收数据回调函数 */
    dcmi_dma_init((uint32_t)p_dcmi_line_buf[0], (uint32_t)p_dcmi_line_buf[1], jpeg_line_size, 2, 1);    /* DCMI DMA配置 */
    
    dcmi_start();                                   /* 启动传输 */
    while (g_jpeg_data_ok != 1);                    /* 等待第一帧图片采集完 */
    g_jpeg_data_ok = 2;                             /* 忽略本帧图片,启动下一帧采集 */
    while (g_jpeg_data_ok != 1);                    /* 等待第二帧图片采集完,第二帧,才保存到SD卡去 */

    dcmi_stop();                                    /* 停止DMA搬运 */
    g_ov_mode = 0;
    sw_sdcard_mode();                               /* 切换为SD卡模式 */
    

    printf("jpeg data size:%d\r\n", g_jpeg_data_len * 4);   /* 串口打印JPEG文件大小 */
    pbuf = (uint8_t *)p_jpeg_data_buf;
    jpglen = 0;                                     /* 设置jpg文件大小为0 */
    headok = 0;                                     /* 清除jpg头标记 */

    for (i = 0; i < g_jpeg_data_len * 4; i++)       /* 查找0XFF,0XD8和0XFF,0XD9,获取jpg文件大小 */
    {
        if ((pbuf[i] == 0XFF) && (pbuf[i + 1] == 0XD8))     /* 找到FF D8 */
        {
            jpgstart = i;
            headok = 1;                             /* 标记找到jpg头(FF D8) */
        }

        if ((pbuf[i] == 0XFF) && (pbuf[i + 1] == 0XD9) && headok)   /* 找到头以后,再找FF D9 */
        {
            jpglen = i - jpgstart + 2;
            break;
        }
    }

    if (jpglen)                     /* 正常的jpeg数据 */
    {
        res = f_open(f_jpg, (const TCHAR *)pname, FA_WRITE | FA_CREATE_NEW);    /* 模式0,或者尝试打开失败,则创建新文件 */

        if (res == 0)
        {
            pbuf += jpgstart;       /* 偏移到0XFF,0XD8处 */
            
            while(datalen < jpglen) /* 循环写入！不能直接写外部SRAM数据到SDIO，否则可能引起FIFO下溢错误 */
            {
                if((jpglen - datalen) > 4096)
                {
                    datasize = 4096;
                }else
                {
                    datasize = jpglen - datalen;    /* 最后的数据 */
                }

                my_mem_copy(databuf, pbuf, datasize);
                res = f_write(f_jpg, databuf, datasize, (UINT *)&bwr); /* 写入内容 */
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
    sw_ov5640_mode();           /* 切换为OV5640模式 */
    ov5640_rgb565_mode();       /* RGB565模式 */

    if (lcdltdc.pwidth != 0)    /* RGB屏 */
    {
        dcmi_rx_callback = rgblcd_dcmi_rx_callback; /* RGB屏接收数据回调函数 */
        dcmi_dma_init((uint32_t)p_dcmi_line_buf[0], (uint32_t)p_dcmi_line_buf[1], lcddev.width / 2, 1, 1);  /* DCMI DMA配置 */
    }
    else                                            /* MCU 屏 */
    {
        dcmi_dma_init((uint32_t)&LCD->LCD_RAM, 0, 1, 1, 0); /* DCMI DMA配置,MCU屏,竖屏 */
    }
    
    myfree(SRAMIN, f_jpg);
    myfree(SRAMIN, databuf);

    return res;
}

int main(void)
{
    uint8_t res;
    float fac;
    uint8_t *pname;                         /* 带路径的文件名 */
    uint8_t key;                            /* 键值 */
    uint8_t i;
    uint8_t sd_ok = 1;                      /* 0,sd卡不正常;1,SD卡正常 */
    uint8_t scale = 1;                      /* 默认是全尺寸缩放 */
    uint8_t msgbuf[15];                     /* 消息缓存区 */
    uint16_t outputheight = 0;

    sys_stm32_clock_init(160, 5, 2, 4);     /* 设置时钟, 400Mhz */
    delay_init(400);                        /* 延时初始化 */
    usart_init(100, 115200);                /* 初始化USART */
    usmart_dev.init(100);                   /* 初始化USMART */
    led_init();                             /* 初始化LED */
    mpu_memory_protection();                /* 保护相关存储区域 */
    sdram_init();                           /* 初始化SDRAM */
    lcd_init();                             /* 初始化LCD */
    norflash_init();                        /* 初始化W25Q256 */
    key_init();                             /* 初始化按键 */
    ov5640_init();                          /* 初始化OV5640 */
    sw_sdcard_mode();                       /* 首先切换为OV5640模式 */
    pcf8574_init();                         /* 初始化PCF8574 */
    piclib_init();                          /* 初始化画图 */
    
    my_mem_init(SRAMIN);                    /* 初始化内部内存池(AXI) */
    my_mem_init(SRAMEX);                    /* 初始化外部内存池(SDRAM) */
    my_mem_init(SRAM12);                    /* 初始化SRAM12内存池(SRAM1+SRAM2) */
    my_mem_init(SRAM4);                     /* 初始化SRAM4内存池(SRAM4) */
    my_mem_init(SRAMDTCM);                  /* 初始化DTCM内存池(DTCM) */
    my_mem_init(SRAMITCM);                  /* 初始化ITCM内存池(ITCM) */

    exfuns_init();                          /* 为fatfs相关变量申请内存 */

    f_mount(fs[0], "0:", 1);                /* 挂载SD卡 */
    f_mount(fs[1], "1:", 1);                /* 挂载SPI FLASH */
    f_mount(fs[2], "2:", 1);                /* 挂载NAND FLASH */

     while (fonts_init())                   /* 检查字库 */
    {
        lcd_show_string(30, 90, 200, 16, 16, "Font error!", RED);
        delay_ms(200);
        lcd_fill(20, 90, 200 + 20, 90 + 16, WHITE);
        delay_ms(200);
    }

    text_show_string(30, 50, 200, 16, "正点原子STM32开发板",16,0, RED);
    text_show_string(30, 70, 200, 16, "照相机实验", 16, 0, RED);
    text_show_string(30, 90, 200, 16, "KEY0:拍照(bmp格式)", 16, 0, RED);
    text_show_string(30, 110, 200, 16, "KEY1:拍照(jpg格式)", 16, 0, RED);
    text_show_string(30, 130, 200, 16, "KEY2:自动对焦", 16, 0, RED);
    text_show_string(30, 150, 200, 16, "WK_UP:FullSize/Scale", 16, 0, RED);

    res = f_mkdir("0:/PHOTO");                                  /* 创建PHOTO文件夹 */
    
    if (res != FR_EXIST && res != FR_OK)                        /* 发生了错误 */
    {
        res = f_mkdir("0:/PHOTO");                              /* 创建PHOTO文件夹 */
        text_show_string(30, 150, 240, 16, "SD卡错误!", 16, 0, RED);
        delay_ms(200);
        text_show_string(30, 150, 240, 16, "拍照功能将不可用!", 16, 0, RED);
        delay_ms(200);
        sd_ok = 0;
    }

    p_dcmi_line_buf[0] = mymalloc(SRAMIN, jpeg_line_size * 4);  /* 为jpeg dma接收申请内存 */
    p_dcmi_line_buf[1] = mymalloc(SRAMIN, jpeg_line_size * 4);  /* 为jpeg dma接收申请内存 */
    p_jpeg_data_buf = mymalloc(SRAMEX, jpeg_buf_size);          /* 为jpeg文件申请内存 */
    pname = mymalloc(SRAMIN, 30);                               /* 为带路径的文件名分配30个字节的内存 */

    while (pname == NULL || !p_dcmi_line_buf[0] || !p_dcmi_line_buf[1] || !p_jpeg_data_buf) /* 内存分配出错 */
    {
        text_show_string(30, 170, 240, 16, "内存分配失败!", 16, 0, RED);
        delay_ms(200);
        lcd_fill(30, 170, 240, 146, WHITE);                     /* 清除显示 */
        delay_ms(200);
    }

    while (ov5640_init())                           /* 初始化OV5640 */
    {
        text_show_string(30, 190, 240, 16, "OV5640 错误!", 16, 0, RED);
        delay_ms(200);
        lcd_fill(30, 190, 239, 206, WHITE);
        delay_ms(200);
    }

    text_show_string(30, 190, 230, 16, "OV5640 正常", 16, 0, RED);
    
    ov5640_rgb565_mode();                           /* RGB565模式 */
    ov5640_focus_init(); 
    ov5640_light_mode(0);                           /* 自动模式 */
    ov5640_color_saturation(3);                     /* 色彩饱和度0 */
    ov5640_brightness(4);                           /* 亮度0 */
    ov5640_contrast(3);                             /* 对比度0 */
    ov5640_sharpness(33);                           /* 自动锐度 */
    ov5640_focus_constant();                        /* 启动持续对焦 */
    dcmi_init();                                    /* DCMI配置 */

    if (lcdltdc.pwidth != 0)                        /* RGB屏 */
    {
        dcmi_rx_callback = rgblcd_dcmi_rx_callback; /* RGB屏接收数据回调函数 */
        dcmi_dma_init((uint32_t)p_dcmi_line_buf[0], (uint32_t)p_dcmi_line_buf[1], lcddev.width / 2, 1, 1);  /* DCMI DMA配置 */
    }
    else                                            /* MCU 屏 */
    {
        dcmi_dma_init((uint32_t)&LCD->LCD_RAM, 0, 1, 1, 0); /* DCMI DMA配置,MCU屏,竖屏 */
    }
    if (lcddev.height == 1024)
    {
        g_yoffset = (lcddev.height - 800) / 2;
        outputheight = 800;
        ov5640_write_reg(0x3035, 0X51);             /* 降低输出帧率，否则可能抖动 */
    }
    else if (lcddev.height == 1280)
    {
        g_yoffset = (lcddev.height - 600) / 2;
        outputheight = 600;
        ov5640_write_reg(0x3035, 0X51);             /* 降低输出帧率，否则可能抖动 */
    }
    else 
    {
        g_yoffset = 0;
        outputheight = lcddev.height;
    }
    g_curline = g_yoffset;                                 /* 行数复位 */
    ov5640_outsize_set(16, 4, lcddev.width, outputheight); /* 满屏缩放显示 */
    dcmi_start();                                          /* 启动传输 */
    lcd_clear(BLACK);

    while (1)
    {
        key = key_scan(0);

        if (key)
        { 
            if (key != KEY2_PRES)
            {
                if (key == KEY0_PRES)               /* 如果是BMP拍照,则等待1秒钟,去抖动,以获得稳定的bmp照片 */
                {
                    delay_ms(300);
                    g_bmp_request = 1;              /* 请求关闭DCMI */
                    while (g_bmp_request);          /* 等带请求处理完成 */
                 }
                else
                {
                    dcmi_stop();
                }
            }
            if (key == WKUP_PRES)                   /* 缩放处理 */
            {
                scale =! scale;  
                if (scale == 0)
                {
                    fac = (float)800 / outputheight;/* 得到比例因子 */
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
                ov5640_focus_single();                 /* 手动单次自动对焦 */
            }
            else if (sd_ok)                            /* SD卡正常才可以拍照 */
            {    
                sw_sdcard_mode();                      /* 切换为SD卡模式 */
                if (key == KEY0_PRES)                  /* BMP拍照 */
                {
                    camera_new_pathname(pname, 0);     /* 得到文件名 */
                    res = bmp_encode(pname, 0, g_yoffset, lcddev.width, outputheight, 0);
                    sw_ov5640_mode();                  /* 切换为OV5640模式 */
                }
                else if (key == KEY1_PRES)             /* JPG拍照 */
                {
                    camera_new_pathname(pname, 1);     /* 得到文件名 */
                    res = ov5640_jpg_photo(pname);
                    if (scale == 0)
                    {
                        fac = 800 / lcddev.height;     /* 得到比例因子 */
                        ov5640_outsize_set((1280 - fac * lcddev.width) / 2, (800 - fac * lcddev.height) / 2, lcddev.width, lcddev.height);      
                     }
                    else 
                    {
                        ov5640_outsize_set(16, 4, lcddev.width, outputheight);
                     }
                    if (lcddev.height > 800)
                    {
                        ov5640_write_reg(0x3035, 0X51); /* 降低输出帧率，否则可能抖动 */
                    }
                }
                if (res)                                /* 拍照有误 */
                {
                    text_show_string(30, 130, 240, 16, "写入文件错误!", 16, 0, RED);
                }
                else 
                {
                    text_show_string(30, 130, 240, 16, "拍照成功!", 16, 0, RED);
                    text_show_string(30, 150, 240, 16, "保存为:", 16, 0, RED);
                    text_show_string(30 + 56, 150, 240, 16, (char *)pname, 16, 0, RED);
                    pcf8574_write_bit(BEEP_IO, 0);    /* 蜂鸣器短叫，提示拍照完成 */
                    delay_ms(100);
                    pcf8574_write_bit(BEEP_IO, 1);    /* 关闭蜂鸣器 */
                }  
                delay_ms(1000);                       /* 等待1秒钟 */
                dcmi_start();                         /* 这里先使能dcmi,然后立即关闭DCMI,后面再开启DCMI,可以防止RGB屏的侧移问题. */
                dcmi_stop();
            }
            else                                      /* 提示SD卡错误 */
            {
                text_show_string(30, 130, 240, 16, "SD卡错误!", 16, 0, RED);
                text_show_string(30, 150, 240, 16, "拍照功能不可用!", 16, 0, RED);
            }           
            if (key != KEY2_PRES)
            {
                dcmi_start();                         /* 开始显示 */
            }
        } 

        delay_ms(10);
        i++;

        if (i == 20)                                  /* DS0闪烁.*/
        {
            i = 0;
            LED0_TOGGLE();
         }
    }
}





