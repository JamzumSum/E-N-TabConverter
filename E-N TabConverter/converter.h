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
#include <vector>

using std::string;
using std::function;
using std::vector;

void TrainMode();
int go(const vector<string>& f, bool isCut, function<void(string)> notify, function<void(int)> progress, string outputDir);

class Converter {
private:
	vector<string> picPath;
	bool ifCut;
	string outputDir;
public:
	Converter(const vector<string>& pics) {
		picPath = pics;
	}

	void Train() {
		TrainMode();
	}

	int scan(function<void(string)> notify, function<void(int)> progress) {
		return go(picPath, ifCut, notify, progress, outputDir);
	}

	void setCut(bool ifCut) {
		this->ifCut = ifCut;
	}

	bool getCut() { return ifCut; }

	void setOutputDir(string dir) {
		outputDir = dir;
	}

	string getOutputDir() { return outputDir; }


};