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
private:
	std::string notation;

public:
	unsigned pos = 0;	//horizontal postion of the note on image. 
	int fret; 
	int string;		//string -1 as invalid, string 0 as a rest. string 1-6 as normal string. 
	std::map<float, char> possible;
	Value time;

	EasyNote(int string, int fret) : string(string), fret(fret) {}
	EasyNote(): EasyNote(0, 0) {}

	static EasyNote rest(int pos, Value value) { EasyNote i(0, 0); i.time = value; i.pos = pos; return i; };
	static EasyNote invalid() { return EasyNote(-1, 0); }

	void acceptRecData(std::map<char, float> data) {
		assert(!data.empty());
		for (auto i : data) possible.insert(std::make_pair(i.second, i.first));
		char c = possible.begin()->second;
		if (c < 0) fret = c;
		else if(isdigit(c)) fret = c - '0';
		else {
			fret = 0;
			switch (c) {
			case 'x': addNotation('x'); break;
			default: _wassert(L"invalid rec val: " + c, __FILEW__, __LINE__);
			}
		}
	}

	void addNotation(char c) {
		auto it = lower_bound(notation.begin(), notation.end(), c);
		notation.insert(it, c);
	}

	void removeNotation(char c) {
		auto it = lower_bound(notation.begin(), notation.end(), c);
		assert(it != notation.end());
		notation.erase(it);
	}

	const std::string getNotation() const {
		return notation;
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

	void sort() { std::sort(this->begin(), end(), 
		[](const EasyNote& x, const EasyNote& y) {return x.string < y.string; });
	}

	bool is_sorted() const { return std::is_sorted(this->begin(), end(),
		[](const EasyNote& x, const EasyNote& y) {return x.string < y.string; });
	}

	const EasyNote* at(unsigned string) const {
		for (const auto& i : *this) if (i.string == string) return &i;
		return nullptr;
	}

	void rest() {
		clear(); emplace_back(0, 0);
	}

	void removeRest() { 
		erase(std::find_if(begin(), end(), [](const EasyNote& x) {return x.string == 0; }));
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
	std::map<unsigned, ChordSet> notes;
	cv::Mat denoised;
	std::vector<cv::Vec4i> rows;

	void recStaffLines();
	void recTime();
	EasyNote dealWithIt(const cv::Rect& region);
	void mergeFret(std::vector<EasyNote>& noteSet);
	void fillTimeAndPos(std::vector<EasyNote>& noteSet);
	EasyNote recNum(const cv::Rect& rect);
	void dealWithLink(const std::pair<cv::Vec4i, double>& arcVec);
	bool rescan(unsigned pos, Value v = Value::whole);

public:
	Time time;
	key key;
	Measure(cv::Mat img, size_t id);
	MusicMeasure getNotes() const;
	void start(const std::vector<cv::Vec4i>& rows, const std::vector<int>& width);

	size_t ID() const { return id; }
	void setID(size_t newID) { id = newID; }
	bool empty() const { return notes.empty(); }
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
	static void inpaint(cv::Mat& img, std::vector<cv::Vec4i> lines, std::vector<int> width);
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
	std::vector<int> getLineThickness() { return thickness; }
};

