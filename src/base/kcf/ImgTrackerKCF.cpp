/*
Default values are set for all properties of the tracker depending on the above choices.
Their values can be customized further before calling init():
    interp_factor: linear interpolation factor for adaptation
    sigma: gaussian kernel bandwidth
    lambda: regularization
    cell_size: HOG cell size
    padding: horizontal area surrounding the target, relative to its size
    output_sigma_factor: bandwidth of gaussian target
    template_size: template size in pixels, 0 to use ROI size
    scale_step: scale step for multi-scale estimation, 1 to disable it
    scale_weight: to downweight detection scores of other scales for added stability

For speed, the value (template_size/cell_size) should be a power of 2 or a product of small prime numbers.

Inputs to init():
   image is the initial frame.
   roi is a cv::Rect with the target positions in the initial frame

Inputs to update():
   image is the current frame.

Outputs of update():
   cv::Rect with target positions for the current frame
*/

#ifndef _IMGTRACKERKCF_HEADERS
#include "ImgTrackerKCF.hpp"
#include "ffttools.hpp"
#include "recttools.hpp"
#include "fhog.hpp"
#endif

using namespace std;
using namespace cv;

// Constructor
ImgTrackerKCF::ImgTrackerKCF()
{
    // Parameters equal in all cases
    lambda = 0.0001;
    padding = 2.5; 
    cell_size = 4;
    cell_sizeQ = cell_size*cell_size;

    // interp_factor = 0.005; // lab
    interp_factor = 0.012; // non-lab: VOT
    // interp_factor = 0.02; // non-lab: TPAMI

    // sigma = 0.4; // lab
    sigma = 0.6; // non-lab: VOT
    // sigma = 0.5; // non-lab: TPAMI

    // output_sigma_factor = 0.1; // lab
    output_sigma_factor = 0.125; // non-lab: VOT
	// output_sigma_factor = 0.025; // non-lab: TPAMI

    template_size = 64;
    scale_step = 1.05;
    scale_weight = 0.95;  
}

// Initialize tracker 
void ImgTrackerKCF::init(const cv::Rect &roi, cv::Mat image)
{
    _roi = roi;
    assert(roi.width >= 0 && roi.height >= 0);
    _tmpl = initFeatures(image);
    _prob = createGaussianPeak(size_patch[0], size_patch[1]);
    _alphaf = cv::Mat(size_patch[0], size_patch[1], CV_32FC2, float(0));
    train(_tmpl, 1.0); // train with initial frame
 }

// Update position based on the new frame
cv::Rect ImgTrackerKCF::update(cv::Mat image)
{
	/*
    if (_roi.x + _roi.width <= 0) _roi.x = -_roi.width + 1;
    if (_roi.y + _roi.height <= 0) _roi.y = -_roi.height + 1;
    if (_roi.x >= image.cols - 1) _roi.x = image.cols - 2;
    if (_roi.y >= image.rows - 1) _roi.y = image.rows - 2;
	*/
	float new_peak_value;
    float cx = _roi.x + _roi.width / 2.0f;
    float cy = _roi.y + _roi.height / 2.0f;

    // 响应最大值对应点位置的亚像素预测
    cv::Point2f res = detect(_tmpl, getFeatures(image), peak_value);

	// Test at a smaller _scale
	cv::Point2f new_res = detect(_tmpl, getFeatures(image, 1.0f / scale_step), new_peak_value);
	if (scale_weight * new_peak_value > peak_value) {
		res = new_res;
		peak_value = new_peak_value;
		_scale /= scale_step;
		_roi.width /= scale_step;
		_roi.height /= scale_step;
	}

	// Test at a bigger _scale
	new_res = detect(_tmpl, getFeatures(image, scale_step), new_peak_value);
	if (scale_weight * new_peak_value > peak_value) {
		res = new_res;
		peak_value = new_peak_value;
		_scale *= scale_step;
		_roi.width *= scale_step;
		_roi.height *= scale_step;
	}

    // Adjust by cell size and _scale
    _roi.x = cx - _roi.width / 2.0f + ((float) res.x * cell_size * _scale);
    _roi.y = cy - _roi.height / 2.0f + ((float) res.y * cell_size * _scale);

    if (_roi.x >= image.cols - 1) _roi.x = image.cols - 1;
    if (_roi.y >= image.rows - 1) _roi.y = image.rows - 1;
    if (_roi.x + _roi.width <= 0) _roi.x = -_roi.width + 2;
    if (_roi.y + _roi.height <= 0) _roi.y = -_roi.height + 2;

    assert(_roi.width >= 0 && _roi.height >= 0);
    cv::Mat x = getFeatures(image);
    train(x, interp_factor);

    return _roi;
}


