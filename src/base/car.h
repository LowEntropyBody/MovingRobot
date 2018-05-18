#ifndef __CAR_H
#define __CAR_H
//////////////////////////////////////
#include "usart.h"
#include <math.h>
#include <iostream>
#include <vector>
#include <thread>
#include <unistd.h>
#include <mutex>
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
	//char ch[10] = {0xff,0xfe,0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x00};
	char ch[10] = {0,0,0,0,0,0,0,0,0,0};
	ch[0] = 0xff;
	ch[1] = 0xfe;
	ch[2] = 0x01;

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


 

class Car{
	private:
		CarSpeed* cs;
		thread* t;
		void run();
		bool run_flag;
		bool car_stop_flag;
		int x_time;
		int y_time;
		int z_time;
		int x_speed;
		int y_speed;
		int z_speed;
		bool x_is_stop;
		bool y_is_stop;
		bool z_is_stop;
		mutex car_mx;
	public:
		Car();
		~Car();
		void thread_run();
		void thread_end();
		void order_car(int x, int x_time, int y, int y_time, int z, int z_time);
};

void Car::order_car(int x, int x_time, int y, int y_time, int z, int z_time){
	lock_guard<mutex> guard(car_mx);
	x_speed = x;
	y_speed = y;
	z_speed = z;
	this -> x_time = x_time;
	this -> y_time = y_time;
	this -> z_time = z_time;
	if(x != 0) x_is_stop = false;
	if(y != 0) y_is_stop = false;
	if(z != 0) z_is_stop = false;
	cout << "car change moving ......" << endl;
	cs -> speed_x_y_z(x,y,z);
}

void Car::run(){
	cpu_set_t mask_k;
    cpu_set_t get_k;
    int num = sysconf(_SC_NPROCESSORS_CONF);
	int cpu_k = 3;
	CPU_ZERO(&mask_k);
    CPU_SET(cpu_k, &mask_k);
    if (pthread_setaffinity_np(pthread_self(), sizeof(mask_k), &mask_k) < 0) {
    	std::cout << "Fails to set the CPU to run the car thread" << std::endl;
    }
    CPU_ZERO(&get_k);
    if (pthread_getaffinity_np(pthread_self(), sizeof(get_k), &get_k) < 0) {
       std::cout << "Fails to get the CPU of the car thread" << std::endl;
    }
    for (int k = 0; k < num; k++) {
        if (CPU_ISSET(k, &get_k)) {
            std::cout << "the car thread " << (int)pthread_self() <<" is running in CPU " << k << std::endl;
        }
    }
	while(run_flag){
		if(x_time == 0){
			if(!x_is_stop){
				lock_guard<mutex> guard(car_mx);
				x_is_stop = true;
				cs -> speed_x_y_z(0, y_speed, z_speed);
				//cout << "stop car x" << endl;
			}
		}else{
			lock_guard<mutex> guard(car_mx);
			x_time--;
		}
		
		if(y_time == 0){
			if(!y_is_stop){
				lock_guard<mutex> guard(car_mx);
				y_is_stop = true;
				cs -> speed_x_y_z(x_speed, 0, z_speed);
				//cout << "stop car y" << endl;
			}
		}else{
			lock_guard<mutex> guard(car_mx);
			y_time--;
		} 
		
		if(z_time == 0){
			if(!z_is_stop){
				lock_guard<mutex> guard(car_mx);
				z_is_stop = true;
				cs -> speed_x_y_z(x_speed, y_speed, 0);
				//cout << "stop car z" << endl;
			}
		}else{
			lock_guard<mutex> guard(car_mx);
			z_time--;
		} 
		usleep(1000*50);
	}
}

Car::Car(){
	run_flag = true;
	x_time = 0;
	y_time = 0;
	z_time = 0;
	x_speed = 0;
	y_speed = 0;
	z_speed = 0;
	x_is_stop = true;
	y_is_stop = true;
	z_is_stop = true;
	cs = new CarSpeed();
	cs -> first_start();
	cs -> move_frist_start();
}
Car::~Car(){}
void Car::thread_run(){
	t = new thread(&Car::run, this);
}
void Car::thread_end(){
	run_flag = false;
	t->join();
}



#endif
