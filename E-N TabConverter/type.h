#pragma once                          
#include <string>
#include <functional>
#include <assert.h>
#include <vector>

typedef struct err {
	int id;
	int line;
	std::string description;
}err;

template<typename va>
class notify {
private:
	va v;
	std::function<void(va)> Set = NULL;
public:
	va operator = (const va newV) {
		assert(Set);
		Set(newV);
		v = newV;
		return newV;
	}
	operator va() {
		return v;
	}
	notify(void (*pSet)(va newV))
	{
		Set = pSet;
	}
};

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

	//GlobalUnit(int init) {
	//	value = init;
	//	initialed = true;
	//}
	//GlobalUnit() {}
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


	GlobalPool(std::string,int);
	~GlobalPool();

};