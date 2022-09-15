#ifndef __MAIN_H		//防止递归包含
#define __MAIN_H

//一、函数所需的头文件
#include <stdio.h>
//open函数、lseek函数
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
//close函数、read函数、write函数、lseek函数
#include <unistd.h>
//映射函数、解除映射函数
#include <sys/mman.h>
//malloc函数、free函数
#include <stdlib.h>
//输入子系统模型的头文件
#include <linux/input.h>
//bzero函数
#include <strings.h>
#include <string.h>
#include <time.h>
#include <errno.h>

// #include "show_color.c"
// #include "touch_screen.c"

// #include "jpeg.h"
// #include "camera.h"
// #include "jpeglib.h"
// #include "jconfig.h"
// #include "jerror.h"
// #include "jmorecfg.h"
// #include "kernel_list.h"
// #include "yuyv.h"
// #include "jpeg.c"
// #include "camera.c"
// #include "api_v4l2.h"

//二、函数的声明
int show_color(int lcd_x, int lcd_y, int color_w, int color_h, int color);		//1、显示颜色(任意位置大小显示任意颜色)
// int show_bmp(int lcd_x, int lcd_y, int bmp_w, int bmp_h, const char *bmp);		//1、显示bmp图片(任意位置、大小、图片显示)
int get_ts_value(int *ts_x, int *ts_y, int *ts_touch);							//1、获取触摸屏的值(坐标值、触摸值)


#endif /*__MAIN_H*/