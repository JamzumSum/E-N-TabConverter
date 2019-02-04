/*
	Dodo.cpp包含了几乎所有的图像处理功能.
*/
#pragma once
#include "Dodo.h"
#include "tools.h"
#include "highgui.h"
#include "global.h"
#include "opencv.hpp"
#include "imgproc.hpp"

#define adaptive 1
#define optimize 1
#if _DEBUG
#define dododebug 0								//0-draw nothing    1-draw rows    2-draw cols    3-draw rows and cols
#endif
//针对最常见的debug，即乐谱的结构划分，dodo提供了置于头部的调试开关，调整开关即可调试对应部分。

#define len(x1,y1,x2,y2) ((x1)-(x2))*((x1)-(x2)) + ((y1)-(y2))*((y1)-(y2))
#define inALine(x1,y1,x2,y2,x3,y3) (((x1) - (x2)) * ((y3) - (y2)) == ((x3) - (x2)) * ((y1) - (y2)))

extern Mat Denoise(cv::Mat img, std::vector<cv::Vec4i> lines, double radius);

/*
	函数名: Morphology
	功能: 横向或纵向的形态学处理
	特殊参数:
		int len，形态学处理的单元长度
		bool horizontal 为真时水平结构，为假时竖直结构
		bool open 为真时开操作，为假时闭操作
*/
Mat Morphology(Mat img, int len, bool horizontal, bool open) {
	Mat dilated, eroded;
	Mat kernel = getStructuringElement(MORPH_RECT, horizontal
			? Size(max(len, 1), 1)													//水平结构
			: Size(1, max(len, 1))); 												//竖直结构
	if (open) {
		dilate(img, dilated, kernel);												//膨胀
		erode(dilated, eroded, kernel);												//腐蚀
		return eroded;
	}
	else {
		erode(img, eroded, kernel);													//腐蚀
		dilate(eroded, dilated, kernel);											//膨胀
		return dilated;
	}
}
/*
函数名：isDotLine
功能：判断一条直线是否为虚线
*/
bool isDotLine(Mat img, Vec4i line) {
	auto y = [line](int x) -> int {
		return line[1] + (line[3] - line[1]) / (line[2] - line[0]) * (x - line[0]);
	};
	int sum = 0;
	for (int i = line[0]; i <= line[2]; i++) {
		if (!img.at<uchar>(y(i),i)) {
			sum++;
		}
	}
	if (sum < 0.7 * (line[2] - line[0])) {
		return true;
	}
	return false;
}

Mat threshold(string picPath)
{
	/*
	 * 函数：threshold
	 * 功能：读取指定路径的图片，并将其二值化
	*/
	Mat arigin, r;
	arigin = imread(picPath.c_str(), 0);
	r = arigin.clone();
#if adaptive
	//blocksize-5:960 7:1860 7:2480
	int blocksize = 2 * (r.cols / 1500 + 2) + 1;
	adaptiveThreshold(arigin, r, 255, ADAPTIVE_THRESH_MEAN_C, CV_THRESH_BINARY, blocksize,17);
#else
	threshold(arigin, r, 210, 255, CV_THRESH_BINARY);				//希望尽量小的参数3
#endif
	return r;
}

bool isEmptyLine(Mat img, int y, double rate) {
	/*
	 * @name isEmptyLine 
	 * @brief 测试一行是否是空白行,返回布尔值
	 * @param[in] rate, 越小，对白色范围要求越严格
	 * @retval 当 y>=img.rows 或 <0 时判断为空行. 
	*/
#if optimize
	//3*3 optimize, csapp chapter4
	size_t sum1 = 0, sum2 = 0, sum3 = 0;
	if (y >= img.rows || y < 0) return true;
	uchar *ptr = img.ptr<uchar>(y);
	size_t w = 0;
	for (; w < img.cols - 2; w += 3)
	{
		sum1 += !ptr[w];
		sum2 += !ptr[w + 1];
		sum3 += !ptr[w + 2];
	}
	sum1 += sum2 + sum3;
	for (; w < img.cols; w++) sum1 += !ptr[w];

	if ((double)sum1 / img.cols > rate) return false;
	else return true;
#else
	size_t sum = 0;
	if (y >= img.rows || y < 0) return true;
	uchar *ptr = img.ptr<uchar>(y);
	for (int w = 0; w < img.cols; w++)
	{
		sum += !ptr[w];
	}
	if ((double)sum / img.cols > rate) return false;
	else return true;
#endif
}

