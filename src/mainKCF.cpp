#include <iostream>  
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <time.h>
#include <pthread.h>
#include <unistd.h>
#include "stdio.h"
#include "stdlib.h"
#include "ImgTrackerKCF.hpp"
#include "pcomm.h"
#include "car.h"

#define FPS_CAL_GROUP 10
#define FRAME_W 320
#define FRAME_H 240
#define MS 1000
#define VIEW_ANGLE 80
#define IMG_NUM 30

using namespace std;
using namespace cv;



/**
	全局变量，用于控制底盘
**/
Car* car = NULL;
/**
	功能：控制硬件初始化
	参数：iscar -> 启动底盘
		  iskcf -> 启动识别
	      isband -> 启动蓝牙手环
		  islaser -> 启动蓝牙激光
	返回：成功返回0，失败-1
**/

int init_hardware(bool iscar, bool iskcf, bool isband, bool islaser);

// yidong
void move_fun();

const string m_text_name = "mtext.txt";
const string m_text_path = "";
Pcomm pcomm_m(m_text_path, m_text_name);

const string text_name = "text.txt";
const string text_path = "";
Pcomm pcomm(text_path, text_name);




////////////////////////////////////////////////////
bool systemExit = false; 
ImgTrackerKCF imgTrackerKCF; 
cv::Rect result, target; ;
cv::Mat imgFrame;
pid_t pid_video_main;
pid_t pid_read;
VideoCapture capture;
const String img_path = "img/";
const String img_format = ".jpg"; 



void initTarget(cv::Rect &target, float left, float width, float up, float height){
	target.x = (int)imgFrame.cols * left; 
	target.width = (int)imgFrame.cols * width;
	target.y = (int)imgFrame.rows * up; 
	target.height = (int)imgFrame.rows * height;
}

void imgRead(){

	float fps_r = 0.0;
	int read_index = 1;
	cv::Mat img_read;
	int write_index = 1;
	string write_str;

	while (!systemExit) {

		stringstream str_r;

		// read
		capture >> img_read;

		// write the image
		str_r << img_path << write_index << img_format;
		imwrite(str_r.str(), img_read);

		// write the file
		std::stringstream ss;
		ss << write_index;
		ss >> write_str;
		pcomm.pwrite_s(write_str);

		// // test
		// printf("READ index = %d, ind = %d\n", read_index++, write_index);
			
		if(++write_index == IMG_NUM) write_index = 1;

	}

	return;
}


// void imgKCF(){

	// int ind_r = 1;
	// int ind_k = 1;
	// int img_index = 0; 
	// float fps_k = 0.0;
	// string write_str;
	// clock_t t_start=clock(), t_end, t_diff;
	// float cx, cy;
	// float cx_r=0.5, cy_r=0.5;
	// bool isFind = true;

	// float peak_last = 0.0;

	// // test
	// // namedWindow("img", WINDOW_AUTOSIZE);
	// waitKey(2000);

	// while (!systemExit) {

	// 	// read
	// 	stringstream str_k;
	// 	vector<string> outputs;
	// 	pcomm.pread_s(write_str);
	// 	ind_r = atoi(write_str.c_str());

	// 	if(ind_r > 3){
	// 		ind_k = ind_r-3;
	// 	}else if(ind_r == 3){
	// 		ind_k = IMG_NUM-1;
	// 	}else if(ind_r == 2){
	// 		ind_k = IMG_NUM-2;
	// 	}else{
	// 		ind_k = IMG_NUM-3;
	// 	}

	// 	str_k << img_path << ind_k << img_format;
	// 	imgFrame = imread(str_k.str());	

	// 	// update	
	// 	result = imgTrackerKCF.update(imgFrame);

	// 	// test
	// 	// rectangle(imgFrame, Point(result.x, result.y), Point(result.x + result.width, result.y + result.height), Scalar(0, 255, 255), 1, 8);
	// 	// imshow("img", imgFrame); 
	// 	// waitKey(1);

	// 	// fps
	// 	if((++img_index)% 10 == 0){
	// 		t_end = clock(); 
	// 		t_diff = t_end - t_start;
	// 		fps_k = CLOCKS_PER_SEC * 10.0 / (float)t_diff;
	// 		printf("KCF frame: %d, fps: %f\n", img_index, fps_k);
	// 		t_start = clock();
	// 	}


	// 	// calculates the output
	// 	cx = result.x + result.width/2.0;
	// 	cy = result.y + result.height/2.0;
	// 	cx_r = (float)cx / (float)FRAME_W;
	// 	cy_r = (float)cy / (float)FRAME_H;
	// 	if(imgTrackerKCF.peak_value < peak_last*0.85 || imgTrackerKCF.peak_value < 0.25 || cx_r<0.10||cx_r>0.90||cy_r<0.10||cy_r>0.90 || ((float)result.width) > (target.width*1.5)) {isFind = false;}
	// 	else {isFind = true;}

	// 	peak_last = imgTrackerKCF.peak_value;

	// 	// writes the output into file
	// 	outputs.push_back(to_string(isFind));
	// 	outputs.push_back(to_string(img_index));
	// 	outputs.push_back(to_string(result.width));
	// 	outputs.push_back(to_string(cx));
	// 	pcomm_m.pwrite(outputs);

	// }
	//return;