// Detect object in the current frame.
cv::Point2f ImgTrackerKCF::detect(cv::Mat z, cv::Mat x, float &peak_value)
{
    using namespace FFTTools;

    cv::Mat k = gaussianCorrelation(x, z);
    cv::Mat res = (real(fftd(complexMultiplication(_alphaf, fftd(k)), true)));

    //minMaxLoc only accepts doubles for the peak, and integer points for the coordinates
    cv::Point2i pi; // 最大值对应点的坐标
    double pv; // 最大值
    cv::minMaxLoc(res, NULL, &pv, NULL, &pi);
    peak_value = (float) pv;

    //subpixel peak estimation, coordinates will be non-integer
    cv::Point2f p((float)pi.x, (float)pi.y);

    if (pi.x > 0 && pi.x < res.cols-1) {
        p.x += subPixelPeak(res.at<float>(pi.y, pi.x-1), peak_value, res.at<float>(pi.y, pi.x+1));
    }

    if (pi.y > 0 && pi.y < res.rows-1) {
        p.y += subPixelPeak(res.at<float>(pi.y-1, pi.x), peak_value, res.at<float>(pi.y+1, pi.x));
    }

    p.x -= (res.cols) / 2;
    p.y -= (res.rows) / 2;

    return p;
}

// train tracker with a single image
void ImgTrackerKCF::train(cv::Mat x, float train_interp_factor)
{
    using namespace FFTTools;

    cv::Mat k = gaussianCorrelation(x, x);
    cv::Mat alphaf = complexDivision(_prob, (fftd(k) + lambda)); // @suppress("Invalid arguments")
    
    _tmpl = (1 - train_interp_factor) * _tmpl + (train_interp_factor) * x;
    _alphaf = (1 - train_interp_factor) * _alphaf + (train_interp_factor) * alphaf;
}

// Evaluates a Gaussian kernel with bandwidth SIGMA for all relative shifts between input images X and Y, which must both be MxN. They must    also be periodic (ie., pre-processed with a cosine window).
cv::Mat ImgTrackerKCF::gaussianCorrelation(cv::Mat x1, cv::Mat x2)
{
    using namespace FFTTools;
    cv::Mat c = cv::Mat( cv::Size(size_patch[1], size_patch[0]), CV_32F, cv::Scalar(0) );
    // HOG features
    cv::Mat caux;
    cv::Mat x1aux;
    cv::Mat x2aux;
    for (int i = 0; i < size_patch[2]; i++) {
        x1aux = x1.row(i);   // Procedure do deal with cv::Mat multichannel bug
        x1aux = x1aux.reshape(1, size_patch[0]);
        x2aux = x2.row(i).reshape(1, size_patch[0]);
        cv::mulSpectrums(fftd(x1aux), fftd(x2aux), caux, 0, true); 
        caux = fftd(caux, true);
        rearrange(caux);
        caux.convertTo(caux,CV_32F);
        c = c + real(caux);
    }
      
    cv::Mat d; 
    cv::max( ((cv::sum(x1.mul(x1))[0] + cv::sum(x2.mul(x2))[0])- 2. * c) / (size_patch[0]*size_patch[1]*size_patch[2]) , 0, d); // 结果为负的提到0

    cv::Mat k;
    cv::exp((-d / (sigma * sigma)), k);
    return k;
}

// Create Gaussian Peak. Function called only in the first frame.
cv::Mat ImgTrackerKCF::createGaussianPeak(int sizey, int sizex)
{
    cv::Mat_<float> res(sizey, sizex);

    int syh = (sizey) / 2;
    int sxh = (sizex) / 2;

    float output_sigma = std::sqrt((float) sizex * sizey) / padding * output_sigma_factor;
    float mult = -0.5 / (output_sigma * output_sigma);

    for (int i = 0; i < sizey; i++)
        for (int j = 0; j < sizex; j++)
        {
            int ih = i - syh;
            int jh = j - sxh;
            res(i, j) = std::exp(mult * (float) (ih * ih + jh * jh));
        }
    return FFTTools::fftd(res);
}

