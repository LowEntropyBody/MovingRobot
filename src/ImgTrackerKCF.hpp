#pragma once

#include "ImgTracker.h"

#ifndef _OPENCV_KCFTRACKER_HPP_
#define _OPENCV_KCFTRACKER_HPP_
#endif

class ImgTrackerKCF : public ImgTracker
{
public:
    // Constructor
	ImgTrackerKCF();

    // Initialize tracker 
    virtual void init(const cv::Rect &roi, cv::Mat image);
    
    // Update position based on the new frame
    virtual cv::Rect update(cv::Mat image);

    float interp_factor; // linear interpolation factor for adaptation
    float sigma; // gaussian kernel bandwidth
    float lambda; // regularization
    int cell_size; // HOG cell size
    int cell_sizeQ; // cell size^2, to avoid repeated operations
    float padding; // extra area surrounding the target
    float output_sigma_factor; // bandwidth of gaussian target
    int template_size; // template size
    float scale_step; // scale step for multi-scale estimation
    float scale_weight;  // to downweight detection scores of other scales for added stability
    float peak_value; // 目标候选框匹配的最大响应值

protected:
    // Detect object in the current frame.
    cv::Point2f detect(cv::Mat z, cv::Mat x, float &peak_value);

    // train tracker with a single image
    void train(cv::Mat x, float train_interp_factor);

    // Evaluates a Gaussian kernel with bandwidth SIGMA for all relative shifts between input images X and Y, which must both be MxN. They must    also be periodic (ie., pre-processed with a cosine window).
    cv::Mat gaussianCorrelation(cv::Mat x1, cv::Mat x2);

    // Create Gaussian Peak. Function called only in the first frame.
    cv::Mat createGaussianPeak(int sizey, int sizex);

    // Obtain sub-window from image, with replication-padding and extract features
    cv::Mat getFeatures(const cv::Mat & image, float scale_adjust = 1.0f);

    // Initialize Hanning window. Function called only in the first frame.
    void createHanningMats();

    // Calculate sub-pixel peak for one dimension
    float subPixelPeak(float left, float center, float right);

    // 初始化时用到的简化版getFeatures
    cv::Mat initFeatures(const cv::Mat & image);

    cv::Mat _alphaf; // (Y, X)
    cv::Mat _prob; // (Y, X)二维高斯的FFT
    cv::Mat _tmpl; // (Y*X, numFeatures)

private:
    int size_patch[3]; // (Y, X, numFeatures)
    cv::Mat hann;  // (Y*X, numFeatures)
    cv::Size _tmpl_sz; // extracted_roi长边归一化到template_size后的size（2*cell_size全覆盖）
    float _scale; // extracted_roi长边对于template_size的比例
};
