/*
**  Author: ZhaoYang
**	Compile: g++ main.cpp -lpthread -o movingRobot.out
**  Run: ./movingRobot.out
**  Download: git clone https://github.com/LowEntropyBody/MovingRobot.git
**  Date: 2018/4
*/
#include "base/car.h"
#include "base/kcf/ImgKCF.h"
#include <math.h>
#include <iostream>
#include <vector>
#include <sstream>
#include <string>
using namespace std;

/**
	全局变量，用于控制底盘
**/
Car* car = NULL;
/**
	全局变量，用于识别
**/
ImgKCF* kcf = NULL;

/**
	功能：控制硬件初始化
	参数：iscar -> 启动底盘
		  iskcf -> 启动识别
	      isband -> 启动蓝牙手环
		  islaser -> 启动蓝牙激光
		  ispan -> 启动云台
	返回：成功返回0，失败-1
**/
int init_hardware(bool iscar, bool iskcf, bool isband, bool islaser, bool ispan);

int main(int argc, char* argv[]){
	{
		cpu_set_t mask_k;
		cpu_set_t get_k;
		int num = sysconf(_SC_NPROCESSORS_CONF);
		int cpu_k = 0;
		CPU_ZERO(&mask_k);
		CPU_SET(cpu_k, &mask_k);
		if (pthread_setaffinity_np(pthread_self(), sizeof(mask_k), &mask_k) < 0) {
			std::cout << "Fails to set the CPU to run the main thread" << std::endl;
		}
		CPU_ZERO(&get_k);
		if (pthread_getaffinity_np(pthread_self(), sizeof(get_k), &get_k) < 0) {
			std::cout << "Fails to get the CPU of the main thread" << std::endl;
		}
		for (int k = 0; k < num; k++) {
			if (CPU_ISSET(k, &get_k)) {
				std::cout << "the main thread " << (int)pthread_self() <<" is running in CPU " << k << std::endl;
			}
		}
	}
	cout << "----Main Thread Start----" << endl;
	init_hardware(true, false, false, false, false);
	
	/**
	kcf -> start_init();
	cout << "  kcf model start init" << endl;
	while(!kcf -> finish_init()){usleep(1000*100);}
	cout << "  kcf model finish init" << endl;
	while(1){
		if(kcf -> is_get_target_data()){
		 	Targetdata td = kcf -> get_target_data();
		 	td.selfpt();
		 	car -> order_car(100,5,11,7,672,22);
		 }
		usleep(1000);
	}
	
	kcf -> thread_end();**/
	car -> order_car(0,0,0,0,50,20);
	car -> thread_end();
	cout << "----Main Thread End----" << endl;
	return 0;
}

int init_hardware(bool iscar, bool iskcf, bool isband, bool islaser, bool ispan){
	if(iscar){
		car = new Car();
		car -> thread_run();
		cout << "  car init success" << endl;
	}
	if(iskcf){
		kcf = new ImgKCF();
		kcf -> thread_run();
		cout << "  kcf init success" << endl;
	}
	if(isband){
		
		cout << "  isband init success" << endl;
	}
	if(islaser){
		
		cout << "  islaser init success" << endl;
	}
	if(ispan){
		
		cout << "  ispan init success" << endl;
	}
	return 0;
}



