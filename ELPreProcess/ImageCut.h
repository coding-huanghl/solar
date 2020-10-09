// ���� ifdef ���Ǵ���ʹ�� DLL �������򵥵�
// ��ı�׼�������� DLL �е������ļ��������������϶���� APP_EXPORTS
// ���ű���ġ���ʹ�ô� DLL ��
// �κ�������Ŀ�ϲ�Ӧ����˷��š�������Դ�ļ��а������ļ����κ�������Ŀ���Ὣ
// APP_API ������Ϊ�Ǵ� DLL ����ģ����� DLL ���ô˺궨���
// ������Ϊ�Ǳ������ġ�

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
	// TODO:  �ڴ�������ķ�����
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