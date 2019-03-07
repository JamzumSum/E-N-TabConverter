#pragma once
#include "tinyxml2.h"
#include <map>
#include <string>

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

	constexpr GlobalUnit(int init) : value(init) {
		initialed = true;
	}

	constexpr GlobalUnit() {}
	void setStudyRate(float newVa) {
		studyRate = newVa;
	}

};

class GlobalPool {
#define getkey(key) global.get(#key)

private:
	std::map<std::string, GlobalUnit> table;
	std::string path;
	int col;								//ÀÖÆ×¿í¶È
public:
	GlobalUnit& get(std::string key);
	int predictValue(int, std::map<int, int>);
	void save();
	void setCol(int);
	GlobalPool(std::string = "global.xml");

};