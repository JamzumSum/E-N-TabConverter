#pragma once
#include "cv.h"                             
#include "highgui.h"
#include "cvaux.h"
#include "cxcore.h"
#include "opencv2/opencv.hpp"
#include "opencv2/imgproc.hpp"
#include "tinyxml2.h"
#include <iostream>
#include<vector>

typedef struct space {
	int start;
	int length;
}space;

typedef struct err {
	int id;
	int line;
	std::string description;
}err;

enum Value {
	whole = 1,
	half = 2,
	quarter = 4,
	eighth = 8,
	sixteenth = 16,
	thirty_second = 32
};
typedef struct technical {
	int string;									//弦
	int fret;									//品
}technical;

typedef struct notations {
	int dynamics;								//力度
	technical technical;
}notations;

typedef struct note {
	bool chord = false;							//和声标记，为true与上一个音符同时发声
	std::vector<int> possible;					//其他可能的品数
	Value timeValue = whole;					//时值
	bool dot = false;							//附点
	int voice = 1;								//发声类型
	int duration = 1;							//延音值
	notations notation;

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
	void recNum(cv::Mat section, std::vector<cv::Vec4i> rows);
public:
	int id;
	unsigned int number;						//小节数
	Time time;
	std::vector<note> notes;
	key key;
	measure(cv::Mat org, cv::Mat img, std::vector<cv::Vec4i> rows,int id);
};

class saveDoc {
private:
	tinyxml2::XMLDocument backup;
public:
	saveDoc(char* title, const char* composer, const char* lyricist, const char* artist, const char* tabber, const char* irights);
	int save(const char* xmlPath);
	void saveMeasure(measure toSave);
};