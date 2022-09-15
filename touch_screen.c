#include "main.h"

//1、获取触摸屏的值(坐标值、触摸值)
int get_ts_value(int *ts_x, int *ts_y, int *ts_touch)
{
	//1、打开触摸屏文件(以只读形式打开)
	int ts_fd = open("/dev/input/event0", O_RDONLY);
	if (-1 == ts_fd)
	{
		perror("open ts error!\n");
		goto Label;
	}

	//2、读取触摸屏的坐标值、触摸值
	struct input_event ts_buf;

	while(1)
	{
		bzero( &ts_buf, sizeof(ts_buf));		//清空ts_buf数组
		read(ts_fd, &ts_buf, sizeof(ts_buf));	//读取触摸屏的坐标值、触摸值(如果没人摸它，就一直堵塞于此)

		//获取x轴的坐标值
		if(ts_buf.type == EV_ABS && ts_buf.code == ABS_X)
		{
			*ts_x = ts_buf.value/1024.0*800;	//黑色边沿的开发板(x:0-1024, y:0-600)
			//*ts_x = ts_buf.value;				//蓝色边沿的开发板(x:0-800,  y:0-480)
		}	
		//获取y轴的坐标值
		if(ts_buf.type == EV_ABS && ts_buf.code == ABS_Y)
		{
			*ts_y = ts_buf.value/600.0*480;		//黑色边沿的开发板(x:0-1024, y:0-600)
			//*ts_y = ts_buf.value;				//蓝色边沿的开发板(x:0-800,  y:0-480)
		}	
		//获取触摸值
		if(ts_buf.type == EV_KEY && ts_buf.code == BTN_TOUCH)
		{
			*ts_touch = ts_buf.value;
			break;
		}	
	}
	printf("(%d, %d, %d)\n", *ts_x, *ts_y, *ts_touch);

	//3、关闭触摸屏文件
	int ret_c = close(ts_fd);
	if (-1 == ret_c)
	{
		perror("close ts error!\n");
		goto Label;
	}
	return 0;

Label:
	return -1;
}