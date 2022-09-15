#include "main.h"
#include <pthread.h>

// 根据设计文件得到的宽高参数，以及替代goto Label这种高风险coding行为的解决方案
#define CAM_X 160
#define CAM_Y 120
#define TXT_X 160
#define TXT_Y 376
#define TXT_W 320
#define TXT_H 24
#define ERR -1

#define BG 0x00202020
// 设置参数
#define MANUAL 0
#define AUTO 1
// 标志位参数
// 1正常运行，2、3、4……自定义为其他状态，-1883为退出，部分功能在程序启动时默认暂停
#define SIGPAUSE 0
#define SIGRUN 1
#define SIGEXIT -1883

// 全局变量区 //
int ts_x, ts_y, ts_touch;
int SIGCAM = SIGPAUSE;
int SIGTOUCH = SIGRUN;
int SIGRFID = SIGPAUSE;
int shot = 0;
// 初始化界面状态机
int stateUI = 0; // 0为主页，1为更多功能按钮页面，2为相册
// RFID
int rfidData;
// 为一般难度做准备，存储图库
int *album[256];
int photoCount = 0;
// 为一般难度做准备
int deliverMode = MANUAL;

unsigned int *load_bmp(char *path, int bmp_w, int bmp_h)
{
	/**
	 * 将原本的show_bmp拆分为加载和绘制两个阶段，最终加载返回的是lcd缓存。
	 * 在子函数内malloc并返回指针地址的可行性有待考究。
	 **/
	/* 一、获取bmp图片的颜色数据(BGR) */
	// 1、打开bmp图片文件(以只读的形式打开)
	int bmp_fd = open(path, O_RDONLY);
	if (-1 == bmp_fd)
	{
		perror("open bmp error!\n");
		return NULL;
	}

	// 2、获取bmp文件大小，跳过54个字节的头信息数据
	// a、获取bmp文件大小
	off_t bmp_filesize = lseek(bmp_fd, 0, SEEK_END);
	if ((off_t)-1 == bmp_filesize)
	{
		perror("count bmp_filesize error!\n");
		return NULL;
	}
	else
	{
		printf("bmp_filesize = %ld\n", bmp_filesize);
	}

	// b、跳过54个字节的头信息数据
	off_t ret_l = lseek(bmp_fd, 54, SEEK_SET);
	if ((off_t)-1 == ret_l)
	{
		perror("lseek bmp error!\n");
		return NULL;
	}

	// 3、读取bmp图的颜色数据(BGR)
	// unsigned char bmp_buf[bmp_w*bmp_h*3];
	unsigned char *bmp_buf = malloc(bmp_w * bmp_h * 3);
	if (NULL == bmp_buf)
	{
		perror("malloc bmp space error!\n");
		return NULL;
	}

	ssize_t ret_r = read(bmp_fd, bmp_buf, bmp_w * bmp_h * 3);
	if (-1 == ret_r)
	{
		perror("read bmp error!\n");
		return NULL;
	}
	else
	{
		printf("读取到的数据的大小为:%ld\n", ret_r);
	}

	// 4、关闭bmp图片文件
	int ret_c = close(bmp_fd);
	if (-1 == ret_c)
	{
		perror("close bmp error!\n");
		return NULL;
	}

	/* 二、将bmp图片的颜色数据(BGR)转换成lcd显示屏数据(ARGB)*/
	// unsigned int lcd_buf[800*480] = {0};
	unsigned int *lcd_buf = malloc(bmp_w * bmp_h * 4);
	if (NULL == lcd_buf)
	{
		perror("malloc lcd space error!\n");
		return NULL;
	}

	unsigned int i;
	for (i = 0; i < bmp_w * bmp_h; i++)
	{
		*(lcd_buf + i) = ((*(bmp_buf + 3 * i)) << 0) + ((*(bmp_buf + 3 * i + 1)) << 8) + ((*(bmp_buf + 3 * i + 2)) << 16) + (0x00 << 24);
		// ARGB              B                    G                   R                 A
	}

	free(bmp_buf);
	return lcd_buf;
}

void draw(unsigned int *resource, int lcd_x, int lcd_y, int bmp_w, int bmp_h)
{
	// 每调用一次draw都会开关一次lcd，但是我不想改太多，就这样吧。
	// 个人认为接触到底层的核心函数应当是互信机制的，外部需要确保传入到draw的参数必然正确，
	// 而draw内部不应当花费太多精力去做错误校验。
	int lcd_fd = open("/dev/fb0", O_RDWR);
	if (-1 == lcd_fd)
	{
		perror("open lcd error!\n");
		return;
	}

	// 2、将lcd显示屏数据(ARGB)写到lcd显示屏上
	// A、使用write函数编写
	/*
		ssize_t ret_w = write(lcd_fd, lcd_buf, sizeof(lcd_buf));
		if (-1 == ret_w)
		{
			perror("write lcd error!\n");
			goto Label;
		}
	*/
	// 2、使用mmap函数编写
	unsigned int *lcd_p = mmap(
		NULL,					//内核会自行帮你设置映射空间的首地址
		800 * 480 * 4,			//映射空间的大小
		PROT_READ | PROT_WRITE, //映射空间的操作许可
		MAP_SHARED,				//该映射空间可被其它进程所访问
		lcd_fd,					//显示屏lcd的文件描述符
		0						//一开始不进行位移
	);

	if ((void *)-1 == lcd_p)
	{
		perror("mmap lcd error!\n");
		return;
	}
	unsigned int *lcd_temp_p = lcd_p; //记录映射空间的首地址

	// 2a、指定任意位置显示
	lcd_p = lcd_p + lcd_x;		 //指定x轴的位置
	lcd_p = lcd_p + lcd_y * 800; //指定y轴的位置

	// 2b、任意大小显示颜色
	int x, y;
	for (y = 0; y < bmp_h; y++)
	{
		for (x = 0; x < bmp_w; x++)
		{
			*(lcd_p + (bmp_h - y - 1) * 800 + x) = *(resource + y * bmp_w + x);
		}
	}

	// 3、解除映射、关闭lcd显示屏文件、释放申请的内存
	// a、解除映射
	int ret_m = munmap(lcd_temp_p, 800 * 480 * 4);
	if (-1 == ret_m)
	{
		perror("munmap lcd error!\n");
		return;
	}

	// b、关闭lcd显示屏文件
	int ret_c = close(lcd_fd);
	if (-1 == ret_c)
	{
		perror("close lcd error!\n");
		return;
	}

	return;
}

