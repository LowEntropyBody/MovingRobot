/*
**  Author: ZhaoYang
**	Compile: g++ main.cpp -lpthread -o movingRobot.out
**  Run: ./movingRobot.out
**  Download: git clone https://github.com/LowEntropyBody/MovingRobot.git
**  Date: 2018/4
*/
#include "base/car.h"
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
	功能：控制硬件初始化
	参数：iscar -> 启动底盘
	      isband -> 启动蓝牙手环
		  islaser -> 启动蓝牙激光
		  ispan -> 启动云台
	返回：成功返回0，失败-1
**/
int init_hardwave(bool iscar, bool isband, bool islaser, bool ispan);

int main(int argc, char* argv[]){
	cout << "----Main Thread Start----" << endl;
	init_hardwave(true,false,false,false);
	
	cout << "----Main Thread End----" << endl;
	return 0;
}

int init_hardwave(bool iscar, bool isband, bool islaser, bool ispan){
	if(iscar){
		car = new Car();
		car -> thread_run();
		cout << "  car init success" << endl;
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



