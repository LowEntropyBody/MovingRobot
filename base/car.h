#ifndef __CAR_H
#define __CAR_H
//////////////////////////////////////
#include "usart.h"
#include <math.h>
#include <iostream>
#include <vector>
using namespace std;

//速度模式
class CarSpeed{
	public:
		CarSpeed();
		~CarSpeed();
		void move_frist_start();
		void speed_x_y_z(int x, int y, int z);
		void move(double degree, double distance, double base_speed);
		
};
void CarSpeed::move(double degree, double distance, double base_speed){
	degree = degree / 180 * 3.1415926;
	double x = - distance * sin(degree);
	double y = distance * cos(degree);
	double x_abs = x;
	double y_abs = y;
	if(x < 0) x_abs = -x;
	if(y < 0) y_abs = -y;
	if(y_abs > x_abs){
		x_abs = base_speed * x_abs / y_abs;
		y_abs = base_speed;
	}else{
		y_abs = base_speed * y_abs / x_abs;
		x_abs = base_speed;
	}
	if(x < 0) x = -x_abs; else x = x_abs;
	if(y < 0) y = -y_abs; else y = y_abs;
	cout << x << ",," << y << endl;
	speed_x_y_z(x, y, 0);
	
}
CarSpeed::CarSpeed(){
	
}
void CarSpeed::speed_x_y_z(int x, int y, int z){
	char ch[10] = {0xff,0xfe,1,0,0,0,0,0,0,0x00};
	char direction = 0x00;
	if(x < 0){
		direction += 4;
		x = -x;
	}
	if(y < 0){
		direction += 2;
		y = -y;
	}
	if(z < 0){
		direction += 1;
		z = -z;
	}
	char low = x%256;
	char high = x/256;
	ch[3] = high;
	ch[4] = low;
	low = y%256;
	high = y/256;
	ch[5] = high;
	ch[6] = low;
	low = z%256;
	high = z/256;
	ch[7] = high;
	ch[8] = low;
	ch[9] = direction;
	write(usart_fd, ch, sizeof(ch));
	tcflush(usart_fd, TCIFLUSH);//清空in缓冲区
    tcflush(usart_fd, TCOFLUSH);//清空out缓冲区
}

void CarSpeed::move_frist_start(){
	char ch[1] = {0x00};
	int re = usart_init();
	if(re < 0) exit(-1);
	write(usart_fd, ch, sizeof(ch));
	tcflush(usart_fd, TCIFLUSH);//清空in缓冲区
    tcflush(usart_fd, TCOFLUSH);//清空out缓冲区
}

//位置模式
class Car{
	double now_degree;
	public:
		Car();
		~Car();
		void move_frist_start();
		void move_rotate(double degree);
		void move_forward(double distance);
		
};

void Car::move_forward(double distance){
	int degree_temp = now_degree * 21;
	int distance_temp = distance * 63;
	int high,low;
	char ch[10] = {0xff,0xfe,2,0,0,20,0,20,0,0x07};
	
	if(degree_temp >= 0){
		ch[9] = 0x07;
		//A轮不动
		low = degree_temp%256;
		high = degree_temp/256;
		ch[3] = high;
		ch[4] = low;
		//C轮正转
		int distancec = degree_temp + distance_temp;
		low = distancec%256;
		high = distancec/256;
		ch[7] = high;
		ch[8] = low;
		//B轮反转
		int distanceb = degree_temp - distance_temp;
		if(distanceb >= 0){
			low = distanceb%256;
			high = distanceb/256;
			ch[5] = high;
			ch[6] = low;
		}else{
			distanceb = -distanceb;
			low = distanceb%256;
			high = distanceb/256;
			ch[5] = high;
			ch[6] = low;
			ch[9] = 0x05;
		}
	}else{
		degree_temp = -degree_temp;
		ch[9] = 0x00;
		//A轮不动
		low = degree_temp%256;
		high = degree_temp/256;
		ch[3] = high;
		ch[4] = low;
		
		//B轮反转
		int distanceb = degree_temp + distance_temp;
		low = distanceb%256;
		high = distanceb/256;
		ch[5] = high;
		ch[6] = low;
		
		//C轮正转
		int distancec = degree_temp - distance_temp;
		if(distancec >= 0){
			low = distancec%256;
			high = distancec/256;
			ch[7] = high;
			ch[8] = low;
		}else{
			distancec = -distancec;
			low = distancec%256;
			high = distancec/256;
			ch[7] = high;
			ch[8] = low;
			ch[9] = 0x01;
		}
		
	}	
	write(usart_fd, ch, sizeof(ch));
	tcflush(usart_fd, TCIFLUSH);//清空in缓冲区
    tcflush(usart_fd, TCOFLUSH);//清空out缓冲区
	
}

void Car::move_rotate(double degree){
	now_degree = degree;
	int degree_temp = degree * 21;
	int high,low;
	char ch[10] = {0xff,0xfe,2,20,0,20,0,20,0,0x07};
	if(degree_temp >= 0){
		low = degree_temp%256;
		high = degree_temp/256;
		ch[3] = ch[5] = ch[7] = high;
		ch[4] = ch[6] = ch[8] = low;
	}else{
		degree_temp = -degree_temp;
		low = degree_temp%256;
		high = degree_temp/256;
		ch[3] = ch[5] = ch[7] = high;
		ch[4] = ch[6] = ch[8] = low;
		ch[9] = 0x00;
	}
	write(usart_fd, ch, sizeof(ch));
	tcflush(usart_fd, TCIFLUSH);//清空in缓冲区
    tcflush(usart_fd, TCOFLUSH);//清空out缓冲区
}

void Car::move_frist_start(){
	char ch[1] = {0x00};
	int re = usart_init();
	if(re < 0) exit(-1);
	write(usart_fd, ch, sizeof(ch));
	tcflush(usart_fd, TCIFLUSH);//清空in缓冲区
    tcflush(usart_fd, TCOFLUSH);//清空out缓冲区
}

Car::Car(){
	now_degree = 0;
}
Car::~Car(){
	usart_close();
}

#endif