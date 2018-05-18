#ifndef __IMG_KCF_H
#define __IMG_KCF_H
//////////////////////////////////////
#include <iostream>  
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <time.h>
#include "ImgTrackerKCF.hpp"
#include <thread>
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
		cv::Mat imgFrame; // ����ͷ�ɼ�����ͼƬ
		VideoCapture capture(2);
		bool trackerWork;
		bool run_flag;
		int img_index;
		Rect target; // ��ʼ���ͼ���õ��ĵ�Ŀ������;
		Rect result;
		clock_t t_start;
		clock_t t_end;
		clock_t t_diff;
		ImgTrackerKCF imgTrackerKCF; // ������
		float fps;
		thread* t;
		void run();
		
		bool start_init_flag; // ģ���ʼ��
		bool finish_init_flag; // ģ���ʼ������
		bool is_result_update;
		Targetdata td;
		mutex get_data_mx;
		
		int frame_width;
		
		
	public:
		ImgKCF();
		~ImgKCF();
		void thread_run();
		void thread_end();
		void start_init(); // ģ���ʼ��
		bool finish_init(); // ģ���ʼ�������������̴߳�Լ2s
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
	// ģ���ʼ��
	while(!start_init_flag){usleep(1000*1000*50);}
	finish_init_flag = false;
	capture >> imgFrame;
	
	frame_width = imgFrame.rows;
	
	//test
	namedWindow("camera", WINDOW_AUTOSIZE);
	
	if (imgFrame.empty()) { cerr << "ERROR: Fails to read the camera!" << endl; exit(-1); }
	imgTrackerKCF.init(target, imgFrame);
	trackerWork = true;
	for(int k=0; k<10; k++){
		t_start = clock();
		capture >> imgFrame;
		if (imgFrame.empty()) { cerr << "ERROR: Fails to read the camera!" << endl; exit(-1); }		
		imgTrackerKCF.update(imgFrame);
	}
	t_end = clock();
	t_diff = t_end - t_start;
	fps = CLOCKS_PER_SEC * 10.0 / (float)t_diff;
	finish_init_flag = true;
	

	// ���빤��ѭ��
	while (!run_flag) {
		capture >> imgFrame;
		if (imgFrame.empty()) { cerr << "ERROR: Fails to read the camera!" << endl; exit(-1); }
		
		// ����״̬
		if (trackerWork) {
			result = imgTrackerKCF.update(imgFrame);
			retangle(imgFrame, cv::Point(result.x, result.y), cv::Point(result.x+result.width, result.y+result.height), cv::Scalar(0,255,255), 1, 8);
			{
				lock_guard<mutex> guard(get_data_mx);
				if((++img_index)%10==0){
					t_end = clock(); 
					t_diff = t_end - t_start;
					fps = CLOCKS_PER_SEC * 10.0 / (float)t_diff;
					t_start = clock();
				}
				
				float cx = result.x + result.width/2;
				// float cy = result.y + result.height/2;
				
				td.index = img_index;
				td.angle = cx/frame_width/80.0-40.0; // ...
				td.distance = (result.width/target.width)*INIT_DISTANCE;
				td.isTracked = true;// td. isTracked = // ... 
				td.fps = fps;
				is_result_update = true;
			}	
			
			// test
			imshow("camera", imgFrame);
			waitKey(1);
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
	// ������ͷ
	if (!capture.isOpened()){ cerr << "ERROR: Fails to open the camera!" << endl; return -1;}
	capture >> imgFrame;
	// ��ʼ��
	trackerWork = false; // �����㷨�Ƿ��ڹ���״̬
	run_flag = false; //ϵͳֹͣ����
	img_index = 0; // ��ǰ֡�����
	t_start=clock();
	// Ŀ����ʼ��(ˮƽռ40%����ֱռ50%����256*240)
	target.x = imgFrame.cols/10*3; 
	target.width = imgFrame.cols/10*4;
	target.y = imgFrame.rows/4; 
	target.height = imgFrame.rows/2;
	
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