bool isEmptyLine(Mat img, int y, size_t from, size_t to,double rate) {
	//函数名 isEmptyLine 
	//功能 测试一行是否是空白行,返回布尔值
	//特殊参数：
	//	thrshold 越大，对“白”要求越严格
	//	rate 越小，对白色范围要求越严格
#if optimize
	//3*3 optimize
	size_t sum1 = 0, sum2 = 0, sum3 = 0;
	size_t w = from, n = min(to, (size_t)img.cols);
	if (y >= img.rows || y < 0) return true;

	uchar *ptr = img.ptr<uchar>(y);
	for (; w < n - 2; w += 3) {
		sum1 += !ptr[w];
		sum2 += !ptr[w + 1];
		sum3 += !ptr[w + 2];
	}
	sum1 += sum2 + sum3;
	for (; w < n; w++) sum1 += !ptr[w];

	if ((double)sum1 / img.cols > rate) return false;
	else return true;
#else
	int sum = 0,n = min(to,img.cols);
	if (y >= img.rows) return true;
	uchar *ptr = img.ptr<uchar>(y);
	for (int w = from; w < n; w++) {
		if (!ptr[w]) sum++;
	}
	if ((double)sum / img.cols > rate) return false;
	else return true;
#endif
}

bool isEmptyCol(Mat img, int x, double rate) {
	//函数名 isEmptyCol
	//功能 测试一列是否是空白列,返回布尔值
	//特殊参数：
	//	thrshold 越大，对“白”要求越严格
	//	rate 越小，对白色范围要求越严格
	int sum = 0;
	if (x >= img.cols) return true;
	for (int y = 0; y < img.rows; y++)
	{
		uchar *ptr = img.ptr<uchar>(y);
		if (!ptr[x]) sum++;
	}
	if ((double)sum / img.rows > rate) return false;
	else return true;
}
/*
 * @name trim
 * @brief 去掉图像周围的白边
 * @param[in] threshold, 当黑点占一行（列）比例小于此double值时，被视为噪点
*/
Mat trim(Mat img, double threshold) {
	int i;
	int upper = -1;
	for (i = 0; i < img.rows; i++) if (!isEmptyLine(img, i, threshold)) {
		upper = i;
		break;
	}
	if (upper < 0) return Mat();
	int lower = upper;
	for (i = img.rows - 1; i > upper ; i--) if (!isEmptyLine(img, i, threshold)) {
		lower = i + 1;
		break;
	}
	if (lower == upper) return Mat();
	Mat ROI = img(Range(upper, lower),Range(0, img.cols)).clone();

	for (i = 0; i < img.cols; i++) if (!isEmptyCol(ROI, i, threshold)) {
		upper = i;
		break;
	}
	lower = upper;
	for (i = img.cols - 1; i >upper; i--) if (!isEmptyCol(ROI, i, threshold)) {
		lower = i + 1;
		break;
	}
	return ROI(Range(0, ROI.rows), Range(upper, lower)).clone();
}

