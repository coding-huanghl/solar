// 下列 ifdef 块是创建使从 DLL 导出更简单的
// 宏的标准方法。此 DLL 中的所有文件都是用命令行上定义的 APP_EXPORTS
// 符号编译的。在使用此 DLL 的
// 任何其他项目上不应定义此符号。这样，源文件中包含此文件的任何其他项目都会将
// APP_API 函数视为是从 DLL 导入的，而此 DLL 则将用此宏定义的
// 符号视为是被导出的。

#define APP_API __declspec(dllexport)


#include<opencv.hpp>


#include <opencv2/opencv.hpp>
#include <stdio.h>
#include <iostream>
#include <opencv2\core\core.hpp>
#include <opencv2\imgproc\imgproc.hpp>
#include <highgui\highgui.hpp>
#include <opencv2\core\core_c.h>
#include <cv.h>
#include <string.h>
#include <opencv\cxcore.h>  

#include <Windows.h> 
#include <string>

#include <algorithm>

using namespace cv;
using namespace std;

class APP_API Capp {
public:
	Capp(void);
	// TODO:  在此添加您的方法。
};

extern APP_API int napp;

extern"C" {
	/*APP_API int calibra(BYTE *img, int h, int w, int channel);
	APP_API bool comp(const Vec3f a, const Vec3f b);
	APP_API int transform(Point CCDPoint, Point &LaserPoint, double param[]);*/

	APP_API int ImageCuter(int thresh, unsigned char *src, int CamNum, int CamIndex, int w, int h, int channel, int imgsize, unsigned char *output1, unsigned char *output2, unsigned char *output3, unsigned char *output);

	APP_API int ProcessImgFor2CamPos1(int thresh, unsigned char *src, int w, int h, int imgsize, bool isFlip, unsigned char *output1, unsigned char *output2, unsigned char *output3, unsigned char *output4, unsigned char *output5, unsigned char *output6, unsigned char *output);

	APP_API int ProcessImgFor2CamPos2(int thresh, unsigned char *src, int w, int h, int imgsize,bool isFlip, unsigned char *output1, unsigned char *output2, unsigned char *output3, unsigned char *output4, unsigned char *output5, unsigned char *output6, unsigned char *output);


	APP_API int add(int a, int b);



}