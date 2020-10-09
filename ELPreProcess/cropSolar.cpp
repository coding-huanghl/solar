#include "timer.h"
#include "cropSolar.h"

bool isLoadDistortionParam = false;
Mat intrinsic_matrix = Mat(3, 3, CV_32FC1);
Mat distortion_coeffs = Mat(1, 5, CV_32FC1);
bool getMaxRectContours(Mat image, Mat thImg, RotatedRect& Box)
{
	vector<vector<Point>>contours;
	vector<Vec4i>hierarchy;
	findContours(thImg, contours, hierarchy, RETR_EXTERNAL, CHAIN_APPROX_NONE);
	int max_index = 0;
	int max_area = 0;
	for (size_t i = 0; i < contours.size(); i++)
	{
		if (max_area < contourArea(contours[i]))
		{
			max_area = contourArea(contours[i]);
			max_index = i;
		}
	}
	Box = minAreaRect(contours[max_index]);

	Mat tmpMat = image.clone();
	Point2f pts[4];
	Box.points(pts);
	for (size_t i = 0; i < 4; i++)
		line(tmpMat, pts[i], pts[(i + 1) % 4], Scalar(255));
	drawContours(tmpMat, contours, max_index, Scalar(255), 2);

	return contourArea(contours[max_index]) / Box.size.area() > 0.8;
}

void getLocalCoord(Mat SolarImage, double thresh, vector<int>& pos_vec, int blocks)
{
	Mat threshImg;
	Mat meanValue, stdValue;
	meanStdDev(SolarImage, meanValue, stdValue);
	threshold(SolarImage, threshImg, meanValue.at<double>(0, 0) - 1.3 * stdValue.at<double>(0, 0), 255, THRESH_BINARY_INV);
	Mat ele3 = getStructuringElement(MORPH_RECT, Size(7, 7));
	Mat ele4 = getStructuringElement(MORPH_RECT, Size(1, 35));
	morphologyEx(threshImg, threshImg, MORPH_DILATE, ele3);
	morphologyEx(threshImg, threshImg, MORPH_ERODE, ele4);
	threshold(threshImg, threshImg, 254, 255, THRESH_BINARY);

	vector<int> mean_vec;
	for (size_t i = 0; i < threshImg.cols; i++)
	{
		Mat tmpColMat = threshImg.col(i);
		int meangray = mean(tmpColMat)[0];
		mean_vec.push_back(meangray);
	}

	int step = mean_vec.size() / blocks;
	for (size_t i = 0; i < blocks; i++)
	{
		int tmpPos = 0;
		int tmpCount = 0;
		for (size_t j = step * i; j < step * i + step; j++)
		{
			if (mean_vec[j] > 128)
			{
				tmpPos += j;
				tmpCount++;
			}
		}
		if (tmpCount == 0)
			pos_vec.push_back(step * i + step / 2);
		else
			pos_vec.push_back(tmpPos / tmpCount);
	}
}

bool loadDistortionParam()
{
	FileStorage fs("D:\\Program Files\\Config\\ELStr\\distortionLens.xml", FileStorage::READ);
	if (fs.isOpened())
	{
		fs["intrinsic_matrix"] >> intrinsic_matrix;
		fs["distortion_coeffs"] >> distortion_coeffs;
		fs.release();
		return true;
	}
	return false;
}

