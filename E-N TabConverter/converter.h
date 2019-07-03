#pragma once

/*
	converter.h
	function to export. 
*/
#include <functional>
#include <string>
#include <vector>
#include <cv.h>

using std::string;
using std::function;
using std::vector;

class Converter {
private:
	vector<string> picPath;
	string outputDir;
	function<string(void)> selectSaveStrategy;
	void* api;
	bool ifCut;
	void* doc = nullptr;
	bool ocrReady = false;

	auto scan(const int startWith, string path, function<void(string)> notify, function<void(int)> progress);
	void title(const vector<cv::Mat>& info);
public:
	Converter(const vector<string>& pics);

	static void Train();

	void scan(function<void(string)> notify, function<void(int)> progress);

	void setCut(bool ifCut) {
		this->ifCut = ifCut;
	}

	bool getCut() { return ifCut; }

	void setOutputDir(string dir) {
		outputDir = dir;
	}

	string getOutputDir() { return outputDir; }

	void setSelectSaveStrategy(function<string(void)> strategy) { selectSaveStrategy = strategy; }

	void setPicPath(vector<string> pics) {
		assert(!pics.empty());
		picPath = pics;
	}

	~Converter();
};