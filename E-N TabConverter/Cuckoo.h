#pragma once
#include <vector>
#include "cv.h" 
#include "music.h"

using namespace std;

#define picFolder "C:\\Users\\Administrator\\Desktop\\oh"

typedef struct {
	int pos;
	unsigned string, fret;
	vector<int> possible;
	vector<float> safety;
	Value time;
}easynote;

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
	cv::Mat org;
	vector<ChordSet> notes;
public:
	size_t id = 0;							//Ð¡½ÚÊý
	Time time;
	key key;
	measure(cv::Mat origin, vector<cv::Vec4i> rows, size_t id);
	vector<note> getNotes();
};

class splitter{
private:
	cv::Mat org;
public:
	splitter(cv::Mat img) : org(img) {}
	void start(vector<cv::Mat>& piece);
};

class denoiser {
private:
	cv::Mat org;
public: 
	denoiser(cv::Mat img) : org(img) {}
	cv::Mat denoise_morphology();
	cv::Mat denoise_inpaint(vector<cv::Vec4i> lines, double radius);
};