//}

void imgKCF(){
	int img_index = 0; 
	float fps_k = 0.0;
	clock_t t_start=clock(), t_end, t_diff;
	float cx, cy;
	float cx_r=0.5, cy_r=0.5;
	bool isFind = true;
	vector<string> outputs;

	float peak_last = 0.0;

	while (!systemExit) {
		capture >> imgFrame;
		result = imgTrackerKCF.update(imgFrame);
		if((++img_index)% 10 == 0){
			t_end = clock(); 
			t_diff = t_end - t_start;
			fps_k = CLOCKS_PER_SEC * 10.0 / (float)t_diff;
			printf("KCF frame: %d, fps: %f\n", img_index, fps_k);
			t_start = clock();
		}


		// calculates the output
		cx = result.x + result.width/2.0;
		cy = result.y + result.height/2.0;
		cx_r = (float)cx / (float)FRAME_W;
		cy_r = (float)cy / (float)FRAME_H;
		if(imgTrackerKCF.peak_value < peak_last*0.85 || imgTrackerKCF.peak_value < 0.25 || cx_r<0.10||cx_r>0.90||cy_r<0.10||cy_r>0.90 || ((float)result.width) > (target.width*1.5)) {isFind = false;}
		else {isFind = true;}

		peak_last = imgTrackerKCF.peak_value;

		// writes the output into file
		outputs.push_back(to_string(isFind));
		outputs.push_back(to_string(img_index));
		outputs.push_back(to_string(result.width));
		outputs.push_back(to_string(cx));
		pcomm_m.pwrite(outputs);
	}

	return;
}

/////////////////////////////////////////

int main(int argc, char* argv[]) {


	cout << "----23123Main Thread Start----" << endl;
	init_hardware(true, true, false, false);
	usleep(2*MS*MS);
	cout << "----receive start sgn----" << endl;

	capture.open(2);
	if (!capture.isOpened()){ printf("ERROR: Fails to open the camera!\n");}
	capture.set(CV_CAP_PROP_FRAME_WIDTH, FRAME_W);  
    capture.set(CV_CAP_PROP_FRAME_HEIGHT, FRAME_H);   
	capture >> imgFrame;

	// 目标框初始化(128*120)
	initTarget(target, 0.3, 0.4, 0.25, 0.5);
	imgTrackerKCF.init(target, imgFrame);

	 pid_video_main = fork();
	 if(pid_video_main == -1) {std::cerr << "ERROR: Fails to start the new process!" << std::endl; exit(EXIT_FAILURE);}
	 else if(pid_video_main == 0)
	 {

		//识别主进程开始//
		// pid_read = fork();
		// if(pid_read == -1) {std::cerr << "ERROR: Fails to start the new process!" << std::endl; exit(EXIT_FAILURE);}
		// else if(pid_read == 0){

		// 	//read进程开始//
		// 	imgRead();
		// 	return EXIT_SUCCESS;
		// 	//read进程结束//

		// }else{
		// 	//kcf进程开始//
		// 	imgKCF();
		// 	// test
		// 	// while(true);
		// 	return EXIT_SUCCESS;
		// 	//kcf进程结束//
		// }
		// return EXIT_SUCCESS;
		// //识别主进程结束//

		imgKCF();
	
	 }
	 else{
	 //--------------------main----------------------//
		move_fun();
		car -> thread_end();
	 	cout << "----Main Thread End----" << endl;
	 	return EXIT_SUCCESS;
	 }
	
	return 0;
}



int init_hardware(bool iscar, bool iskcf, bool isband, bool islaser){
	if(iscar){
		car = new Car();
		car -> thread_run();
		cout << "  car init success" << endl;
	}
	if(iskcf){
		
		cout << "  kcf init success" << endl;
	}
	if(isband){
		
		cout << "  isband init success" << endl;
	}
	if(islaser){
		
		cout << "  islaser init success" << endl;
	}
	return 0;
}


// yidong
void move_fun(){

	float angle = 0.;
	
	std::vector<string> outs;
	int now_index = -1;
	while(1){
		
		pcomm_m.pread(outs);
		if(outs.size() == 4){
			if(atoi(outs[1].c_str()) != now_index){

				now_index = atoi(outs[1].c_str());
				if(atoi(outs[0].c_str()) == 1){
					printf("found\n");

					//printf("%s\n", outs[2].c_str());
					//printf("%s\n", outs[3].c_str());


					angle = (float)atoi(outs[3].c_str()) / FRAME_W * VIEW_ANGLE - (float)VIEW_ANGLE / 2.0;
					cout << "angle = " << angle <<endl;
					if(angle < -5|| angle > 5){
						if(angle < 0){
							angle = - angle;
							car -> order_car(0,0,0,0,-50,(int)angle/1.65);
						}else{
							car -> order_car(0,0,0,0,50,(int)angle/1.65);
						}

					}else{
						printf("in mid\n");
						car -> order_car(0,0,0,0,0,0);
					}

				}else{
					printf("not found\n");
					car -> order_car(0,0,0,0,0,0);
					break;
				}
			}
		}
		waitKey(10);
	}
	
}
