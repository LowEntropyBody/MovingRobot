/*
**  Author: ZhaoYang
**	Compile: g++ MovingRobot/main.cpp -lpthread -std=c++11 -lm -fpermissive -o MovingRobot.out
**  Run: ./MovingRobot.out
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

	CarSpeed* cs = new CarSpeed(); 
	cs -> move_frist_start();
	cs -> speed_x_y_z(xs, ys, zs);
	usleep(1000*3000);
	cs -> speed_x_y_z(0, 0, 0);
	
	return 0;
}