#include "main.h"

//1、显示颜色(任意位置大小显示任意颜色)
int show_color(int lcd_x, int lcd_y, int color_w, int color_h, int color)
{
	/* 一、打开lcd显示屏文件(以读写形式打开) */
	int lcd_fd = open("/dev/fb0", O_RDWR);
	if (-1 == lcd_fd)
	{
		perror("open lcd error!\n");
		goto Label;
	}

	/* 二、向lcd显示屏文件写颜色数据(ARGB) */
	// 1、使用write函数编写
	/*
		unsigned int lcd_buf[800*480] = {0}; 
		unsigned int i;
		for (i = 0; i < 800*480; i++)
		{
			lcd_buf[i] = 0x0000ff00;	//绿色
		}

		ssize_t ret_w = write(lcd_fd, lcd_buf, sizeof(lcd_buf));
		if (-1 == ret_w)
		{
			perror("write lcd error!\n");
			goto Label;
		}
		else
		{
			printf("写入的数据大小为%ld\n",  ret_w);
		}
	*/
	// 2、使用mmap函数编写
	unsigned int *lcd_p = mmap(
									NULL,					//内核会自行帮你设置映射空间的首地址
									800*480*4,				//映射空间的大小
									PROT_READ|PROT_WRITE,	//映射空间的操作许可
									MAP_SHARED,				//该映射空间可被其它进程所访问
									lcd_fd,					//显示屏lcd的文件描述符
									0						//一开始不进行位移
							  );

	if ((void *)-1 == lcd_p)
	{
		perror("mmap lcd error!\n");
		goto Label;
	}
	unsigned int *lcd_temp_p = lcd_p;						//记录映射空间的首地址


	//2a、指定任意位置显示
	lcd_p = lcd_p + lcd_x;		//指定x轴的位置
	lcd_p = lcd_p + lcd_y*800;	//指定y轴的位置

	//2b、任意大小显示颜色
	int x, y;
	for(y=0; y<color_h; y++)
	{
		for(x=0; x<color_w; x++)
		{
			*(lcd_p+y*800+x) = color;
		}
	}

	/* 三、解除映射、关闭lcd显示屏文件 */
	//1、解除映射
	int ret_m = munmap(lcd_temp_p, 800*480*4);
	if (-1 == ret_m)
	{
		perror("munmap lcd error!\n");
		goto Label;
	}

	//2、关闭lcd显示屏文件
	int ret_c = close(lcd_fd);
	if (-1 == ret_c)
	{
		perror("close lcd error!\n");
		goto Label;
	}
	return 0;

Label:
	return -1;
}