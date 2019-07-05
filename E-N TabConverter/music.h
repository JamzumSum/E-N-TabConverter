#pragma once
#include <vector>

class Value {
private:
	float v = 1;
	Value(const float init) {
		v = init;
	}
public:
	enum predef {
		none = 0,
		whole = 1,
		half = 2,
		quarter = 4
	};

	bool dot = false;
	Value(const int init)  {
		v = 1.0f / init;
	}
	Value() {}
	const Value operator= (const int x) {
		v = 1.0f / x;
		return x;
	}
	const Value operator= (const Value x) {
		v = x.v;
		return x;
	}
	Value operator+(const int x) {
		return Value(v + 1.0f / x);
	}
	Value operator+= (const int x) {
		v += 1.0f / x;
		return *this;
	}
	Value operator-(const int x) {
		return Value(abs(v - 1.0f / x));
	}
	const Value operator*(const int x) {
		return Value(v * x);
	}
	const Value operator/(const int x) {
		return Value(v / x);
	}
	bool operator<(const int x) {
		return v < 1.0f / x;
	}
	bool operator>(const int x) {
		return v > 1.0f / x;
	}
	bool operator==(const int x){
		return x == int(round(1 / v));
	}
	bool operator==(const Value& x) const {
		return x.v == int(round(1 / v)) && x.dot == dot;
	}
	operator int() {
		return int(round(1 / v));
	}
};

typedef struct {
	int string = 1;									//弦
	int fret = 0;									//品
}technical;

typedef struct {
	char* dynamics = (char*)"mf";					//力度
	technical technical;
	std::string notation;
}notations;

class note {
public:
	bool chord = false;							//和声标记，为true与上一个音符同时发声
	std::vector<int> possible;					//其他可能的品数
	Value timeValue;							//时值
	bool dot = false;							//附点
	int voice = 1;								//发声类型
	int duration = 1;							//延音值
	notations notation = { (char*)"mf", {1,0}, ""};

	int pos;									//x坐标
	bool operator==(const note x) {
		return pos == x.pos &&
			notation.technical.string == x.notation.technical.string &&
			notation.technical.fret == x.notation.technical.fret;
	}
};

typedef struct{
	int beats = 4;											//每小节拍数
	Value beat_type = 4;									//几分音符算作一拍
}Time;

typedef struct{
	int fifth = 0;											//音调升降
	std::string mode = "major";									//1-标准音主序列
}key;

typedef struct  {
	size_t id;
	std::vector<note> notes;
	Time time;
	key key;
}MusicMeasure;