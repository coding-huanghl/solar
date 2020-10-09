#pragma once
#include<iostream>
#include<cmath>
#include <opencv.hpp>

using namespace cv;
using namespace std;

bool getMaxRectContours(Mat image, Mat thImg, RotatedRect& Box);
void getLocalCoord(Mat SolarImage, double thresh, vector<int>& pos_vec, int blocks);
bool loadDistortionParam();
int cropSolar(const Mat& srcImage, vector<Mat>& CutMats, Mat& SolarImage, double param[]);
double getDistance(CvPoint pointO, CvPoint pointA);

extern "C" __declspec(dllexport) int cropSolarSharp(unsigned char* pSrcImage, int width, int height, int solarSize[4], double param[4], unsigned char** pSolarImage);

extern "C" __declspec(dllexport) int calibrateCam(unsigned char* pSrcImage, int width, int height, double& firstSide, double& secondSide, double& thirdSide, double& fourthSide);