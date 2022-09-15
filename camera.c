#include "camera.h"
#include "jpeg.h"
#include "main.h"
// #include "touch_screen.c"

struct jpg_data video_buf;

//1、打开摄像头
void Camera_Open(char *pathname_camera)
{
	//a、打开开发板屏幕、申请开发板虚拟内存
	Jpeg_Lcd_Open();
	//b、初始化摄像头
	linux_v4l2_yuyv_init(pathname_camera);
	//c、开启摄像头捕捉
	linux_v4l2_start_yuyv_capturing();
	return ;
}

//2、运行摄像头(600*480)
void Camera_Show(int lcd_x, int lcd_y)
{
	int ts_x, ts_y, ts_touch;
	get_ts_value(&ts_x, &ts_y, &ts_touch);
	while(1)
	{
		//a、获取摄像头捕捉的画面数据(一直读)
		linux_v4l2_get_yuyv_data(&video_buf);
		//b、显示摄像头捕捉的画面(一直写)
		Jpeg_Lcd_Show_Camera(lcd_x, lcd_y, video_buf.jpg_data , video_buf.jpg_size);
		// 触摸退出
		if (ts_touch == 1) break;
	}
}


//3、关闭摄像头
void Camera_Close(void)
{
	//a、摄像头解除初始化
	linux_v4l2_yuyv_quit();
	
	//b、关闭摄像头
	Jpeg_Lcd_Close();
	return ;
}


