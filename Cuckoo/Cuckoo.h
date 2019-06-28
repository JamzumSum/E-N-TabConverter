#pragma once
#pragma comment(lib, "Cuckoo.lib")
#include "cv.h" 
#include "../E-N TabConverter/music.h"
#include "../E-N TabConverter/global.h"
#include <atomic>

#if _DEBUG
#define imdebug(title, img) imshow((title), img); cv::waitKey()
#else
#define imdebug(title, img)
#endif

extern GlobalPool global;

class easynote {
public:
	int pos = 0, fret = 0;
	unsigned string = 0;
	std::vector<int> possible;
	std::vector<float> safety;
	Value time;

	static easynote invalid() { easynote i; i.fret = -1; return i; }
};

using ChordSet = struct {
	int avrpos;
	std::vector<easynote> chords;
};

class ImageProcess {
protected:
	cv::Mat org;
public:
	ImageProcess(cv::Mat origin) : org(origin) {
		assert(org.empty() != true);
	}
};

class measure: ImageProcess {
private:
	int maxCharacterWidth = 0;
	int maxCharacterHeight = 0;
	void recNum(cv::Mat section, std::vector<cv::Vec4i> rows);
	void recTime(std::vector<cv::Vec4i> rows);
	easynote dealWithIt(cv::Mat it);
	std::vector<ChordSet> notes;
public:
	size_t id = 0;							//Ð¡½ÚÊý
	Time time;
	key key;
	measure(cv::Mat img, size_t id);
	MusicMeasure getNotes();
	void start(std::vector<cv::Vec4i> rows);
};

class Splitter: public ImageProcess{
public:
	Splitter(cv::Mat img) : ImageProcess(img) {}
	void start(std::vector<cv::Mat>& piece);
};

class Denoiser : public ImageProcess {
public: 
	Denoiser(cv::Mat img) : ImageProcess(img) {}
	cv::Mat denoise_morphology();
	cv::Mat denoise_inpaint(std::vector<cv::Vec4i> lines, double radius);
};

class LineFinder: public ImageProcess{
private:
	double range;
	std::vector<int> thickness;
	int upper = 0, lower = 0;

	bool isDotLine(cv::Vec4i line);
public:
	LineFinder(cv::Mat img, double rangeRad) :ImageProcess(img), range(rangeRad) {}
	LineFinder(cv::Mat img, int rangeDeg) :ImageProcess(img) {
		range = double(rangeDeg) * CV_PI / 180.0;
	}
	void findRow(std::vector<cv::Vec4i>& lines);
	void findCol(std::vector<cv::Vec4i>& lines);
	std::vector<int> getThickness() { return thickness; }
};

