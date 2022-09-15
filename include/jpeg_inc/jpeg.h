#ifndef _JPEG_H
#define _JPEG_H

//一、头文件
#include <stdio.h>   	
#include <fcntl.h>	 	 
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <stdlib.h>
#include "jpeg.h"
#include "jpeglib.h"

//二、宏定义区域
#define LCD_WIDTH  			800
#define LCD_HEIGHT 			480
#define FB_SIZE				(LCD_WIDTH * LCD_HEIGHT * 4)
#define BUFFER_COUNT 		4


//三、数据类型变量定义区域
typedef struct VideoBuffer 
{
    void   *start;  	//映射到用户空间的地址
    size_t  length; 	//采集到数据的长度
} VideoBuffer1; 		//存放采集数据的位置


VideoBuffer1 framebuf[BUFFER_COUNT]; 


//四、函数声明区域
int Jpeg_Lcd_Open(void);	//1、打开显示屏(jpeg)
int Jpeg_Lcd_Show(unsigned int x,unsigned int y,const char *pjpg_path);
							//2、显示jpg图片
int Jpeg_Lcd_Show_Camera(unsigned int x,unsigned int y,char *pjpg_buf,unsigned int jpg_buf_size);
							//3、显示摄像头捕捉画面 
int Jpeg_Lcd_Close(void);	//4、关闭显示屏(jpeg)


#endif