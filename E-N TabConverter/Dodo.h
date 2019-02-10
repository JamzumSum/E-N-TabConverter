#pragma once
#include "type.h"
#include "cv.h"

#define SUCCEED(x) (x>=0)
#define cfgPath "C:\\Users\\Administrator\\Desktop\\E-N TabConverter\\global.cfg"

using namespace cv;
using namespace std;

//Dodo.cpp
Mat threshold(cv::Mat);
Mat perspect(Mat img, int width, int height);
bool isEmptyLine(Mat img, int y, double rate);
bool isEmptyLine(Mat img, int y, size_t from, size_t to, double rate);
Mat trim(Mat img, double threshold = 0.01);
void findRow(Mat img, double rangeTheta, vector<Vec4i> &lines, vector<int> &thickness);
void findCol(Mat img, double rangeTheta, int upper,int lower, vector<int> thick , vector<Vec4i> &lines);
unsigned whichLine(Rect character, vector<Vec4i> rows);
void saveNums(string folder, vector<Mat> nums);
Mat Morphology(Mat img, int len, bool horizontal, bool open);
void savePic(string folder, Mat pic);
//eagle.cpp
extern int rec(Mat character, vector<int> &possible, vector<float>& safety);
extern void train(string save = "C:\\Users\\Administrator\\Desktop\\E-N TabConverter\\tData.csv");
//framework.cpp
extern int cut(Mat img, vector<Vec4i> divideBy, int direction, vector<Mat> &container, bool includeAll);
