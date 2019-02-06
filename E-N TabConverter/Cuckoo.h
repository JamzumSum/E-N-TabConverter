#pragma once
#include <vector>
#include "cv.h" 

using namespace std;

#define picFolder "C:\\Users\\Administrator\\Desktop\\oh"

enum value {
	zero = 0,
	whole = 1,
	half = 2,
	quarter = 4,
	eighth = 8,
	sixteenth = 16,
	thirty_second = 32
};

class Value {
private:
	int v = whole;
public:
	bool dot = false;
	Value(const value init) :v(init) {}
	Value() {}
	Value operator= (const value x) {
		v = x;
		return x;
	}
	Value operator+(const value x) {
		if (!v) v = x;
		else if (!x) return *this;
		else return value(v * x / (v + x));
	}
	Value operator+= (const value x) {
		if (!v) v = x;
		else if (!x) return *this;
		else v = v * x / (v + x);
		return *this;
	}
	Value operator-(const value x) {
		if (x == v) return zero;
		if (!v) return x;
		if (!x) return (value)v;
		else return value(x * v / abs(x - v));
	}
	Value operator*(const float x) {
		assert(x);
		return (value)(int)(v / x);
	}
	Value operator/(const int x) {
		return (value)(v * x);
	}
	bool operator<(const value x) {
		return x < this->v;
	}
	bool operator>(const value x) {
		return x > this->v;
	}
	bool operator==(const value x) {
		return x == this->v;
	}
	operator value() {
		return (value)v;
	}
};

typedef struct {
	int pos;
	unsigned string, fret;
	vector<int> possible;
	Value time;
}easynote;

typedef struct {
	vector<easynote> chords;
	int avrpos;
}ChordSet;

typedef struct{
	int string = 1;									//弦
	int fret = 0;									//品
}technical;

typedef struct{
	char* dynamics = (char*)"mf";					//力度
	technical technical;
}notations;

class note{
public:
	bool chord = false;							//和声标记，为true与上一个音符同时发声
	std::vector<int> possible;					//其他可能的品数
	Value timeValue;							//时值
	bool dot = false;							//附点
	int voice = 1;								//发声类型
	int duration = 1;							//延音值
	notations notation = { (char*)"mf",{1,0} };

	int pos;									//x坐标
	bool operator==(const note x) {
		return pos == x.pos &&
			notation.technical.string == x.notation.technical.string &&
			notation.technical.fret == x.notation.technical.fret;
	}
};

typedef struct Time {
	int beats = 4;								//每小节拍数
	Value beat_type = quarter;					//几分音符算作一拍
}Time;

typedef struct key {
	int fifth = 0;								//音调升降
	char* mode = (char*)"major";				//1-标准音主序列
}key;

class measure {
private:
	int maxCharacterWidth = 0;
	int maxCharacterHeight = 0;
	void recNum(cv::Mat section, vector<cv::Vec4i> rows);
	void recTime(vector<cv::Vec4i> rows);
	cv::Mat org;
	vector<ChordSet> notes;
public:
	int id;
	unsigned int number = 1;						//小节数
	Time time;
	key key;
	measure(cv::Mat org, cv::Mat img, vector<cv::Vec4i> rows, int id);
	vector<note> getNotes();
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
};