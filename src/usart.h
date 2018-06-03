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

/*�����ļ����*/
int usart_fd;

/*��ʼ������*/
int usart_init();

/*��������*/
//write(usart_fd, ch, sizeof(ch));

/*�رմ���*/
int usart_close();

/*���ô��ڲ���*/
int usart_set(int fd);





/*�رմ���*/
int usart_close(){
	printf(" close usart!\n");
	return close(usart_fd);
}


/*��ʼ������*/
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
	timeout.tv_sec = 1;//���ó�ʱʱ��Ϊ1��
    timeout.tv_usec = 0;
	printf(" init usart success!\n");
    return 0;
}


/*���ô��ڲ���*/
int usart_set(int fd)
{
    struct termios termios_rfid;

    bzero(&termios_rfid, sizeof(termios_rfid));//��սṹ��

    cfmakeraw(&termios_rfid);//�����ն����ԣ�����ѡ��

    cfsetispeed(&termios_rfid, B115200);//���벨����
    cfsetospeed(&termios_rfid, B115200);//���������

    termios_rfid.c_cflag |= CLOCAL | CREAD;//�������Ӻͽ���ʹ��

    termios_rfid.c_cflag &= ~CSIZE;//�������λ
    termios_rfid.c_cflag |= CS8;//����λΪ8λ

    termios_rfid.c_cflag &= ~PARENB;//����żУ��

    termios_rfid.c_cflag &= ~CSTOPB;//һλֹͣλ

    tcflush(fd,TCIFLUSH);

    termios_rfid.c_cc[VTIME] = 10;//���õȴ�ʱ��
    termios_rfid.c_cc[VMIN] = 1;

    tcflush(fd, TCIFLUSH);//������뻺����

    if(tcsetattr(fd, TCSANOW, &termios_rfid)<0)//���������
        return -1;

    return 0;
}



#endif