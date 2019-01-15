#pragma once
#include <vector>
#include "cv.h" 

using namespace std;

#define savepic 0
#define picFolder "C:\\Users\\Administrator\\Desktop\\oh"

enum Value {
	whole = 1,
	half = 2,
	quarter = 4,
	eighth = 8,
	sixteenth = 16,
	thirty_second = 32
};
typedef struct technical {
	int string = 1;									//弦
	int fret = 0;									//品
}technical;

typedef struct notations {
	char* dynamics = (char*)"mf";					//力度
	technical technical;
}notations;

typedef struct note {
	bool chord = false;							//和声标记，为true与上一个音符同时发声
	std::vector<int> possible;					//其他可能的品数
	Value timeValue = whole;					//时值
	bool dot = false;							//附点
	int voice = 1;								//发声类型
	int duration = 1;							//延音值
	notations notation = { (char*)"mf",{1,0} };

	int pos;									//x坐标
}note;

typedef struct Time {
	int beats = 4;								//每小节拍数
	int beat_type = 4;							//几分音符算作一拍
}Time;

typedef struct key {
	int fifth = 0;								//音调升降
	char* mode = (char*)"major";				//1-标准音主序列
}key;

class measure {
private:
	int noteBottom = 0;
	int maxCharacterWidth = 0;
	void recNum(cv::Mat section, vector<cv::Vec4i> rows);
	void recTime(vector<cv::Vec4i> rows);
	cv::Mat org;
public:
	int id;
	unsigned int number = 1;						//小节数
	Time time;
	vector<note> notes;
	key key;
	measure(cv::Mat org, cv::Mat img, vector<cv::Vec4i> rows, int id);
};

typedef struct{
	unsigned start;
	unsigned length;
}space;

class splitter{
public:
	splitter(cv::Mat img);
	void start(vector<cv::Mat>& piece);

private:
	cv::Mat org;
	vector<space> collection;
	int split();
	void interCheck(vector<int> &f);
	void KClassify(vector<bool> &classifier);
};

/*
class Denoiser {
private:
	double radius;
};
*/