int cropSolar(const Mat& srcImage, vector<Mat>& CutMats, Mat& SolarImage, double param[])
{
	int blocks = param[0];//切割的电池片块数
	double oneSwitchCell = param[1];//电池片类型
	int k1 = param[2];
	int k2 = param[3];

	int cut_pos[2] = { 200,1000 };
	Mat copiedSrcImage;
	if (!isLoadDistortionParam)
	{
		isLoadDistortionParam = loadDistortionParam();
		if (!isLoadDistortionParam) return -5;
	}
	undistort(srcImage, copiedSrcImage, intrinsic_matrix, distortion_coeffs);

	Rect SolarRegion = Rect(0, cut_pos[0], copiedSrcImage.cols, cut_pos[1] - cut_pos[0]);
	//图像亮度修正
	double a = -5e-5;
	int center_x = copiedSrcImage.cols / 2;
	int center_y = copiedSrcImage.rows / 2;
	for (int i = cut_pos[0]; i < cut_pos[1]; i++)
	{
		uchar* p = copiedSrcImage.ptr<uchar>(i);
		for (int j = 0; j < copiedSrcImage.cols; j++)
		{
			double p1 = i - center_y;
			double p2 = j - center_x;
			float x = sqrt(p1 * p1 + p2 * p2);
			p[j] = saturate_cast<uchar>(p[j] - a * x * x);
		}
	}
	copiedSrcImage = copiedSrcImage(SolarRegion);

	Mat threshImg;
	double t = threshold(copiedSrcImage, threshImg, 0, 255, THRESH_OTSU);
	//Mat meanValue, stdValue;
	//Mat tmpMat = copiedSrcImage(Rect(copiedSrcImage.cols / 2 - 300, copiedSrcImage.rows / 2 - 100, 600, 100)).clone();
	//meanStdDev(tmpMat, meanValue, stdValue);
	//double t = threshold(copiedSrcImage, threshImg, meanValue.at<double>(0, 0) - 30, 255, THRESH_BINARY);

	Mat ele3 = getStructuringElement(MORPH_RECT, Size(15, 15));
	morphologyEx(threshImg, threshImg, MORPH_OPEN, ele3);

	Mat ele0 = getStructuringElement(MORPH_RECT, Size(10, 10));
	morphologyEx(threshImg, threshImg, MORPH_CLOSE, ele0);

	Mat ele4 = getStructuringElement(MORPH_RECT, Size(70, 70));
	morphologyEx(threshImg, threshImg, MORPH_OPEN, ele4);

	Mat ele1 = getStructuringElement(MORPH_RECT, Size(k1, k1));
	morphologyEx(threshImg, threshImg, MORPH_ERODE, ele1);
	Mat ele2 = getStructuringElement(MORPH_RECT, Size(k2, k2));
	morphologyEx(threshImg, threshImg, MORPH_DILATE, ele2);
	threshold(threshImg, threshImg, 254, 255, THRESH_BINARY);

	RotatedRect Box;
	bool isOK = getMaxRectContours(copiedSrcImage, threshImg, Box);
	if (!isOK)
	{
		cout << "提取电池串错误1,判为NG" << endl;
		return -1;
	}

	float rotation_angle = Box.angle;
	if (rotation_angle < -45) rotation_angle = rotation_angle + 90;
	if (rotation_angle > 45) rotation_angle = rotation_angle - 90;
	Mat M = getRotationMatrix2D(Box.center, rotation_angle, 1);
	Mat affinedImage;
	warpAffine(copiedSrcImage, affinedImage, M, copiedSrcImage.size());
	Mat colorImage;
	cvtColor(affinedImage, colorImage, COLOR_GRAY2BGR);

	int SolarHeight, SolarWidth;
	if (Box.angle < -45)
	{
		SolarHeight = rotation_angle >= 0 ? Box.size.width : Box.size.height + k1 - k2;
		SolarWidth = rotation_angle < 0 ? Box.size.width : Box.size.height + k1 - k2;
	}
	else
	{
		SolarHeight = rotation_angle > 0 ? Box.size.width : Box.size.height + k1 - k2;
		SolarWidth = rotation_angle <= 0 ? Box.size.width : Box.size.height + k1 - k2;
	}

	
	if (SolarWidth < blocks * SolarHeight / oneSwitchCell)
	{
		cout << "提取电池串错误2,判为NG" << endl;
		return -2;
	}

	int offset = SolarHeight / (oneSwitchCell * 4);
	int startPos;
	Rect posROI;
	Rect rectSolar;
	vector<int> pos_vec;

	if (Box.center.x > copiedSrcImage.cols / 2)
	{
		startPos = Box.center.x - SolarWidth / 2 + offset;
		Rect r = Rect(startPos, Box.center.y - SolarHeight / 2, int(blocks * SolarHeight / oneSwitchCell), SolarHeight);
		rectangle(colorImage, r, Scalar(0, 255, 0));
		pos_vec.push_back(-offset);
		getLocalCoord(affinedImage(r).clone(), t, pos_vec, blocks);
	}
	else
	{
		int endPos = int(blocks * SolarHeight / oneSwitchCell) + offset;
		startPos = Box.center.x + SolarWidth / 2 - endPos;
		if (startPos < 0)
		{
			offset += startPos;
			endPos += startPos;
			startPos = 0;
		}
		Rect r = Rect(startPos, Box.center.y - SolarHeight / 2, int(blocks * SolarHeight / oneSwitchCell), SolarHeight);
		rectangle(colorImage, r, Scalar(0, 255, 0));
		getLocalCoord(affinedImage(r).clone(), t, pos_vec, blocks);
		pos_vec.push_back(endPos);
	}
	rectSolar = Rect(startPos + pos_vec[0], Box.center.y - SolarHeight / 2, pos_vec[pos_vec.size() - 1] - pos_vec[0], SolarHeight);
	rectangle(colorImage, rectSolar, Scalar(0, 0, 255), 1);
	SolarImage = affinedImage(rectSolar).clone();
	for (size_t i = 1; i < pos_vec.size(); i++)
	{
		Rect tmpR = Rect(startPos + pos_vec[i - 1], Box.center.y - SolarHeight / 2, pos_vec[i] - pos_vec[i - 1], SolarHeight);
		rectangle(colorImage, tmpR, Scalar(255, 0, 0), 2);
		CutMats.push_back(affinedImage(tmpR));
	}
	return 0;
}

