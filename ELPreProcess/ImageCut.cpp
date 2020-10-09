// ImageCut.cpp : 定义 DLL 应用程序的导出函数。
//

#include "stdafx.h"
#include "ImageCut.h"
#include <math.h>
using namespace cv;
using namespace std;




APP_API int napp = 0;
APP_API int Bytes2Mat(BYTE* src, int w, int h, int channel, Mat& img_gray) {
	try {
		int format;
		switch (channel)
		{
		case 1:
			format = CV_8UC1;
			break;

		case 2:
			format = CV_8UC2;
			break;
		case 3:
			format = CV_8UC3;
			break;
		default:
			format = CV_8UC4;
			break;
		}
		Mat img(h, w, format, src);

		if (false) imwrite("Cell.png", img);

		switch (format)
		{
		case CV_8UC1:
			img.copyTo(img_gray);
			break;
		case CV_8UC3:
			cvtColor(img, img_gray, CV_RGB2GRAY);
			break;
		default:
			break;
		}
	}
	catch (Exception e) {
		return -1;
	}
	return 0;

}
//开运算
void opening(Mat in, Mat & out, double x, double y, int times)
{
	Mat element = getStructuringElement(MORPH_RECT, Size(x, y), Point(-1, -1));
	//for (int i = 0; i < times; i++)
	//{
	//erode(in, out, element);
	//dilate(out, out, element);
	//}
	erode(in, out, element, Point(-1, -1), times);
	dilate(out, out, element, Point(-1, -1), times);
}
//闭运算
void closing(Mat in, Mat & out, double x, double y, int times)
{
	Mat element = getStructuringElement(MORPH_RECT, Size(x, y), Point(-1, -1));

	dilate(in, out, element, Point(-1, -1), times);
	erode(out, out, element, Point(-1, -1), times);
}
//阈值分割（二值化）
void thresholdImg(Mat in, Mat out, double thresh, double maxVal)
{
	threshold(in, out, thresh, maxVal, THRESH_BINARY);
}
//获取白色区域
void getWhiteRigion(Mat in, Rect & Roirect, RotatedRect & Roirectmin)
{
	vector<cv::Point> nonBlackList;
	nonBlackList.reserve(in.rows*in.cols);

	// add all non-black points to the vector
	//TODO: there are more efficient ways to iterate through the image
	for (int j = 0; j<in.rows; ++j)
	for (int i = 0; i<in.cols; ++i)
	{
		// if not black: add to the list
		if (in.at <uchar>(j, i) != 0)
		{
			nonBlackList.push_back(Point(i, j));
		}
	}

	// create bounding rect around those points
	Roirect = boundingRect(nonBlackList);
	Roirectmin = minAreaRect(nonBlackList);

}

APP_API int add(int a, int b)
{
	int sum = a + b;

	return sum;
}

APP_API int Mat2Bytes(BYTE* &src, Mat& img_gray, size_t &size)
{

	try {
		while (*src) *src++ = *img_gray.data++;
	}
	catch (Exception e) {
		return -1;
	}
	return 0;
}


