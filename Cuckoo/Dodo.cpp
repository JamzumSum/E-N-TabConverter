#include "Dodo.h"
/*
	Dodo.cpp包含了几乎所有的图像处理功能.
*/
#pragma once
#include "Dodo.h"
#include "pch.h"
#include "Dodo.h"
#include "tools.h"

using namespace cv;
using namespace std;

#define adaptive 1
#define optimize 1
#if _DEBUG
#define dododebug 0								//0-draw nothing    1-draw rows    2-draw cols    3-draw rows and cols
#define imdebug(title, img) imshow((title), img); cv::waitKey()
#else
#define imdebug(title, img)
#endif
//针对最常见的debug，即乐谱的结构划分，dodo提供了置于头部的调试开关，调整开关即可调试对应部分。

//#define len(x1,y1,x2,y2) ((x1)-(x2))*((x1)-(x2)) + ((y1)-(y2))*((y1)-(y2))
template<typename T1, typename T2, typename T3, typename T4, typename T5, typename T6>
constexpr auto inALine(T1 x1, T2 y1, T3 x2, T4 y2, T5 x3, T6 y3) { return (((x1) - (x2)) * ((y3) - (y2)) == ((x3) - (x2)) * ((y1) - (y2))); }

/*
	横向或纵向的形态学处理
	@name Morphology
	@param len			int, 形态学处理的单元长度
	@param horizontal	bool, 为真时水平结构，为假时竖直结构
	@param open			bool, 为真时开操作，为假时闭操作
*/
Mat Morphology(Mat img, int len, bool horizontal, bool open) {
	Mat r;
	Mat kernel = getStructuringElement(MORPH_RECT, horizontal
			? Size(max(len, 1), 1)													//水平结构
			: Size(1, max(len, 1))); 												//竖直结构
	morphologyEx(img, r, open ? MORPH_CLOSE : MORPH_OPEN, kernel);
	return r;
}

Mat threshold(Mat pic){
	/*
	 * 函数：threshold
	 * 功能：读取指定路径的图片，并将其二值化
	*/
	Mat r;
	//blocksize-5:960 7:1860 7:2480
	int blocksize = 2 * (pic.cols / 1500 + 2) + 1;
	adaptiveThreshold(pic, r, 255, ADAPTIVE_THRESH_MEAN_C, CV_THRESH_BINARY, blocksize, 17);
	return r;
}
/*
	 * @name isEmptyLine 
	 * @brief 测试一行是否是空白行,返回布尔值
	 * @param[in] rate, 越小，对白色范围要求越严格
	 * @retval 当 y>=img.rows 或 <0 时判断为空行. 
	*/