int cropSolarSharp(unsigned char* pSrcImage, int width, int height, int solarSize[4], double param[2], unsigned char** pSolarImage)
{
	Mat SrcImage = Mat(height, width, CV_8UC1, pSrcImage);
	vector<Mat> cutMats;
	Mat SolarImage;
	int status = cropSolar(SrcImage, cutMats, SolarImage, param);
	if (status == 0)
	{
		resize(SolarImage, SolarImage, Size(solarSize[0], solarSize[1]));
		memcpy(pSolarImage[0], SolarImage.data, SolarImage.rows * SolarImage.cols);
		for (size_t i = 0; i < cutMats.size(); i++)
		{
			resize(cutMats[i], cutMats[i], Size(solarSize[2], solarSize[3]));
			memcpy(pSolarImage[i + 1], cutMats[i].data, cutMats[i].rows * cutMats[i].cols);
		}
	}
	return status;
}

int calibrateCam(unsigned char * pSrcImage, int width, int height, double& firstSide, double& secondSide, double& thirdSide, double& fourthSide)
{
	Mat SrcImage = Mat(height, width, CV_8UC1, pSrcImage);
	Mat threshImg;
	double t = threshold(SrcImage, threshImg, 0, 255, THRESH_OTSU);

	int k1 = 25, k2 = 40;
	Mat ele0 = getStructuringElement(MORPH_RECT, Size(5, 5));
	morphologyEx(threshImg, threshImg, MORPH_CLOSE, ele0);
	Mat ele1 = getStructuringElement(MORPH_RECT, Size(k1, k1));
	morphologyEx(threshImg, threshImg, MORPH_ERODE, ele1);
	Mat ele2 = getStructuringElement(MORPH_RECT, Size(k2, k2));
	morphologyEx(threshImg, threshImg, MORPH_DILATE, ele2);
	threshold(threshImg, threshImg, 254, 255, THRESH_BINARY);

	vector<vector<Point>>contours;
	vector<Vec4i>hierarchy;
	findContours(threshImg, contours, hierarchy, RETR_EXTERNAL, CHAIN_APPROX_NONE);
	int max_index = 0;
	int max_area = 0;
	for (size_t i = 0; i < contours.size(); i++)
	{
		if (max_area < contourArea(contours[i]))
		{
			max_area = contourArea(contours[i]);
			max_index = i;
		}
	}

	vector<vector<Point>> contours_poly(contours.size());
	approxPolyDP(Mat(contours[max_index]), contours_poly[max_index], 300, true);
	vector<Point> maxPloy = contours_poly[max_index];

	if (maxPloy.size() == 4)
	{
		Point2f angle[4];
		for (int i = 0; i < maxPloy.size(); i++)
		{
			angle[i] = maxPloy[i];
		}
	
		return 1;
	}
	else
		return -1;
	
}

-- by coding-huanghl

