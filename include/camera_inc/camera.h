#ifndef __CAMERA_H
#define __CAMERA_H

//一、函数头文件
#include <stdio.h>
#include "jpeglib.h"
#include "yuyv.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <strings.h>
#include <linux/input.h>
#include "jpeg.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <pthread.h>

//二、函数声明
//1、打开摄像头
void Camera_Open(char *pathname_camera);
//2、运行摄像头
void Camera_Show(int lcd_x, int lcd_y);
//3、关闭摄像头
void Camera_Close(void);


//三、宏定义、变量声明



#endif /* __CAMERA_H */