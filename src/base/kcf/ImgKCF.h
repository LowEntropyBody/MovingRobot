#ifndef __IMG_KCF_H
#define __IMG_KCF_H
//////////////////////////////////////
#include <iostream>  
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <time.h>
#include "ImgTrackerKCF.hpp"
#include <pthread.h>
#include <thread>
#include <sched.h>
#include <unistd.h>
#include <mutex>
using namespace std;
using namespace cv;


class Targetdata{
	public:
		bool isTracked;
		float angle;
		float distance;
		long long index;
		float fps;
		void selfpt(){
			cout << "  isTracked:" << isTracked << endl;
			cout << "  distance:" << distance << endl;
			cout << "  angle:" << angle << endl;
			cout << "  fps:" << fps << endl;
			cout << "  index:" << index << endl;
		}
};

class ImgKCF{
	private:
		cv::Mat imgFrame; // 摄像头采集到的图片
		VideoCapture capture;
		bool trackerWork;
		bool run_flag;
		int img_index;
		Rect target; // 初始化和计算得到的的目标区域;
		Rect result;
		clock_t t_start;
		clock_t t_end;
		clock_t t_diff;
		ImgTrackerKCF imgTrackerKCF; // 跟踪类
		float fps;
		thread* t;
		void run();
		
		bool start_init_flag; // 模板初始化
		bool finish_init_flag; // 模板初始化结束
		bool is_result_update;
		Targetdata td;
		mutex get_data_mx;
		
		int frame_cols;
		
		
	public:
		ImgKCF();
		~ImgKCF();
		void thread_run();
		void thread_end();
		void start_init(); // 模板初始化
		bool finish_init(); // 模板初始化结束，阻塞线程大约2s
		Targetdata get_target_data();
		bool is_get_target_data();
};

bool ImgKCF::is_get_target_data(){
	return is_result_update;
}

Targetdata ImgKCF::get_target_data(){
	lock_guard<mutex> guard(get_data_mx);
	is_result_update = false;

	return td;
}

void ImgKCF::run(){
	cpu_set_t mask_k;
    cpu_set_t get_k;
    int num = sysconf(_SC_NPROCESSORS_CONF);
	int cpu_k = 2;
	CPU_ZERO(&mask_k);
    CPU_SET(cpu_k, &mask_k);
    if (pthread_setaffinity_np(pthread_self(), sizeof(mask_k), &mask_k) < 0) {
    	std::cout << "Fails to set the CPU to run the KCF thread" << std::endl;
    }
    CPU_ZERO(&get_k);
    if (pthread_getaffinity_np(pthread_self(), sizeof(get_k), &get_k) < 0) {
       std::cout << "Fails to get the CPU of the KCF thread" << std::endl;
    }
    for (int k = 0; k < num; k++) {
        if (CPU_ISSET(k, &get_k)) {
            std::cout << "the KCF thread " << (int)pthread_self() <<" is running in CPU " << k << std::endl;
        }
    }
	// 模板初始化
	while(!start_init_flag){usleep(1000*50);}
	finish_init_flag = false;
	capture >> imgFrame;
	
	frame_cols = imgFrame.cols;
	
	//test
	// namedWindow("camera", WINDOW_AUTOSIZE);
	
	if (imgFrame.empty()) { cerr << "  ERROR: Fails to read the camera!" << endl; exit(-1); }
	imgTrackerKCF.init(target, imgFrame);
	trackerWork = true;
	for(int k=0; k<10; k++){
		t_start = clock();
		capture >> imgFrame;
		if (imgFrame.empty()) { cerr << "  ERROR: Fails to read the camera!" << endl; exit(-1); }		
		imgTrackerKCF.update(imgFrame);
	}
	t_end = clock();
	t_diff = t_end - t_start;
	fps = CLOCKS_PER_SEC * 10.0 / (float)t_diff;
	finish_init_flag = true;
	// 进入工作循环
	while (!run_flag) {
		capture >> imgFrame;
		if (imgFrame.empty()) { cerr << "  ERROR: Fails to read the camera!" << endl; exit(-1); }
		// 跟踪状态
		if (trackerWork) {
			result = imgTrackerKCF.update(imgFrame);
			// test
			// rectangle(imgFrame, cv::Point(result.x, result.y), cv::Point(result.x+result.width, result.y+result.height), cv::Scalar(0,255,255), 1, 8);
			{
				lock_guard<mutex> guard(get_data_mx);
				if((++img_index)%10==0){
					t_end = clock(); 
					t_diff = t_end - t_start;
					fps = CLOCKS_PER_SEC * 10.0 / (float)t_diff;
					cout << "  fps = " << fps << endl;
					t_start = clock();

				}
				
				float cx = result.x + result.width/2;				
				td.index = img_index;
				td.angle = cx/frame_cols*80.0-40.0; // ...
				td.distance = 1.0/((float)result.width/(float)target.width);
				td.isTracked = true;// td. isTracked = // ... 
				td.fps = fps;
				is_result_update = true;				 
			}				
			// test
			// imshow("camera", imgFrame);
			// waitKey(1);
		}
	}	
}

bool ImgKCF::finish_init(){
	return finish_init_flag;
}


void ImgKCF::start_init(){
	start_init_flag = true;
}

ImgKCF::ImgKCF(){
	// 打开摄像头
	capture.open(0);
	if (!capture.isOpened()){ cerr << "  ERROR: Fails to open the camera!" << endl; exit(-1);}
	capture >> imgFrame;
	// 初始化
	trackerWork = false; // 跟踪算法是否处于工作状态
	run_flag = false; //系统停止工作
	img_index = 0; // 当前帧的序号
	t_start=clock();
	// 目标框初始化
	target.x = imgFrame.cols/10*4; 
	target.width = imgFrame.cols/5;
	target.y = imgFrame.rows/8*3; 
	target.height = imgFrame.rows/4;
	
	start_init_flag = false;
	finish_init_flag = false;
	is_result_update = false;
	fps = 0;
}
ImgKCF::~ImgKCF(){
	
}
void ImgKCF::thread_run(){
	t = new thread(&ImgKCF::run, this);
}
void ImgKCF::thread_end(){
	run_flag = false;
	t -> join();
}

#endif