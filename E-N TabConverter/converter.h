#pragma once
#pragma comment(lib, "Cuckoo.lib")
#if _DEBUG
#pragma comment(lib, "opencv_world401d.lib")
#else
#pragma comment(lib, "opencv_world401.lib")
#endif

/*
	converter.h
	function to export. 
*/
#include <functional>
#include <string>
using std::string;
using std::function;

void TrainMode();
int go(string f, bool isCut, function<void(string)> notify, function<void(int)> progress);