#pragma once
#include "type.h"
#include "cv.h"

#define SUCCEED(x) (x>=0)
#define cfgPath "C:\\Users\\Administrator\\Desktop\\E-N TabConverter\\global.cfg"

using namespace cv;
using namespace std;

//Dodo.cpp
extern Mat threshold(string);
extern Mat perspect(Mat img, int width, int height);
extern bool isEmptyLine(Mat img, int y, double rate);
extern bool isEmptyLine(Mat img, int y, int from, int to, double rate);
extern Mat trim(Mat img, double threshold = 0.01);
extern void findRow(Mat img, Mat &Outputimg, double rangeTheta, vector<Vec4i> &lines, vector<int> &thickness);
extern void findCol(Mat img, double rangeTheta, int upper,int lower, vector<int> thick , vector<Vec4i> &lines);
extern int whichLine(Vec4i character, vector<Vec4i> rows);
extern void saveNums(string folder, vector<Mat> nums);
extern Mat Morphology(Mat img, int len, bool horizontal, bool open);
extern void savePic(string folder, Mat pic);
//eagle.cpp
extern int rec(Mat character, vector<int> &possible);
extern void train(string save = "C:\\Users\\Administrator\\Desktop\\E-N TabConverter\\tData.csv");
//framework.cpp
extern void extractNum(vector<Vec4i> &pos, vector<Mat> &nums, vector<Mat> section, vector<Vec4i> rows,int &bottom,int range);
extern Mat Denoise(Mat img, vector<Vec4i> lines, double radius);
int cut(Mat img, vector<Vec4i> divideBy, int direction, vector<Mat> &container, bool includeAll);
