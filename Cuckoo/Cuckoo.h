#pragma once
#include "cv.h" 
#include "../E-N TabConverter/music.h"
#include "../E-N TabConverter/global.h"

#if _DEBUG
#define imdebug(title, img) imshow((title), img); cv::waitKey()
#else
#define imdebug(title, img)
#endif

using namespace std;

extern GlobalPool global;

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

class ImageProcess {
protected:
	cv::Mat org;
public:
	ImageProcess(cv::Mat origin) : org(origin) {

	}
};

class measure: ImageProcess {
private:
	int maxCharacterWidth = 0;
	int maxCharacterHeight = 0;
	void recNum(cv::Mat section, vector<cv::Vec4i> rows);
	void recTime(vector<cv::Vec4i> rows);
	easynote dealWithIt(cv::Mat it);
	vector<ChordSet> notes;
public:
	size_t id = 0;							//Ð¡½ÚÊý
	Time time;
	key key;
	measure(cv::Mat img, vector<cv::Vec4i> rows, size_t id);
	vector<note> getNotes();
};

class Splitter: public ImageProcess{
public:
	Splitter(cv::Mat img) : ImageProcess(img) {}
	void start(vector<cv::Mat>& piece);
};

class Denoiser : public ImageProcess {
public: 
	Denoiser(cv::Mat img) : ImageProcess(img) {}
	cv::Mat denoise_morphology();
	cv::Mat denoise_inpaint(vector<cv::Vec4i> lines, double radius);
};

class LineFinder: public ImageProcess{
private:
	double range;
	vector<int> thickness;
	int upper, lower;

	bool isDotLine(cv::Vec4i line);
public:
	LineFinder(cv::Mat img, double rangeRad) :ImageProcess(img), range(rangeRad) {}
	LineFinder(cv::Mat img, int rangeDeg) :ImageProcess(img) {
		range = double(rangeDeg) * CV_PI / 180.0;
	}
	void findRow(vector<cv::Vec4i>& lines);
	void findCol(vector<cv::Vec4i>& lines);
	vector<int> getThickness() { return thickness; }
};