cv::Mat ImgTrackerKCF::initFeatures(const cv::Mat & image)
{
    cv::Rect extracted_roi;

    float cx = _roi.x + _roi.width / 2;
    float cy = _roi.y + _roi.height / 2;

	int padded_w = _roi.width * padding;
	int padded_h = _roi.height * padding;

	if (padded_w >= padded_h)
		_scale = padded_w / (float)template_size;
	else
		_scale = padded_h / (float)template_size;

	_tmpl_sz.width = padded_w / _scale;
	_tmpl_sz.height = padded_h / _scale;

	// Round to cell size and also make it even
	// _tmpl_sz保证了extracted_roi的size是2×cell_size的整数倍，并能被全覆盖
	_tmpl_sz.width = ( ( (int)(_tmpl_sz.width / (2 * cell_size)) ) * 2 * cell_size ) + cell_size*2;
	_tmpl_sz.height = ( ( (int)(_tmpl_sz.height / (2 * cell_size)) ) * 2 * cell_size ) + cell_size*2;

	// extracted_roi应该是添加了padding的外围目标区域（size大约是roi的padding倍）
    extracted_roi.width = _scale * _tmpl_sz.width;
    extracted_roi.height = _scale * _tmpl_sz.height;

    // center roi with new size
    extracted_roi.x = cx - extracted_roi.width / 2.0;
    extracted_roi.y = cy - extracted_roi.height / 2.0;

    cv::Mat FeaturesMap;
    cv::Mat z = RectTools::subwindow(image, extracted_roi, cv::BORDER_REPLICATE);
    cv::resize(z, z, _tmpl_sz); // 把extracted_roi的长边归一化到template_size

    // 每次调用getFeatures进来新的extracted_roi都把size_patch更新
	// HOG features
	IplImage z_ipl = z;
	CvLSVMFeatureMapCaskade *map;
	getFeatureMaps(&z_ipl, cell_size, &map);
	normalizeAndTruncate(map, 0.2f);
	PCAFeatureMaps(map);
	size_patch[0] = map->sizeY;
	size_patch[1] = map->sizeX;
	size_patch[2] = map->numFeatures;

	FeaturesMap = cv::Mat(cv::Size(map->numFeatures, map->sizeX*map->sizeY), CV_32F, map->map);  // Procedure do deal with cv::Mat multichannel bug
	FeaturesMap = FeaturesMap.t();
	freeFeatureMapObject(&map);

	// 到这里FeaturesMap提取完成，size是(Y*X, numFeatures+nClusters)
    createHanningMats();
    FeaturesMap = hann.mul(FeaturesMap); // 按元素相乘
    return FeaturesMap;
}

// Obtain sub-window from image, with replication-padding and extract features
cv::Mat ImgTrackerKCF::getFeatures(const cv::Mat & image, float scale_adjust)
{
    cv::Rect extracted_roi;

    float cx = _roi.x + _roi.width / 2;
    float cy = _roi.y + _roi.height / 2;

	// extracted_roi应该是添加了padding的外围目标区域（size大约是roi的padding倍）
    extracted_roi.width = scale_adjust * _scale * _tmpl_sz.width;
    extracted_roi.height = scale_adjust * _scale * _tmpl_sz.height;

    // center roi with new size
    extracted_roi.x = cx - extracted_roi.width / 2.0;
    extracted_roi.y = cy - extracted_roi.height / 2.0;

    cv::Mat FeaturesMap;  
    cv::Mat z = RectTools::subwindow(image, extracted_roi, cv::BORDER_REPLICATE);
    cv::resize(z, z, _tmpl_sz); // 把extracted_roi的长边归一化到template_size（_tmpl_sz在init的时候确定）

    // 每次调用getFeatures进来新的extracted_roi都把size_patch更新
	// HOG features
	IplImage z_ipl = z;
	CvLSVMFeatureMapCaskade *map;
	getFeatureMaps(&z_ipl, cell_size, &map);
	normalizeAndTruncate(map, 0.2f);
	PCAFeatureMaps(map);
	size_patch[0] = map->sizeY;
	size_patch[1] = map->sizeX;
	size_patch[2] = map->numFeatures;

	FeaturesMap = cv::Mat(cv::Size(map->numFeatures, map->sizeX*map->sizeY), CV_32F, map->map);  // Procedure do deal with cv::Mat multichannel bug
	FeaturesMap = FeaturesMap.t();
	freeFeatureMapObject(&map);

    FeaturesMap = hann.mul(FeaturesMap); // 按元素相乘
    return FeaturesMap;
}
    
// Initialize Hanning window. Function called only in the first frame.
void ImgTrackerKCF::createHanningMats()
{   
    cv::Mat hann1t = cv::Mat(cv::Size(size_patch[1],1), CV_32F, cv::Scalar(0));
    cv::Mat hann2t = cv::Mat(cv::Size(1,size_patch[0]), CV_32F, cv::Scalar(0)); 

    for (int i = 0; i < hann1t.cols; i++)
        hann1t.at<float> (0, i) = 0.5 * (1 - std::cos(2 * 3.14159265358979323846 * i / (hann1t.cols - 1)));
    for (int i = 0; i < hann2t.rows; i++)
        hann2t.at<float> (i, 0) = 0.5 * (1 - std::cos(2 * 3.14159265358979323846 * i / (hann2t.rows - 1)));

    cv::Mat hann2d = hann2t * hann1t;

	// HOG features
	cv::Mat hann1d = hann2d.reshape(1, 1); // Procedure do deal with cv::Mat multichannel bug // 转化为1个channel和1行（长度Y*X）

	hann = cv::Mat(cv::Size(size_patch[0] * size_patch[1], size_patch[2]), CV_32F, cv::Scalar(0));
	for (int i = 0; i < size_patch[2]; i++) {
		for (int j = 0; j < size_patch[0] * size_patch[1]; j++) {
			hann.at<float>(i, j) = hann1d.at<float>(0, j); // 对于每个Feature都被长Y*X的cos窗加权
		}
	}   
}

// Calculate sub-pixel peak for one dimension
float ImgTrackerKCF::subPixelPeak(float left, float center, float right)
{   
    float divisor = 2 * center - right - left;

    if (divisor == 0)
        return 0;
    
    return 0.5 * (right - left) / divisor;
}
