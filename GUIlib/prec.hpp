#pragma once
#pragma warning(disable : 4302)
#pragma warning(disable : 4311)
#pragma warning(disable : 4312)
#include <assert.h>
#include <vector>
#include <windows.h>
class window;
class form;
class control;

#define Dsetter(na,ty) void set##na(ty na){ this->##na = na; }

#define Dgetter(na,ty) ty get##na(){ return this->##na; }

#define Dgesetter(na,ty) ty na = NULL;\
Dgetter(na,ty)\
Dsetter(na,ty)

enum permission {
	readWrite, readOnly, writeOnly
};

template <typename Container, typename Value, enum permission prop>
class Property {
private:
	Container * me = NULL;
	void (Container::*Set)(Value value) = NULL;
	Value(Container::*Get)() = NULL;
	bool init = true;
public:
	Property() noexcept {}
	Property(bool initialed) noexcept {
		init = initialed;
	}
	void setContainer(Container* x) {
		me = x;
	}
	void setter(void (Container::*pSet)(Value value)) {
		Set = pSet;
	}
	void getter(Value(Container::*pGet)()) {
		if ((prop == readWrite) || (prop == readOnly)) Get = pGet;
	}
	Value operator= (const Value& value)
	{
		assert(me);
		assert(Set);
		switch (prop) {
		case writeOnly:
		case readWrite:
			(me->*Set)(value);
			break;
		case readOnly:
			assert(!init);
			(me->*Set)(value);
			break;
		}
		init = true;
		return value;
	}
	operator Value() {
		assert(me);
		assert(Get);
		return (me->*Get)();
	}
};


class windowSet {
private:
	std::vector<window*> pool;
	size_t size = 0;
public:
	window* find(HWND hWnd);
	window* find(LPTSTR clsname = NULL);
	window* find(size_t id) {
		return pool[id - 1];
	}
	void remove(size_t id) {
		assert(id);
		pool[id - 1] = NULL;
		size--;
	}
	void add(window* which);
	operator std::vector<window*>& () {
		return pool;
	}
	window*& operator[] (size_t index) {
		return pool[index];
	}
};