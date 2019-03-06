#pragma once
#include "type.h"
#include "swan.h"
#include <map>
#include <algorithm>
#include <fstream>

class GlobalUnit {
private:
	int value = 0;
	bool initialed = false;
	float studyRate = 0.5;
public:
	void operator +=(int newVa) {
		value = initialed ? (int)((1 - studyRate) * value + studyRate * newVa) : (initialed = true, newVa);
	}
	operator int() {
		return value;
	}

	void setStudyRate(float newVa) {
		studyRate = newVa;
	}

};

class GlobalPool {
private:
	std::string path;
	int col;								//乐谱宽度
public:
	GlobalUnit lineThickness;				//线粗细
	GlobalUnit rowLenth;					//六线宽度
	GlobalUnit colLenth;					//小节宽度
	GlobalUnit valueSignalLen;
	GlobalUnit characterWidth;				//字符宽度


	GlobalPool(std::string, int);
	~GlobalPool();

};


extern GlobalPool* global;