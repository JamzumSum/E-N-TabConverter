#pragma once
#include "stdafx.h"
#include "opencv.hpp"
#include "ml.hpp"

using namespace std;
using namespace cv;
using namespace cv::ml;

class NumReader {
private:
	Ptr<ml::KNearest> knn = KNearest::create();;
public:
	static void train(string save);
	int rec(Mat character, vector<int>& possible, vector<float>& safety, float thresh = 60.0f);
	void load(string csv);
	bool isTrained() { return knn->isTrained(); }

	NumReader() {}
	NumReader(string csv) { load(csv); }
};