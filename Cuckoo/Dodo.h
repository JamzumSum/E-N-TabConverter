#pragma once
#include "type.h"
#include "cv.h"

#define SUCCEED(x) (x>=0)

using cv::Mat;
using cv::Vec4i;
using std::vector;
using std::string;

//Dodo.cpp
Mat threshold(cv::Mat);
Mat perspect(Mat img, int width, int height);
bool isEmptyLine(Mat img, int y, double rate);
bool isEmptyLine(Mat img, int y, size_t from, size_t to, double rate);
Mat trim(Mat img, double threshold = 0.01);
unsigned whichLine(cv::Rect character, const vector<Vec4i>& rows);
void saveNums(string folder, vector<Mat> nums);
unsigned whichLine(cv::Range verticalRange, const vector<Vec4i>& rows);
Mat Morphology(Mat img, int len, bool horizontal, bool open);
void savePic(string folder, Mat pic);
//framework.cpp
extern int cut(Mat img, vector<Vec4i> divideBy, int direction, vector<Mat> &container, bool includeAll);
