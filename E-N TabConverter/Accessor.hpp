#pragma once
#include <functional>

using std::function;

enum AccessControl
{
	ReadOnly, WriteOnly, ReadWrite
};

template<class T, const AccessControl actrl>
class Accessor {
private:
	T& val;
	mutable function<void(const T&)>bfGet = nullptr;
	mutable function<void(const T&)>afSet = nullptr;

public:
	Accessor(const T& val) :val(const_cast<T&>(val)) {}

	//setter
	void operator=(const T& val) { 
		if constexpr (actrl != ReadOnly) this->val = val;
		if(afSet) afSet(this->val);
	}
	//const getter
	operator const T&() const { 
		if(bfGet) bfGet(this->val);
		if constexpr (actrl != WriteOnly) return val;
	}
	//getter
	operator T() const { 
		if (bfGet) bfGet(this->val);
		if constexpr (actrl != WriteOnly) return val;
	}

	void beforeGet(function<void(const T&)> func) const { bfGet = func; }
	void afterSet(function<void(const T&)> func) const { afSet = func; }
};