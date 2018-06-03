#ifndef __USART_H
#define __USART_H

#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <termios.h>
#include <sys/types.h>
#include <sys/select.h>
#include <stdlib.h>
#include <unistd.h>
#include <netdb.h>
#include <string.h>
#include <pthread.h>
#include <errno.h>
#include <time.h>

/*串口文件句柄*/
int usart_fd;

/*初始化串口*/
int usart_init();

/*发送数据*/
//write(usart_fd, ch, sizeof(ch));

/*关闭串口*/
int usart_close();

/*设置串口参数*/
int usart_set(int fd);





/*关闭串口*/
int usart_close(){
	printf(" close usart!\n");
	return close(usart_fd);
}


/*初始化串口*/
int usart_init(){
	
	usart_fd = open("/dev/ttyUSB0", O_RDWR);
	if(usart_fd < 0){
		printf(" open /dev/ttyUSB0 failed!\n");
		return -1;
	}
	if(usart_set(usart_fd) == -1){
		printf(" set usart parameter failed!\n");
		return -1;
	}
	struct timeval timeout;
	timeout.tv_sec = 1;//设置超时时间为1秒
    timeout.tv_usec = 0;
	printf(" init usart success!\n");
    return 0;
}


/*设置串口参数*/
int usart_set(int fd)
{
    struct termios termios_rfid;

    bzero(&termios_rfid, sizeof(termios_rfid));//清空结构体

    cfmakeraw(&termios_rfid);//设置终端属性，激活选项

    cfsetispeed(&termios_rfid, B115200);//输入波特率
    cfsetospeed(&termios_rfid, B115200);//输出波特率

    termios_rfid.c_cflag |= CLOCAL | CREAD;//本地连接和接收使能

    termios_rfid.c_cflag &= ~CSIZE;//清空数据位
    termios_rfid.c_cflag |= CS8;//数据位为8位

    termios_rfid.c_cflag &= ~PARENB;//无奇偶校验

    termios_rfid.c_cflag &= ~CSTOPB;//一位停止位

    tcflush(fd,TCIFLUSH);

    termios_rfid.c_cc[VTIME] = 10;//设置等待时间
    termios_rfid.c_cc[VMIN] = 1;

    tcflush(fd, TCIFLUSH);//清空输入缓冲区

    if(tcsetattr(fd, TCSANOW, &termios_rfid)<0)//激活串口设置
        return -1;

    return 0;
}



#endif