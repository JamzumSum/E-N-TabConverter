#pragma once
#include "cv.h" 
#include "music.h"
#include "global.h"
#include <atomic>

#if _DEBUG
#define imdebug(title, img) imshow((title), img); cv::waitKey()
#else
#define imdebug(title, img)
#endif

extern GlobalPool global;

class EasyNote {
public:
	unsigned pos = 0;	//horizontal postion of the note on image. 
	int fret = 0;		//fret -1 as invalid. 
	int string = 0;		//string -1 as invalid, string 0 as a rest. string 1-6 as normal string. 
	std::map<float, char> possible;
	Value time;

	static EasyNote rest(int pos, Value value) { EasyNote i; i.time = value; i.pos = pos; return i; };
	static EasyNote invalid() { EasyNote i; i.string = -1; return i; }

	void acceptRecData(std::map<char, float> data) {
		assert(!data.empty());
		for (auto i : data) possible.insert(std::make_pair(i.second, i.first));
		fret = possible.begin()->second - '0';
	}
};

class ChordSet: public std::vector<EasyNote>
{
public:
	ChordSet() {}
	ChordSet(std::initializer_list<EasyNote> initializer) 
		: std::vector<EasyNote>(initializer){}

	static ChordSet rest(int pos, Value value) {
		return ChordSet({ EasyNote::rest(pos, value) });
	}
};

class ImageProcess {
protected:
	cv::Mat org;
public:
	ImageProcess(cv::Mat origin) : org(origin) {
		assert(org.empty() != true);
	}
};

class Measure: ImageProcess {
private:
	size_t id = 0;							//Ð¡½ÚÊý
	int maxCharacterWidth = 0;
	int maxCharacterHeight = 0;
	void recNum(cv::Mat section, const std::vector<cv::Vec4i>& rows);
	void recTime(const std::vector<cv::Vec4i>& rows);
	EasyNote dealWithIt(const cv::Mat& org, const cv::Rect& region, const std::vector<cv::Vec4i>& rows);
	std::map<unsigned, ChordSet> notes;
public:
	Time time;
	key key;
	Measure(cv::Mat img, size_t id);
	MusicMeasure getNotes() const;
	void start(const std::vector<cv::Vec4i>& rows);

	size_t ID() const { return id; }
	void setID(size_t newID) { id = newID; }
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