bool isEmptyLine(Mat img, int y, double rate) {
	return isEmptyLine(img, y, 0, img.cols, rate);
}
/*
 * @name isEmptyLine
 * @brief 测试一行是否是空白行,返回布尔值
 * @param[in] thrshold 越大，对“白”要求越严格
 * @param[in] rate 越小，对白色范围要求越严格
*/
bool isEmptyLine(Mat img, int y, size_t from, size_t to, double rate) {
	size_t sum = 0;
	size_t w = from, n = min(to, (size_t)img.cols);
	if (y >= img.rows || y < 0) return true;

	uchar *ptr = img.ptr<uchar>(y);
//#pragma simd
	for (; w < n; w++) {
		sum += !ptr[w];
	}

	auto cmp = (double)sum / img.cols;
	if (cmp > rate && cmp < 1 - rate) return false;
	else return true;
}
/*
 * @name isEmptyCol
 * @brief 测试一列是否是空白列,返回布尔值
 * @param[in] thrshold 越大，对“白”要求越严格
 * @param[in] rate 越小，对白色范围要求越严格
*/
bool isEmptyCol(Mat img, int x, double rate) {
	int sum = 0;
	if (x >= img.cols) return true;
	for (int y = 0; y < img.rows; y++)
	{
		uchar *ptr = img.ptr<uchar>(y);
		if (!ptr[x]) sum++;
	}
	auto cmp = (double)sum / img.rows;
	if (cmp > rate && cmp < 1 - rate) return false;
	else return true;
}
/*
 * @name trim
 * @brief 去掉图像周围的白/黑边
 * @param[in] img, 应当是二值化的. 
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

/*
	将一个Mat保存到硬盘
	@name savePic
	@see cv::imwrite
*/
void savePic(string folder,Mat pic) {
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
		i = perspect(i, 8, 14);
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
/**
	判断字符在哪条直线上
	@name	whichLine
	@retval 0 if no line. else 1-6. 
*/
unsigned whichLine(Rect numRect, const vector<Vec4i>& rows) {
	assert(rows.size() == 6);
	auto through = [](Rect character, int row) -> bool {
		return character.y < row&& row < character.br().y;
	};

	for (size_t i = 0; i < 5; i++) {
		if (through(numRect, rows[i][1])) {
			if (through(numRect, rows[i + 1][1]))  return 0;
			else return static_cast<unsigned>(i + 1);
		}
	}
	if (through(numRect, rows[5][1])) return 6;
	return 0;
}

/**
	判断字符在哪条直线上
	@name	whichLine
	@retval 0 if no line. else 1-6.
*/
unsigned whichLine(Range verticalRange, const vector<Vec4i>& rows) {
	assert(rows.size() == 6);
	auto through = [](Range character, int row) -> bool {
		return character.start < row && row < character.end;
	};

	for (size_t i = 0; i < 5; i++) {
		if (through(verticalRange, rows[i][1])) {
			if (through(verticalRange, rows[i + 1][1]))  return 0;
			else return static_cast<unsigned>(i + 1);
		}
	}
	if (through(verticalRange, rows[5][1])) return 6;
	return 0;
}

int cut(Mat img, vector<Vec4i> divideBy, int direction, vector<Mat>& container, bool includeAll) {
	//direction: 0是竖直裁剪, 1是水平裁剪
	if (direction > 1) return 0;
	int from = (int)container.size();
	switch (direction) {
	case 0:
		//竖直裁剪

		if (includeAll) {
			if (divideBy[0][0] > 5) {
				Vec4i h = { 0,0,0,img.rows };
				divideBy.insert(divideBy.begin(), h);
			}
			if (divideBy.back()[0] < img.cols - 5) {
				Vec4i h = { img.cols,0,img.cols,img.rows };
				divideBy.insert(divideBy.begin() + divideBy.size(), h);
			}
		}
		for (int i = 0; i < (int)divideBy.size() - 1; i++) {
			Mat tmp;
			img(Range::all(), Range(divideBy[i][0], divideBy[i + 1][0])).copyTo(tmp);
			container.push_back(tmp);
		}
		break;
	case 1:
		//水平裁剪

		if (includeAll) {
			if (divideBy[0][1] > 5) {
				Vec4i h = { 0,0,img.cols,0 };
				divideBy.insert(divideBy.begin(), h);
			}
			if (divideBy.back()[1] < img.rows - 5) {
				Vec4i h = { 0,img.rows,img.cols,img.rows };
				divideBy.insert(divideBy.begin() + divideBy.size(), h);
			}
		}
		for (int i = 0; i < (int)divideBy.size() - 1; i++) {
			Mat tmp;
			img(Range(divideBy[i][1], divideBy[i + 1][1]), Range::all()).copyTo(tmp);
			container.push_back(tmp);
		}
	}
	return (int)container.size() - from;
}

/*
	recognize an arc. get its start/end point, direction
	@param pSet, the set of the points. maybe from cv::findContours. 
		requires:	pSet.size() > 10.	
					pSet is x-continuous. 
	@return {0,0,0,0} if not an arc. else start point and end point (in Vec4i). 
	@abort	if pSet.size() < 10. 
	@complexity		O(nlogn).
	@thread safe
*/
const pair<Vec4i, double> recArc(vector<cv::Point> pSet, const cv::Point& offset) {
	sort(pSet.begin(), pSet.end(), 
		[](const Point& x, const Point& y) {return x.x < y.x || (x.x == y.x && x.y < y.y); }
	);			//sort. upper points in front. 

	auto it_end = unique(pSet.begin(), pSet.end(), [](const Point& x, const Point& y) {return x.x == y.x; });
	pSet.resize(distance(pSet.begin(), it_end));				//remove repeative points. 

	assert(pSet.size() >= 10);
	if (pSet.size() != 1 + pSet.back().x - pSet.begin()->x) return make_pair(Vec4i{0, 0, 0, 0}, 0);
	
	auto lp = pSet.begin() + 2;
	auto rp = pSet.end() - 3;					//calculate radius and center through lp and rp. 
	auto mp = pSet.begin() + pSet.size() / 2;
	bool upward = mp->y > pSet.back().y;

	auto normalSlope = [&pSet, upward](vector<Point>::iterator it) -> double {
		auto lp = it - 2, rp = it + 2;
		return rp->y == lp->y ? (upward ? -FLT_MAX : FLT_MAX) :
			static_cast<double>(lp->x - rp->x) / (rp->y - lp->y);			// -x / y
	};

	double kl = normalSlope(lp), kr = normalSlope(rp);		//slope of the normal. 
	double dk = kl - kr;						//k1 - k2
	double x = ((rp->y - lp->y) + (kl * lp->x - kr * rp->x)) / dk;
	double y = (kl * rp->y - kr * lp->y + kl * kr * (lp->x - rp->x)) / dk;		//center

	double r = sqrt(pow(x - lp->x, 2) + pow(y - lp->y, 2));						//radius

	auto pred = [x, y, r, &pSet, normalSlope](vector<Point>::iterator it) -> bool {
		double k = normalSlope(it);
		double theta = atan(k);
		double rx = x + cos(theta), ry = y + sin(theta);
		double d = sqrt(pow(x - rx, 2) + pow(y - ry, 2));
		return d * 4 < r;			//d < r / 4. 
	};

	for (auto i = lp + 2; i + 1 != pSet.end(); i += 2) {
		if (!pred(i)) return make_pair(Vec4i{0, 0, 0, 0}, 0);
	}

	double km = normalSlope(mp);
	return make_pair(Vec4i{offset.x + pSet.begin()->x, offset.y + pSet.begin()->y, 
		offset.x + pSet.back().x, offset.y + pSet.back().y }, km);
}