int msleep(long msec)
{
	struct timespec ts;
	int res;

	if (msec < 0)
	{
		errno = EINVAL;
		return -1;
	}

	ts.tv_sec = msec / 1000;
	ts.tv_nsec = (msec % 1000) * 1000000;

	do
	{
		res = nanosleep(&ts, &ts);
	} while (res && errno == EINTR);

	return res;
}
/*
void launchCamera(char *device)
{
	// 1、打开摄像头
	Camera_Open(device);
	// 2、运行摄像头
	Camera_Show(CAM_X, CAM_Y);
	// 3、关闭摄像头
	Camera_Close();
}
*/
void clearDisplay(int color)
{
	// show_color(0,0,800,480, color);
}

void *cameraHandler(void *device)
{
	char *dev = (char *)device;
	printf("[cameraHandler] Ready to open camera: %s\n", dev);
	printf("SIGCAM = %d\n", SIGCAM);
	printf("shot = %d\n", shot);
	while (1)
	{
		if (SIGCAM == SIGRUN)
		{
			// Code
		}
		else if (SIGCAM == SIGPAUSE)
		{
			printf("[cameraHandler] Pausing.\n");
			sleep(1);
			continue;
		}

		if (SIGCAM == SIGEXIT)
		{
			printf("[cameraHandler] Got SIGEXIT, Exiting.\n");
			break;
		}
	}
	pthread_exit(0);
}

void *rfidHandler(void *device)
{
	char *dev = (char *)device;
	printf("[fridHandler] Ready to open serial: %s\n", dev);
	printf("SIGRFID = %d\n", SIGRFID);
	while (1)
	{
		if (SIGRFID == SIGRUN)
		{
			/* code */
		}
		else if (SIGRFID == SIGPAUSE)
		{
			printf("[rfidHandler] Pausing.\n");
			sleep(1);
			continue;
		}
		if (SIGRFID == SIGEXIT)
		{
			printf("[rfidHandler] Got SIGEXIT, Exiting.\n");
			break;
		}
	}
	pthread_exit(0);
}

void *touchHandler(void *device)
{
	char *dev = (char *)device;
	printf("[touchHandler] Ready to open touchscreen: %s \n", dev);
	printf("[touchHandler] Note that arg \"device\" is fake for test only, it won't be passed into any function.\n");
	printf("SIGTOUCH = %d\n", SIGTOUCH);
	while (1)
	{
		if (SIGTOUCH == SIGRUN)
		{
			// get_ts_value(&ts_x, &ts_y, &ts_touch);
		}
		else if (SIGTOUCH == SIGPAUSE)
		{
			printf("[touchHandler] Pausing.\n");
			sleep(1);
			continue;
		}
		if (SIGTOUCH == SIGEXIT)
		{
			printf("[touchHandler] Got SIGEXIT, Exiting.\n");
			break;
		}
	}
	pthread_exit(0);
}

int main(int argc, char const *argv[])
{
	printf("Hello, World! %d\n", ts_touch);
	// 初始化多线程
	pthread_t tRFID, tCam, tTouch;
	int result;

	result = pthread_create(&tTouch, NULL, touchHandler, "/dev/input/event0");
	if (result != 0)
	{
		printf("Thread tTouch failed.\n");
		return ERR;
	}
	result = pthread_create(&tRFID, NULL, rfidHandler, "/"); // 在此处填写tty的位置
	if (result != 0)
	{
		printf("Thread tRFID failed.\n");
		return ERR;
	}

	result = pthread_create(&tCam, NULL, cameraHandler, "/"); // 在此处填写camera的位置
	if (result != 0)
	{
		printf("Thread tCam failed.\n");
		return ERR;
	}
	// 阻塞使得线程执行
	sleep(1);
	printf("[Main] Check.\n");

	// 状状状状状状状态机！
	switch (stateUI)
	{
	case 0:
		clearDisplay(BG);
		// Code
		break;
	case 1:
		clearDisplay(BG);
		// Code
		break;
	default:
		printf("[MainUI] Got unavailable state id, Exiting.\n");
		break;
	}

	// 暂停测试
	// SIGCAM = SIGTOUCH = SIGRFID = SIGPAUSE;
	// for (int i = 10; i > 0; i--)
	// {
	// 	// 直接sleep(10)会导致代码阻塞，子线程无法正常输出
	// 	sleep(1);
	// }

	// 结束线程准备退出
	SIGCAM = SIGTOUCH = SIGRFID = SIGEXIT;
	pthread_join(tCam, NULL);
	pthread_join(tRFID, NULL);
	pthread_join(tTouch, NULL);

	return 0;
}