vector<vector<cv::Point2i>> contoursFor2CamPos1;
vector<vector<cv::Point2i>> contourFor2CamPos1;
//半片两个相机 相机位1
APP_API int ProcessImgFor2CamPos1(int thresh, BYTE* src, int w, int h, int imgsize, bool isFlip, unsigned char *output1, unsigned char *output2, unsigned char *output3, unsigned char *output4, unsigned char *output5, unsigned char *output6, unsigned char *output)
{
	try
	{
		Mat in = Mat(h, w, CV_8UC1, src);
		/*Mat in;
		Bytes2Mat(src, w, h, 1, in);*/
		Rect RoiCut(100, in.rows / 5, in.cols - 360, in.rows * 3 / 5);
		in = in(RoiCut);
		Mat FirstGray;
		in.copyTo(FirstGray);
		threshold(FirstGray, FirstGray, thresh, 255, cv::THRESH_BINARY);
		closing(FirstGray, FirstGray, 3, 5, 2);
		opening(FirstGray, FirstGray, 40, 10, 1);

		findContours(FirstGray, contourFor2CamPos1, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_NONE);
		double maxarea = 0;
		int maxAreaIdx = 0;
		for (int index = 0; index <= contourFor2CamPos1.size() - 1; index++)
		{
			double tmparea = fabs(contourArea(contourFor2CamPos1[index]));
			if (tmparea>maxarea)
			{
				maxarea = tmparea;
				maxAreaIdx = index;//记录最大轮廓的索引号
			}
		}

		RotatedRect  Roirectmin1 = minAreaRect(contourFor2CamPos1[maxAreaIdx]);

		double angle = Roirectmin1.angle;
		if (angle < -45)
		{
			angle = angle + 90;
		}
		Mat ImgRotate;//旋转后图像
		Mat in2;
		Point2f center(in.cols / 2, in.rows / 2);
		Mat rot = getRotationMatrix2D(center, angle, 1);
		Rect box = RotatedRect(center, in.size(), angle).boundingRect();
		warpAffine(in, ImgRotate, rot, box.size());//进行旋转

		Rect Roirectdst;
		RotatedRect Roirectmindst;
		Mat SecondGray, DstImage;
		threshold(ImgRotate, in2, thresh, 255, cv::THRESH_BINARY);
		closing(in2, in2, 1, 20, 1);
		opening(in2, SecondGray, 40, 260, 1);

		Mat hierarchy;
		findContours(SecondGray, contoursFor2CamPos1, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_NONE);


		int size = contoursFor2CamPos1.size();
#pragma region //找轮廓坐标由小到大排序
		int leftLocation[10];
		int rightLocation[10];
		int YLocation[10];
		int heightList[10];
		for (int i = 0; i < contoursFor2CamPos1.size(); i++)
		{
			Rect  Roirect1 = boundingRect(contoursFor2CamPos1[i]);
			int left, right, Y, height;
			left = Roirect1.x;
			leftLocation[i] = left;

			right = Roirect1.x + Roirect1.width;
			rightLocation[i] = right;

			Y = Roirect1.y;
			YLocation[i] = Y;

			height = Roirect1.height;
			heightList[i] = height;

		}
		sort(leftLocation, leftLocation + size);
		sort(rightLocation, rightLocation + size);
		sort(YLocation, YLocation + size);
		sort(heightList, heightList + size);
#pragma endregion

		int leftX, leftY, regionWidth, regionHeight;
		leftX = leftLocation[0] - 5;
		leftY = YLocation[0] - 5;
		regionWidth = (rightLocation[size - 2] + leftLocation[size - 1]) / 2 - leftX;
		regionHeight = heightList[size - 1] + 5;
		Rect RoiCutLocation(leftX, leftY, regionWidth, regionHeight);
		DstImage = ImgRotate(RoiCutLocation);
		//imwrite("C:/Users/ATWER/Desktop/左.jpg", DstImage);
		Size PicSize;
		Mat TempImage;
		//PicSize = Size(DstImage.rows, DstImage.rows);
		PicSize = Size(imgsize * 3, imgsize);
		resize(DstImage, TempImage, PicSize);
		if (isFlip)
		{
			cv::flip(TempImage, TempImage, 1);
		}
		memcpy(output, TempImage.data, imgsize*imgsize * 3);

		
#pragma region //分割图片，传出指针
		Mat FirstImg, SecondImg, ThirdImg, FourthImg, FifthImg, SixthImg;
		int X = (DstImage.cols) / 6;
		Rect RoiCutFirst(0, 0, X, DstImage.rows);
		Rect RoiCutSecond(X, 0, X, DstImage.rows);
		Rect RoiCutThird(2 * X, 0, X, DstImage.rows);
		Rect RoiCutFourth(3 * X, 0, X, DstImage.rows);
		Rect RoiCutFifth(4 * X, 0, X, DstImage.rows);
		Rect RoiCutSixth(5 * X, 0, DstImage.cols - 5 * X, DstImage.rows);

		FirstImg = DstImage(RoiCutFirst);
		SecondImg = DstImage(RoiCutSecond);
		ThirdImg = DstImage(RoiCutThird);
		FourthImg = DstImage(RoiCutFourth);
		FifthImg = DstImage(RoiCutFifth);
		SixthImg = DstImage(RoiCutSixth);
		
		PicSize = Size(imgsize/2, imgsize);
		resize(FirstImg, FirstImg, PicSize);
		resize(SecondImg, SecondImg, PicSize);
		resize(ThirdImg, ThirdImg, PicSize);
		resize(FourthImg, FourthImg, PicSize);
		resize(FifthImg, FifthImg, PicSize);
		resize(SixthImg, SixthImg, PicSize);

		memcpy(output1, FirstImg.data, imgsize*imgsize/2);
		memcpy(output2, SecondImg.data, imgsize*imgsize/2);
		memcpy(output3, ThirdImg.data, imgsize*imgsize/2);
		memcpy(output4, FourthImg.data, imgsize*imgsize / 2);
		memcpy(output5, FifthImg.data, imgsize*imgsize / 2);
		memcpy(output6, SixthImg.data, imgsize*imgsize / 2);
#pragma endregion
		return 1;		
	}
	catch (Exception e)
	{
		return 0;
	}
}