void findRow(Mat img,Mat &Outputimg, double rangeTheta,vector<Vec4i> &lines,vector<int> &thickness) {
	/*
	* 函数：findRow
	* 功能：概率霍夫变换提取横线
	* 特殊参数：rangeTheta，直线偏离水平方向的最大角度
	* thickness，算得直线的最大宽度
	* lines，筛选后的结果
	*/

	//Mat dilated, eroded;
	//Mat hline = getStructuringElement(MORPH_RECT, Size(max(img.cols / 120, 1),1));						//竖直结构
	//erode(img, eroded, hline);																			//腐蚀
	//dilate(eroded, dilated, hline);																		//膨胀
	//Mat inv = 255 - dilated;
	HoughLinesP(255-img, lines, 1, CV_PI / 180, 100, img.cols * 2 / 3, img.cols/50);
	  //角度筛选
	for (size_t i = 0; i < lines.size(); i++) {
		if (lines[i][2] == lines[i][0]) lines.erase(lines.begin() + i--);
		else if (abs(((float)lines[i][3] - lines[i][1]) / (lines[i][2] - lines[i][0])) > tan(rangeTheta)) {
			lines.erase(lines.begin() + i--);
			continue;
		}
	}
	sort(lines.begin(), lines.end(), [](const Vec4i x, const Vec4i y) ->bool {
		return x[1] < y[1];
	});
	if (!lines.size()) return;
	//去除虚线
	for (int i = 0; i < lines.size(); i++) {
		if (isDotLine(img, lines[i])) {
			lines.erase(lines.begin() + i);
		}
	}
	//去除连续
	vector<Vec4i> copy(lines);
	for (size_t i = 0; i < lines.size(); i++) {
		if (i == lines.size() - 1) {
			thickness.push_back(1);
			break;
		}
		if (lines[i][1] + 1 >= lines[i + 1][1]) {
			size_t st = i, ed = i+1;
			for (size_t j = i + 1; j < lines.size() - 1; j++) {
				if (lines[j][1] + 1 >= lines[j + 1][1]) {
					ed = j + 1;
				}
				else {
					break;
				}
			}
			size_t d = (st + ed) / 2;
			thickness.emplace_back(int(ed - st + 1));
			for (int j = (int)ed; j >= (int)st; j--) {
				if (j == d) continue;
				lines.erase(lines.begin()+j);
			}
		}
		else thickness.push_back(1);
	}
	for (int i : thickness) global->lineThickness += i;
	Outputimg = Denoise(img, copy, *max_element(thickness.begin(), thickness.end()) / 2.0);
#if dododebug % 2
	Mat color;
	cvtColor(img, color, CV_GRAY2BGR);
	for (size_t i = 0; i < lines.size(); i++) {
		line(color,Point(lines[i][0], lines[i][1]),Point(lines[i][2], lines[i][3]),Scalar(0,0,255));
	}
	imshow("2", color); cvWaitKey();
#endif
}

void findCol(Mat img, double rangeTheta, int upper,int lower, vector<int> thick, vector<Vec4i> &lines) {
	/*
	 * 函数：findCol
	 * 功能：概率霍夫变换提取竖线
	 * 特殊参数：rangeTheta，直线偏离竖直方向的最大角度
	 * upper，检测到的横线的上界；lower，检测到的横线的下界
	 * thickness，直线的宽度
	 * lines，筛选后的结果
	*/
	int length = upper - lower;
	int thickness = max(1, *max_element(thick.begin(),thick.end()));
	Mat toScan = img(Range(lower,upper), Range::all()).clone();
	Mat inv = 255 - Morphology(toScan, max(toScan.rows / 2, 1), false, true);
	//imshow("2", inv); cvWaitKey();
	//80:140
	HoughLinesP(inv, lines, 1, CV_PI / 180, (toScan.rows - 2 * thickness) / 2, toScan.rows - 2 * thickness, thickness + 2);
	for (size_t i = 0; i < lines.size(); i++) {
		if (lines[i][2] == lines[i][0]) continue;
		else if (abs(((float)lines[i][3] - lines[i][1]) / (lines[i][2] - lines[i][0])) < tan(rangeTheta))
			lines.erase(lines.begin() + i--);
	}
	sort(lines.begin(), lines.end(), [](const Vec4i x,const Vec4i y) ->bool {
		return x[0] < y[0];
	});
	//去除连续
	for (int i = 0; i < (int)lines.size() - 1; i++) {
		if (lines[i][0] + 1 >= lines[i + 1][0]) {
			size_t st = i, ed = i + 1;
			for (int j = i + 1; (int)j < lines.size() - 1; j++) {
				if (lines[j][0] + 1 >= lines[j + 1][0]) {
					ed = j + 1;
				}
				else {
					break;
				}
			}
			size_t d = (st + ed) / 2;
			Vec4i tmp = lines[d];
			lines.erase(lines.begin() + st, lines.begin() + ed + 1);
			lines.insert(lines.begin() + st, tmp);
		}
	}
	for (size_t i = 1; i < lines.size(); i++) {
		if (lines[i][0] - lines[i - 1][0] >= max(16,global->colLenth / 5)) {
			global->colLenth += lines[i][0] - lines[i - 1][0];
		}
	}
#if dododebug == 2
	Mat color;
	cvtColor(img, color, CV_GRAY2BGR);
	for (size_t i = 0; i < lines.size(); i++) {
		line(color, Point(lines[i][0], 0), Point(lines[i][2], img.rows), Scalar(0, 0, 255));
	}
	imshow("2", color); waitKey(); cvWaitKey();
#endif
}

