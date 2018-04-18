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

int main(int argc, char* argv[]){
	int xs,ys,zs;
	{
		stringstream ss;
		ss << argv[1];
		ss >> xs;
	}
	{
		stringstream ss;
		ss << argv[2];
		ss >> ys;
	}
	{
		stringstream ss;
		ss << argv[3];
		ss >> zs;
	}
	usleep(1000*1000*20);
	CarSpeed* cs = new CarSpeed(); 
	cs -> move_frist_start();
	for(int i=0; i< 75; i++){
		cs -> speed_x_y_z(xs, ys, zs);
		usleep(1000*2000);
		cs -> speed_x_y_z(xs, -ys, zs);
		usleep(1000*2000);
	}
	cs -> speed_x_y_z(0, 0, 0);
	
	return 0;
}