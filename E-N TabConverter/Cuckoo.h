#pragma once
#include "cv.h" 
#include "music.h"

using namespace std;

class easynote {
public:
	int pos = 0, fret = 0;
	unsigned string;
	vector<int> possible;
	vector<float> safety;
	Value time;

	static easynote invalid() { easynote i; i.fret = -1; return i; }
};

typedef struct {
	vector<easynote> chords;
	int avrpos;
}ChordSet;

class measure {
private:
	int maxCharacterWidth = 0;
	int maxCharacterHeight = 0;
	void recNum(cv::Mat section, vector<cv::Vec4i> rows);
	void recTime(vector<cv::Vec4i> rows);
	easynote dealWithIt(cv::Mat it);
	cv::Mat org;
	vector<ChordSet> notes;
public:
	size_t id = 0;							//Ð¡½ÚÊý
	Time time;
	key key;
	measure(cv::Mat origin, vector<cv::Vec4i> rows, size_t id);
	vector<note> getNotes();
};

class Splitter{
private:
	cv::Mat org;
public:
	Splitter(cv::Mat img) : org(img) {}
	void start(vector<cv::Mat>& piece);
};

class Denoiser {
private:
	cv::Mat org;
public: 
	Denoiser(cv::Mat img) : org(img) {}
	cv::Mat denoise_morphology();
	cv::Mat denoise_inpaint(vector<cv::Vec4i> lines, double radius);
};

class LineFinder {
private:
	cv::Mat img;
	double range;
	vector<int> thickness;
	int upper, lower;

	bool isDotLine(cv::Vec4i line);
public:
	LineFinder(cv::Mat img, double rangeRad) :img(img), range(rangeRad) {}
	LineFinder(cv::Mat img, int rangeDeg) :img(img) {
		range = double(rangeDeg) * CV_PI / 180.0;
	}
	void findRow(vector<cv::Vec4i>& lines);
	void findCol(vector<cv::Vec4i>& lines);
	vector<int> getThickness() { return thickness; }
};