#pragma once
#include "stdafx.h"
#include "opencv.hpp"
#include "ml.hpp"

using namespace std;
using namespace cv;
using namespace cv::ml;

class NumReader {
private:
	int preferWidth = 8;
	int preferHeight = 14;
	Ptr<ml::KNearest> knn = KNearest::create();;
public:
	static void train(string save);
	map<char, float> rec(Mat character, float threshold = 60.0f);
	void load(string csv);
	bool isTrained() { return knn->isTrained(); }

	NumReader() {}
	NumReader(string csv) { load(csv); }

	int getPreferWidth() { return preferWidth; }
	int getPreferHeight() { return preferHeight; }
};