#pragma once
#include "stdafx.h"
#include "opencv.hpp"
#include "ml.hpp"

using namespace std;
using namespace cv;
using namespace cv::ml;

class NumReader {
private:
	Ptr<ml::KNearest> knn;
	void train(string save);
	int rec(Mat character, vector<int>& possible, vector<float>& safety, float thresh);
	void load(string csv);
};