vector<vector<cv::Point2i>> contoursFor2CamPos2;
vector<vector<cv::Point2i>> contourFor2CamPos2;
//半片两个相机 相机位1
APP_API int ProcessImgFor2CamPos2(int thresh, BYTE* src, int w, int h, int imgsize,bool isFlip, unsigned char *output1, unsigned char *output2, unsigned char *output3, unsigned char *output4, unsigned char *output5, unsigned char *output6, unsigned char *output)
{
	try
	{
		Mat in = Mat(h, w, CV_8UC1, src);
		//Bytes2Mat(src, w, h, channel, in);
		Rect RoiCut(260, in.rows / 5, in.cols - 360, in.rows * 3 / 5);
		in = in(RoiCut);
		Mat FirstGray;
		in.copyTo(FirstGray);
		threshold(FirstGray, FirstGray, thresh, 255, cv::THRESH_BINARY);
		closing(FirstGray, FirstGray, 3, 5, 2);
		opening(FirstGray, FirstGray, 40, 10, 1);

		findContours(FirstGray, contourFor2CamPos2, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_NONE);
		double maxarea = 0;
		int maxAreaIdx = 0;
		for (int index = 0; index <= contourFor2CamPos2.size() - 1; index++)
		{
			double tmparea = fabs(contourArea(contourFor2CamPos2[index]));
			if (tmparea>maxarea)
			{
				maxarea = tmparea;
				maxAreaIdx = index;//记录最大轮廓的索引号
			}
		}

		RotatedRect  Roirectmin1 = minAreaRect(contourFor2CamPos2[maxAreaIdx]);

		double angle = Roirectmin1.angle;
		if (angle < -45)
		{
			angle = angle + 90;
		}
		Mat ImgRotate;//旋转后图像
		Mat in2;
		Point2f center(in.cols / 2, in.rows / 2);
		Mat rot = getRotationMatrix2D(center, angle, 1);
		Rect box = RotatedRect(center, in.size(), angle).boundingRect();
		warpAffine(in, ImgRotate, rot, box.size());//进行旋转

		Rect Roirectdst;
		RotatedRect Roirectmindst;
		Mat SecondGray, DstImage;
		threshold(ImgRotate, in2, thresh, 255, cv::THRESH_BINARY);
		closing(in2, in2, 1, 20, 1);
		opening(in2, SecondGray, 40, 260, 1);

		Mat hierarchy;
		findContours(SecondGray, contoursFor2CamPos2, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_NONE);


		int size = contoursFor2CamPos2.size();
#pragma region //找轮廓坐标由小到大排序
		int leftLocation[10];
		int rightLocation[10];
		int YLocation[10];
		int heightList[10];
		for (int i = 0; i < contoursFor2CamPos2.size(); i++)
		{
			Rect  Roirect1 = boundingRect(contoursFor2CamPos2[i]);
			int left, right, Y, height;
			left = Roirect1.x;
			leftLocation[i] = left;

			right = Roirect1.x + Roirect1.width;
			rightLocation[i] = right;

			Y = Roirect1.y;
			YLocation[i] = Y;

			height = Roirect1.height;
			heightList[i] = height;

		}
		sort(leftLocation, leftLocation + size);
		sort(rightLocation, rightLocation + size);
		sort(YLocation, YLocation + size);
		sort(heightList, heightList + size);
#pragma endregion

		int leftX, leftY, regionWidth, regionHeight;
		leftX = (leftLocation[1] + rightLocation[0]) / 2;
		leftY = YLocation[0] - 5;
		regionWidth = rightLocation[size-1] + 5 - leftX;
		regionHeight = heightList[size-1] + 5;
		cv::Rect RoiCutLocation(leftX, leftY, regionWidth, regionHeight);
		DstImage = ImgRotate(RoiCutLocation);
		//imwrite("C:/Users/ATWER/Desktop/右.jpg", DstImage);
		Size PicSize;
		Mat TempImage;
		//PicSize = Size(DstImage.rows, DstImage.rows);
		PicSize = Size(imgsize * 3, imgsize);
		resize(DstImage, TempImage, PicSize);
		if (isFlip)
		{
			cv::flip(TempImage, TempImage, 1);
		}
		memcpy(output, TempImage.data, imgsize*imgsize * 3);


#pragma region //分割图片，传出指针
		Mat FirstImg, SecondImg, ThirdImg, FourthImg, FifthImg, SixthImg;
		int X = (DstImage.cols) / 6;
		Rect RoiCutFirst(0, 0, X, DstImage.rows);
		Rect RoiCutSecond(X, 0, X, DstImage.rows);
		Rect RoiCutThird(2 * X, 0, X, DstImage.rows);
		Rect RoiCutFourth(3 * X, 0, X, DstImage.rows);
		Rect RoiCutFifth(4 * X, 0, X, DstImage.rows);
		Rect RoiCutSixth(5 * X, 0, DstImage.cols - 5 * X, DstImage.rows);

		FirstImg = DstImage(RoiCutFirst);
		SecondImg = DstImage(RoiCutSecond);
		ThirdImg = DstImage(RoiCutThird);
		FourthImg = DstImage(RoiCutFourth);
		FifthImg = DstImage(RoiCutFifth);
		SixthImg = DstImage(RoiCutSixth);

		PicSize = Size(imgsize / 2, imgsize);
		resize(FirstImg, FirstImg, PicSize);
		resize(SecondImg, SecondImg, PicSize);
		resize(ThirdImg, ThirdImg, PicSize);
		resize(FourthImg, FourthImg, PicSize);
		resize(FifthImg, FifthImg, PicSize);
		resize(SixthImg, SixthImg, PicSize);

		memcpy(output1, FirstImg.data, imgsize*imgsize / 2);
		memcpy(output2, SecondImg.data, imgsize*imgsize / 2);
		memcpy(output3, ThirdImg.data, imgsize*imgsize / 2);
		memcpy(output4, FourthImg.data, imgsize*imgsize / 2);
		memcpy(output5, FifthImg.data, imgsize*imgsize / 2);
		memcpy(output6, SixthImg.data, imgsize*imgsize / 2);
#pragma endregion
		return 1;
	}
	catch (Exception e)
	{
		return 0;
	}
}
