void savePic(string folder,Mat pic) {
	/*
	 * 函数：sacePic
	 * 功能：将一个矩阵保存到硬盘
	*/
	
	makedir(folder);
	static int num = 0;
	char c[8] = {0,0,0,0,0,0,0,0};
	_itoa_s(num++, c, 10);
	imwrite(folder + "\\" + c + ".jpg",pic);
}

Mat perspect(Mat img, int width, int height) {
	/*
	* 函数：perspect
	* 功能：借助透视变换拉伸图片
	* 特殊参数：width, 目标宽度；height，目标高度
	*/
	if (img.cols == width && img.rows == height) return img;
	cv::Mat r;

	std::vector<cv::Point2f> scorners(4);
	std::vector<cv::Point2f> dcorners(4);

	scorners[0] = cv::Point2f(0, 0);
	scorners[1] = cv::Point2f((float)img.cols - 1, 0);
	scorners[2] = cv::Point2f(0, (float)img.rows - 1);
	scorners[3] = cv::Point2f((float)img.cols - 1, (float)img.rows);

	dcorners[0] = cv::Point2f(0, 0); dcorners[1] = cv::Point2f((float)width, 0);
	dcorners[2] = cv::Point2f(0, (float)height); dcorners[3] = cv::Point2f((float)width, (float)height);

	cv::Mat tmp = cv::getPerspectiveTransform(scorners, dcorners);
	cv::warpPerspective(img, r, tmp, cv::Size(width, height));

	return r;
}

void saveNums(string folder, vector<Mat> nums) {
	/*
	 * 函数：saveNums
	 * 功能：将nums中的图片保存到folder
	*/
	cout << "开始保存图片" << endl;
	for (Mat& i : nums) {
		if (i.cols != 8 || i.rows != 10) i = perspect(i, 8, 10);
		int sum = 0;
		for (int j = 0; j < i.rows; j++) {
			uchar *ptr = i.ptr<uchar>(j);
			for (int w = 0; w < i.cols; w++)
			{
				if (!ptr[w]) sum++;
			}
		}
		if (sum > 8 && sum < 48) savePic(folder, i);
	}
	cout << "图片保存完成" << endl;
}
/*
	函数名：whichLine
	功能：判断字符在哪条直线上
*/
unsigned whichLine(Rect number, vector<Vec4i> rows) {
	auto through = [](Rect character, int row) -> bool {
		return character.y < row&& row < character.br().y;
	};

	for (int i = 0; i < 5; i++) {
		if (through(number, rows[i][1])) {
			if (through(number, rows[i+1][1]))  return 0;
			else return i + 1;
		}
	}
	if (through(number, rows[5][1])) return 6;
	return 0;
}


