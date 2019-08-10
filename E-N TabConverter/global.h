#pragma once
#include <map>
#include <mutex>
#include <string>
#include "Accessor.hpp"

using std::string;

namespace easy {
	template<class T, size_t bias = 0>
	size_t hash(T val, T vals...) {
		if constexpr (sizeof(vals) == 0) return bias + std::hash<T>()(val);
		else return bias + 31 * std::hash<T>()(val);
	}
}


class Property {
public:
	string cfgPath;
	string samplePath;
	string defaultCSV;
	string picFolder;

	bool ifCut;
	bool savePic;

};

class GlobalUnit {
private:
	int value;
	bool initialed = false;
	float studyRate = 0.5;
	std::mutex lock;

public:
	GlobalUnit& operator= (const GlobalUnit& that) {
		this->value = that.value;
		this->initialed = that.initialed;
		this->studyRate = that.studyRate;
		return *this;
	}

	void operator +=(const int newVa) {
		lock.lock();
		value = initialed ? static_cast<int>((1 - studyRate) * value + studyRate * newVa) 
			: (initialed = true, newVa);
		lock.unlock();
	}

	operator int() const {
		assert(initialed);
		return value;
	}

	GlobalUnit(int init) : value(init) {
		initialed = true;
	}

	GlobalUnit() : value(0) {}

	void setStudyRate(float newVa) {
		lock.lock();
		studyRate = newVa;
		lock.unlock();
	}

	const bool isInit() const { return initialed; }
};

class GlobalPool {
private:
	mutable std::map<string, const GlobalUnit*> table;
	Property prop;
	int col;										//ÀÖÆ×¿í¶È
	int predictValue(int, std::map<int, int>);
public:
	const GlobalUnit& operator[](const string& key) const;
	~GlobalPool();
	void save();
	void setCol(int);
	GlobalPool();

	Accessor<Property, ReadOnly> Prop = prop;
};