vector<vector<cv::Point2i>> contours;
vector<vector<cv::Point2i>> contourFor2CamPos;
APP_API int ImageCuter(int thresh, BYTE* src, int CamNum, int CamIndex, int w, int h, int channel, int imgsize, unsigned char *output1, unsigned char *output2, unsigned char *output3, unsigned char *output)
{
	int aa1 = 100;

	try
	{
		Mat in;
		Bytes2Mat(src, w, h, channel, in);
		//in = imread("F:/串EL资料/图片/rawZ4/2__2H.jpg", 0);
		Rect RoiCut(500, in.rows / 5, in.cols - 1000, in.rows * 3 / 5);
		in = in(RoiCut);
		Rect Roirect;
		RotatedRect Roirectmin;
		Mat FirstGray;
		in.copyTo(FirstGray);
		threshold(FirstGray, FirstGray, thresh, 255, cv::THRESH_BINARY);
		closing(FirstGray, FirstGray, 3, 5, 2);
		opening(FirstGray, FirstGray, 40, 10, 1);


		findContours(FirstGray, contourFor2CamPos, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_NONE);
		double maxarea = 0;
		int maxAreaIdx = 0;
		for (int index = 0; index <= contourFor2CamPos.size() - 1; index++)
		{
			double tmparea = fabs(contourArea(contourFor2CamPos[index]));
			if (tmparea>maxarea)
			{
				maxarea = tmparea;
				maxAreaIdx = index;//记录最大轮廓的索引号
			}
		}
		//for ( int index = contourFor2CamPos1.size() - 1; index >= 0; index--)
		//{
		//	double tmparea =  fabs(contourArea(contourFor2CamPos1[index]));
		//	if (tmparea>maxarea)
		//	{
		//		maxarea = tmparea;
		//		maxAreaIdx = index;//记录最大轮廓的索引号
		//	}
		//}
		//Rect ee = boundingRect(contourFor2CamPos1[0]);
		RotatedRect  Roirectmin1 = minAreaRect(contourFor2CamPos[maxAreaIdx]);
		//getWhiteRigion(FirstGray, Roirect, Roirectmin);


		double angle = Roirectmin1.angle;
		if (angle < -45)
		{
			angle = angle + 90;
		}
		Mat ImgRotate;//旋转后图像
		Mat in2;
		Point2f center(in.cols / 2, in.rows / 2);
		Mat rot = getRotationMatrix2D(center, angle, 1);
		Rect box = RotatedRect(center, in.size(), angle).boundingRect();
		warpAffine(in, ImgRotate, rot, box.size());//进行旋转
		//opening(ImgRotate, in2, 60, 1, 1);

		Rect Roirectdst;
		RotatedRect Roirectmindst;
		Mat SecondGray, DstImage;
		threshold(ImgRotate, in2, thresh, 255, cv::THRESH_BINARY);
		closing(in2, in2, 1, 10, 1);
		opening(in2, SecondGray, 40, 260, 1);
		//getWhiteRigion(SecondGray, Roirectdst, Roirectmindst);
		/*imshow("SecondGray", SecondGray);
		imwrite("C:/Users/ATWER/Desktop/轮廓.jpg", SecondGray);*/
		/*vector<vector<cv::Point2i>> contour;
		vector<cv::Vec4i> hierarchy;*/
		//vector<Mat> contours(100);

		Mat hierarchy;
		findContours(SecondGray, contours, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_NONE);

		Mat ii;

#pragma region //找轮廓坐标由小到大排序
		int leftLocation[7];
		int rightLocation[7];
		int YLocation[7];
		int heightList[7];
		for (int i = 0; i < contours.size(); i++)
		{
			Rect  Roirect1;
			Roirect1 = boundingRect(contours[i]);
			int left, right, Y, height;
			left = Roirect1.x;
			leftLocation[i] = left;

			right = Roirect1.x + Roirect1.width;
			rightLocation[i] = right;

			Y = Roirect1.y;
			YLocation[i] = Y;

			height = Roirect1.height;
			heightList[i] = height;

		}
		sort(leftLocation, leftLocation + 7);
		sort(rightLocation, rightLocation + 7);
		sort(YLocation, YLocation + 7);
		sort(heightList, heightList + 7);
#pragma endregion

		if (CamNum == 2 && CamIndex == 1)//半片两个相机，右边
		{

			//imwrite("C:/Users/ATWER/Desktop/11111.jpg", SecondGray);
			int leftX, leftY, regionWidth, regionHeight;
			leftX = (leftLocation[1] + rightLocation[0]) / 2;
			leftY = YLocation[0] - 5;
			regionWidth = rightLocation[6] + 5 - leftX;
			regionHeight = heightList[6] + 5;
			cv::Rect RoiCutLocation(leftX, leftY, regionWidth, regionHeight);
			DstImage = ImgRotate(RoiCutLocation);
			//imwrite("C:/Users/ATWER/Desktop/右.jpg", DstImage);
			Size PicSize;
			Mat TempImage;
			//PicSize = Size(DstImage.rows, DstImage.rows);
			PicSize = Size(imgsize * 3, imgsize);
			resize(DstImage, TempImage, PicSize);
			memcpy(output, TempImage.data, imgsize*imgsize * 3);

		}

		if (CamNum == 2 && CamIndex == 2)//半片两个相机，左边
		{


			int leftX, leftY, regionWidth, regionHeight;
			leftX = leftLocation[0] - 5;
			leftY = YLocation[0] - 5;
			regionWidth = (rightLocation[5] + leftLocation[6]) / 2 - leftX;
			regionHeight = heightList[6] + 5;
			Rect RoiCutLocation(leftX, leftY, regionWidth, regionHeight);
			DstImage = ImgRotate(RoiCutLocation);
			//imwrite("C:/Users/ATWER/Desktop/左.jpg", DstImage);
			Size PicSize;
			Mat TempImage;
			//PicSize = Size(DstImage.rows, DstImage.rows);
			PicSize = Size(imgsize * 3, imgsize);
			resize(DstImage, TempImage, PicSize);
			memcpy(output, TempImage.data, imgsize*imgsize * 3);

		}
#pragma region //分割图片，传出指针
		Mat FirstImg, SecondImg, ThirdImg;
		int X = (DstImage.cols) / 3;
		Rect RoiCutFirst(0, 0, X, DstImage.rows);
		Rect RoiCutSecond(X, 0, X, DstImage.rows);
		Rect RoiCutThird(2 * X, 0, DstImage.cols - 2 * X, DstImage.rows);
		FirstImg = DstImage(RoiCutFirst);
		SecondImg = DstImage(RoiCutSecond);
		ThirdImg = DstImage(RoiCutThird);
		Mat qq;
		Size PicSize;
		//PicSize = Size(DstImage.rows, DstImage.rows);
		PicSize = Size(imgsize, imgsize);
		resize(FirstImg, FirstImg, PicSize);
		resize(SecondImg, SecondImg, PicSize);
		resize(ThirdImg, ThirdImg, PicSize);
		/*	imwrite("C:/Users/ATWER/Desktop/第一张.jpg", FirstImg);
		imwrite("C:/Users/ATWER/Desktop/第二张.jpg", SecondImg);
		imwrite("C:/Users/ATWER/Desktop/第三张.jpg", ThirdImg);*/


		/*IplImage aa;
		aa = DstImage;*/

		/*width = DstImage.cols;
		height = DstImage.rows;*/
		memcpy(output1, FirstImg.data, imgsize*imgsize);
		memcpy(output2, SecondImg.data, imgsize*imgsize);
		memcpy(output3, ThirdImg.data, imgsize*imgsize);
#pragma endregion
		//	Mat FirstImg, SecondImg, ThirdImg;
		//	int X = (DstImage.cols) / 3;
		//	Rect RoiCutFirst(0, 0, X, DstImage.rows);
		//	Rect RoiCutSecond(X, 0, X, DstImage.rows);
		//	Rect RoiCutThird(2 * X, 0, DstImage.cols - 2 * X, DstImage.rows);
		//	FirstImg = DstImage(RoiCutFirst);
		//	SecondImg = DstImage(RoiCutSecond);	
		//	ThirdImg = DstImage(RoiCutThird);
		//	Mat qq;
		//	Size PicSize;
		//	//PicSize = Size(DstImage.rows, DstImage.rows);
		//	PicSize = Size(imgsize, imgsize);
		//	resize(FirstImg, FirstImg, PicSize);
		//	resize(SecondImg, SecondImg, PicSize);
		//	resize(ThirdImg, ThirdImg, PicSize);
		///*	imwrite("C:/Users/ATWER/Desktop/第一张.jpg", FirstImg);
		//	imwrite("C:/Users/ATWER/Desktop/第二张.jpg", SecondImg);
		//	imwrite("C:/Users/ATWER/Desktop/第三张.jpg", ThirdImg);*/
		//
		//	
		//	/*IplImage aa;
		//	aa = DstImage;*/
		//	
		//	/*width = DstImage.cols;
		//	height = DstImage.rows;*/
		//	memcpy(output1, FirstImg.data, imgsize*imgsize);
		//	memcpy(output2, SecondImg.data, imgsize*imgsize);
		//	memcpy(output3, ThirdImg.data, imgsize*imgsize);
		//vector<uchar>buf;
		//读入图片
		//imencode(".bmp", DstImage, buf);   //将Mat以BMP格式存入uchar的buf数组中
		//int size = buf.size();
		//BYTE* output = new BYTE[size];
		//for each(uchar var in buf)    //将buf拷贝到C#的byte[]中
		//{
		//	*output  = var;
		//	output++;
		//}
		//Mat2Bytes(output, DstImage, size);
		//Getmat(DstImage, output, size);
	}
	catch (Exception e)
	{

	}
	return aa1;
}

void MatToByte(Mat srcImg, BYTE*& pImg)
{
	int nFlag = srcImg.channels() * 8;//一个像素的bits
	int nHeight = srcImg.rows;
	int nWidth = srcImg.cols;

	int nBytes = nHeight * nWidth * nFlag / 8;//图像总的字节
	if (pImg)
		delete[] pImg;
	pImg = new BYTE[nBytes];//new的单位为字节
	memcpy(pImg, srcImg.data, nBytes);//转化函数,注意Mat的data成员	
}
Capp::Capp()
{
	return;
}

-- by coding -huanghl
