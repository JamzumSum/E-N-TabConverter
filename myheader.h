#pragma once
#include"type.h"

#define workMode 0
/*
	* workMode
	|value	|function		|
	|:-----:|:-------------:|
	|0		|main program	|
	|1		|trian			|
*/

#define SUCCEED(x) (x>=0)
#define defaultCSV "C:\\Users\\Administrator\\Desktop\\E-N TabConverter\\tData.csv"

//Dodo.cpp
extern cv::Mat threshold(std::string);
extern cv::Mat perspect(cv::Mat img, int width, int height);
extern bool isEmptyLine(cv::Mat img, int y, double rate);
extern bool isEmptyLine(cv::Mat img, int y, int from, int to, double rate);
extern cv::Mat trim(cv::Mat img, double threshold = 0.01);
extern void findRow(cv::Mat img, double rangeTheta, std::vector<cv::Vec4i> &lines, std::vector<int> &thickness);
extern void findCol(cv::Mat img, double rangeTheta, int upper,int lower, std::vector<int> thick , std::vector<cv::Vec4i> &lines);
extern int whichLine(cv::Vec4i character, std::vector<cv::Vec4i> rows);
extern void saveNums(std::string folder, std::vector<cv::Mat> nums);
extern void KClassify(std::vector<space> collection,std::vector<bool> &classifier);
extern void interCheck(std::vector<space> &collection,std::vector<int> &f);
extern cv::Mat Morphology(cv::Mat img, int len, bool horizontal, bool open);
extern void savePic(std::string folder, cv::Mat pic);
//eagle.cpp
extern int rec(cv::Mat character, std::vector<int> &possible);
extern void train(std::string save = "C:\\Users\\Administrator\\Desktop\\E-N TabConverter\\tData.csv");
//framework.cpp
extern int cut(cv::Mat img, std::vector<cv::Vec4i> divideBy, int direction, std::vector<cv::Mat> &container, bool includeAll = true);
extern int split(cv::Mat img, std::vector<space> &coll);
extern void extractNum(std::vector<cv::Vec4i> &pos, std::vector<cv::Mat> &nums, std::vector<cv::Mat> section, std::vector<cv::Vec4i> rows,int &bottom,int range);
//maincpp.cpp
void fname(const char* path,char* name);
//tools.cpp
extern std::string getPath();
extern void fname(const char* path, char* name);