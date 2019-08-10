#pragma once
#include "global.h"
#include "opencv.hpp"
#include "ml.hpp"
#include "converter.h"

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
	const Property& prop;
	virtual bool onDestroyed(const string& csv) const noexcept;

public:

	static void train(Property save);

	virtual map<char, float> rec(Mat character, float threshold = 60.0f) const = 0;
	virtual bool isTrained() = 0;
	virtual void load(string csv) const = 0;

	CharReader();
	CharReader(string csv);

	int getPreferWidth() const { return preferWidth; }
	int getPreferHeight() const { return preferHeight; }
	void setPreferSize(int width, int height) {
		preferWidth = width;
		preferHeight = height;
	}

	virtual ~CharReader() = 0;
};

class knnReader: public CharReader {
private:
	cv::Ptr<KNearest> knn;
	using CharReader::onDestroyed;
public:
	bool isTrained() { return knn->isTrained(); }

	map<char, float> rec(Mat character, float threshold = 60.0f) const;
	void load(string csv) const;
	knnReader();
};