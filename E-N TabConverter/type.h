#pragma once                          
#include "stdafx.h"
#include <functional>

class Err: public std::runtime_error {
#define raiseErr(description, id) throw Err(description, id, __LINE__)
public:
	int id;
	int line;

	Err(std::string desc, int id, int line) : runtime_error(desc), line(line) {
		this->id = id;
	}
};

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
	notify(void (*pSet)(va newV)){
		Set = pSet;
	}
};