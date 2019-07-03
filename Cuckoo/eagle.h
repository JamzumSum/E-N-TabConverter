#pragma once
#include "stdafx.h"
#include "opencv.hpp"
#include "ml.hpp"

using cv::Mat;
using std::string;
using std::map;
using namespace cv::ml;

constexpr char readable[12] = "0123456789x";

class CharReader {
private:
	int preferWidth = 8;
	int preferHeight = 14;

protected:
	virtual bool onDestroyed(const string& csv) noexcept;

public:

	static void train(string save);

	virtual map<char, float> rec(Mat character, float threshold = 60.0f) = 0;
	virtual bool isTrained() = 0;
	virtual void load(string csv) = 0;

	CharReader() {}
	CharReader(string csv) { load(csv); }

	int getPreferWidth() { return preferWidth; }
	int getPreferHeight() { return preferHeight; }
	void setPreferSize(int width, int height) {
		preferWidth = width;
		preferHeight = height;
	}

	virtual ~CharReader() = 0;
};

class knnReader: public CharReader {
private:
	cv::Ptr<KNearest> knn = KNearest::create();
	using CharReader::onDestroyed;
public:
	bool isTrained() { return knn->isTrained(); }

	map<char, float> rec(Mat character, float threshold = 60.0f);
	void load(string csv);
};