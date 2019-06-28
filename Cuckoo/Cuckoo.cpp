#include "pch.h"
#include "Cuckoo.h"
#include "Dodo.h"
#include "../E-N TabConverter/eagle.h"

using namespace std;
using namespace cv;

#define find_in(vector, lambda) find_if(vector##.begin(), vector##.end(), lambda)
#if _DEBUG
#define Showrectangle if(1)
#define Showline if(0)
#define draw(func, img, from, to, color) Show##func func(img, from, to, color)
#else 
#define Showrectangle /##/
#define Showline /##/
#define draw(func, img, from, to, color)
#endif

bool savepic = 0;
static NumReader reader(defaultCSV);

static int count(Mat img, Rect range, int delta) {
	Mat roi = img(range);
	bool lock = false;
	int sum = 0, x = range.x + (delta > 0 ? range.width : 0) + delta;
	//int blocksize = 2 * (int)max(1.0, round(getkey(col / 1000.0)) + 1;
	for (int y = range.height / 2; y < range.height; y++)
	{
		uchar* ptr1 = img.ptr<uchar>(y);
		if (!ptr1[x]) {
			if (!lock) {
				sum++;
				lock = true;
			}
		}
		else {
			lock = false;
		}
	}
	return sum;
}

void measure::recNum(Mat denoised, vector<Vec4i> rows) {
	/*
	* 函数：measure::recNum
	* 功能：从传入图像中提取数字等
	* 参数：
		denoised，Mat，传入图像
		rows，Vec4i，传入的网格信息（谱线）
	*/
	vector<vector<Point>> cont;
	vector<Rect> region, possible;
	Mat inv = 255 - denoised;
	Mat ccolor;
	auto mergeFret = [this](int t) {
		vector<easynote>::iterator m, n;
		while (1) {										//合并相邻的fret version2

			m = find_in(notes[0].chords, ([this, t, &n](const easynote x) ->bool {
				n = find_in(notes[0].chords, ([x, t](const easynote y) ->bool {
					return x.pos != y.pos
						&& y.string == x.string
						&& abs(x.pos - y.pos) <= 2 * t;
				}));
				return n != notes[0].chords.end();
			}));
			if (m == notes[0].chords.end()) break;

			if (m->pos > n->pos) swap(m, n);
			if (m->fret > 1) {
				bool flag = false;
				for (int i : m->possible) {
					if (i < 2) {
						m->pos = (m->pos + n->pos) / 2;
						m->fret = 10 * i + n->fret;
						notes[0].chords.erase(n);
						flag = true;
					}
				}
				if (!flag) raiseErr("fret合并：不可置信条件. 检查模型", 0);
			}
			m->pos = (m->pos + n->pos) / 2;
			m->fret = 10 + n->fret;
			notes[0].chords.erase(n);
		}
	};
	auto fillTimeAndPos = [this](int t) {
		for (easynote& i : notes[0].chords) {
			auto add = find_if(notes.begin() + 1, notes.end(), [i, t](const ChordSet x) -> bool {
				return abs(i.pos - x.avrpos) <= t / 2;
			});
			if (add == notes.end()) {
				ChordSet newc;
				newc.avrpos = i.pos;
				newc.chords.emplace_back(i);
				notes.emplace_back(newc);
			}
			else {
				add->chords.emplace_back(i);
				unsigned sum = 0;
				for (easynote& i : add->chords) sum += i.pos;
				add->avrpos = sum / (int)add->chords.size();
			}
		}
	};
	auto countBlack = [](Mat number) -> int {
		int sum = 0;
		for (int y = 0; y < number.rows; y++) {
			uchar* ptr = number.ptr<uchar>(y);
			for (int w = 0; w < number.cols; w++) if (!ptr[w]) sum++;
		}
		return sum;
	};
	Showrectangle cvtColor(denoised, ccolor, CV_GRAY2BGR);

	notes.resize(1);

	findContours(inv, cont, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_NONE);
	for (vector<Point>& i : cont) {								//convert contour to rect
		Rect tmp = boundingRect(i);
		if (tmp.area() > 9
			&& tmp.height > 3
			&& tmp.width > 3
			&& tmp.height < 0.8 * denoised.rows
			&& tmp.width < 0.8 * denoised.cols
			) {
			region.emplace_back(tmp);
		}
	}
	cont.clear();

	for (Rect& i : region) {
		//限定筛选

		if (i.height < 5 * i.width
			&& (getkey(characterWidth) ? (i.width > getkey(characterWidth) / 2) : 1)
			) {
			if (i.height > rows[1][1] - rows[0][1]
				|| i.height < i.width
				|| i.height > 2.5 * i.width
				) {
				//TODO: 形状异常
				if (i.height < maxCharacterHeight / 2) continue;
				if (!whichLine(i, rows)) continue;
				possible.emplace_back(i); continue;
			}
			easynote newNote;
			Mat number = org(i).clone();
			if (countBlack(number) > 0.8 * i.area()) continue;

			cvtColor(number, number, CV_GRAY2BGR);

			number = perspect(number, 8, 14);
			if (savepic) {
				savePic(picFolder, number);													//保存数字样本
				continue;
			}

			newNote.string = whichLine(i, rows);											//几何关系判断string
			if (!newNote.string) continue;
			try {
				newNote.fret = reader.rec(number, newNote.possible, newNote.safety);				//识别数字fret
				if (newNote.fret < 0) continue;												//较大的几率不是数字
			}
			catch (runtime_error ex) {
				throw ex;
			}

			draw(rectangle, ccolor, i.tl(), i.br(), Scalar(0, 0, 255));

			newNote.pos = i.width / 2 + i.x;
			getkey(characterWidth) += i.width;

			maxCharacterWidth = max(maxCharacterWidth, i.width);
			maxCharacterHeight = max(maxCharacterHeight, i.height);
			notes[0].chords.emplace_back(newNote);
		}
	}
	for (Rect& i : possible) {
		//TODO: blocked, like 3--3
		if (i.area() < 0.8 * maxCharacterHeight * maxCharacterWidth) continue;
		easynote newNote = dealWithIt(org(i));
		if (newNote.fret > 0) notes[0].chords.emplace_back(newNote);
	}
	possible.clear();
	Showrectangle imdebug("Showrectangle", ccolor);


	mergeFret(maxCharacterWidth);
	std::sort(notes[0].chords.begin(), notes[0].chords.end(), [](const easynote x, const easynote y) -> bool {
		return x.pos < y.pos || (x.pos == y.pos && x.string < y.string);
	});

	fillTimeAndPos(maxCharacterWidth);
	//assert(notes.size() > 1);
	notes.erase(notes.begin());
}

void measure::recTime(vector<Vec4i> rows) {
	map<int, Value> TimeValue;
	int t = maxCharacterWidth;
	Mat picValue = org(Range(max(rows[5][1], rows[5][3]) + maxCharacterHeight / 2, org.rows), Range::all()).clone();
	//TODO: bug if no value area in org. 
	//		present as max(rows[5][1], rows[5][3]) + maxCharacterHeight / 2 > org.rows, and cvException here. 

	Mat inv;
	vector<vector<Point>> cont;
	auto predictLenth = [&inv, &picValue, &cont, this]() -> int {
		int predLen = 0;
		if (org.rows < getkey(rowLenth) * 2 && org.rows > getkey(rowLenth) / 2) {
			inv = 255 - Morphology(picValue, picValue.rows / 3, false, true);
			findContours(inv, cont, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_NONE);

			int tr = picValue.rows / 4;
			while (!cont.size()) {
				inv = 255 - Morphology(picValue, tr--, false, true);
				findContours(inv, cont, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_NONE);
				if (tr < 2) {
					for (int i = 0; i < picValue.rows; i++) {
						if (!isEmptyLine(picValue, i, 0.04)) raiseErr("未扫描到纵向结构", 6);
					}
					//是空白图
					return 0;
				}
			}

			for (vector<Point> &i: cont) {
				Rect temp = boundingRect(i);
				predLen = max((int)predLen, temp.height);
			}
			getkey(valueSignalLen) += predLen;
		}
		else {
			predLen = getkey(valueSignalLen);
		}
		return predLen;
	};
	auto time_denoise = [&cont]() -> vector<Rect>{
		vector<Rect> region(cont.size());
		std::transform(cont.begin(), cont.end(), region.begin(), [](vector<Point> x) {return boundingRect(x); });
		while (1) {
			auto m = find_in(region, ([&region](Rect x) ->bool {
				return any_of(region.begin(), region.end(), [&](Rect y) ->bool {
					return y.br().y < x.y
						&& y.height > x.height;
				});
			}));
			if (m == region.end()) break;
			else region.erase(m);
		};
		std::sort(region.begin(), region.end(), [](Rect x, Rect y) ->bool {
			return x.x < y.x;
		});
		return region;
	};
	//==========================================local and lambda=================================================
	//==========================================local and lambda=================================================
	//==========================================local and lambda=================================================
	int predLen = predictLenth();
	if (!predLen) return;

	inv = 255 - Morphology(picValue, int(round(predLen * 0.3)), false, true);
	findContours(inv, cont, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_NONE);

	vector<Rect> region = time_denoise();						//去掉节拍记号再往下的乱七八糟的东西

	for (Rect &i: region) {
		int sum1 = 0, sum2 = 0, sum3 = 0;
		sum1 = count(picValue, i, -2);
		sum2 = count(picValue, i, 2);
		sum3 = count(picValue, i, 1);
		if (i.height <= predLen / 2 && !sum1 && !sum2) {
			sum1--; sum2--;
		}
		Value newc = time.beat_type / (int)pow(2, max(sum1, !sum3 && sum2 ? sum2 - 1 : sum2));;
		newc.dot = !sum3 && sum2;
		TimeValue[i.x + i.width / 2] = newc;
	}
	assert(!TimeValue.empty());
	Value kk = TimeValue.begin()->second;

	if (all_of(TimeValue.begin(), TimeValue.end(), [kk](const pair<int, Value>& x) -> bool {return x.second == kk; })) {
		kk = time.beat_type * (time.beats / (int)TimeValue.size());		//防止4个全音符或者2个四分音符之类的情况
		for (auto& i : TimeValue) i.second = kk;
	}
	
	if (TimeValue.empty() && notes.size() == 1) {
		//全音符
		kk = time.beat_type * time.beats;
		for (easynote& i : notes[0].chords) i.time = kk;
		return;
	}
	//不应该含全音符


	/*vector<ChordSet*> noValue;
	for (ChordSet& i : notes) noValue.emplace_back(&i);*/
	
	for (auto& i : notes) {
		auto range = TimeValue.equal_range(i.avrpos);

		if (range.second != TimeValue.end() && abs(range.second->first - i.avrpos) < t) {
			for (auto& j : i.chords) j.time = range.second->second;
		}
		else if(auto prevRange = --(range.second); abs(prevRange->first - i.avrpos) < t){
			for (auto& j : i.chords) j.time = prevRange->second;
		}
		else 
			for (auto& j : i.chords) j.time = Value::whole;
	}

	/*for (auto &i: TimeValue) {
		int pos = i.first;
		for (;;) {
			auto s = find_in(noValue, ([pos, t](const ChordSet * x)->bool {
				return abs(x->avrpos - pos) < t;
			}));
			if (s == noValue.end()) break;
			for (easynote& j : (**s).chords) {
				j.time = i.second;
			}
			noValue.erase(s);
		}
	}
	if (!noValue.empty()) 
		raiseErr("有未分配时值的乐符", 1);*/
}

MusicMeasure measure::getNotes() {
	MusicMeasure r;
	r.time = this->time;
	r.key = this->key;
	r.id = this->id;
	for (ChordSet& i : notes) {
		if (i.chords.empty()) continue;
		note newn;
		newn.chord = false;  newn.timeValue = i.chords[0].time;
		newn.notation.technical.string = i.chords[0].string;
		newn.notation.technical.fret = i.chords[0].fret;
		r.notes.emplace_back(newn);
		newn.chord = true;
		for (unsigned j = 1; j < i.chords.size(); j++) {
			newn.timeValue = i.chords[j].time;
			newn.notation.technical.string = i.chords[j].string;
			newn.notation.technical.fret = i.chords[j].fret;
			r.notes.emplace_back(newn);
		}
	}
	return r;
}

easynote measure::dealWithIt(Mat img) {
	auto tooWide_MORPH = [this, &img]() -> easynote {
		int more = img.cols - maxCharacterWidth;
		Mat mask = Morphology(255 - img, more - 1, true, false);
		img |= mask;
		vector<vector<Point>> cont;
		vector<Rect> region;
		findContours(img, cont, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_NONE);
		for (vector<Point>& i : cont) {								//convert contour to rect
			Rect tmp = boundingRect(i);
			if (tmp.area() > 9
				&& tmp.height > 3
				&& tmp.width > 3
				) {
				region.emplace_back(tmp);
			}
		}
		cont.clear();
		for (Rect& i : region) {
			if (i.height < 2.5 * i.width
				&& (getkey(characterWidth) ? (i.width > getkey(characterWidth) / 2) : 1)
				&& i.height > i.width) {
				easynote newNote;
				newNote.fret = reader.rec(img(i), newNote.possible, newNote.safety, 45.0f);
				return newNote;
			}
		}
		return easynote::invalid();
	};
	auto tooHigh_MORPH = [this, &img]() -> easynote {
		int more = img.rows - maxCharacterHeight;
		Mat mask = Morphology(255 - img, more - 1, true, false);
		img |= mask;
		vector<vector<Point>> cont;
		vector<Rect> region;
		findContours(img, cont, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_NONE);
		for (vector<Point>& i : cont) {								//convert contour to rect
			Rect tmp = boundingRect(i);
			if (tmp.area() > 9
				&& tmp.height > 3
				&& tmp.width > 3
				) {
				region.emplace_back(tmp);
			}
		}
		cont.clear();
		for (Rect& i : region) {
			if (i.height < 2.5 * i.width
				&& (getkey(characterWidth) ? (i.width > getkey(characterWidth) / 2) : 1)
				&& i.height > i.width) {
				easynote newNote;
				newNote.fret = reader.rec(img(i), newNote.possible, newNote.safety, 45.0f);
				return newNote;
			}
		}
		return easynote::invalid();
	};
	auto tooWide_Rec = [this, &img]() -> easynote {
		int trueWidth = int(float(img.rows) / maxCharacterHeight * maxCharacterWidth);
		for (int i = 0; i < img.cols - trueWidth; i++) {
			Mat test = img(Range::all(), Range(i, i + trueWidth));
			easynote newNote;
			newNote.fret = reader.rec(test, newNote.possible, newNote.safety, 25.0f);
			if (newNote.fret > 0) return newNote;
		}
		return easynote::invalid();
	};
	auto tooHigh_Rec = [this, &img]() -> easynote {
		int trueHeight = int(float(img.rows) / maxCharacterHeight * maxCharacterWidth);
		for (int i = 0; i < img.rows - trueHeight; i++) {
			Mat test = img(Range(i, i + trueHeight), Range::all());
			easynote newNote;
			newNote.fret = reader.rec(test, newNote.possible, newNote.safety, 25.0f);
			if (newNote.fret > 0) return newNote;
		}
		return easynote::invalid();
	};
	if (img.cols > maxCharacterWidth && img.rows < maxCharacterHeight) {
		if (img.cols > 2 * maxCharacterWidth) return tooWide_Rec();
		else return tooWide_MORPH();
	}
	else if (img.cols < maxCharacterWidth && img.rows > maxCharacterHeight) {
		if (img.rows > 2 * maxCharacterWidth) return tooHigh_Rec();
		else return tooHigh_MORPH();
	}
	else return easynote::invalid();
}

measure::measure(Mat origin, size_t id)
	: id(id), ImageProcess(origin)
{
	maxCharacterWidth = getkey(characterWidth) / 2;
	maxCharacterHeight = maxCharacterWidth + 1;
	
	if (org.cols < getkey(colLenth) / 5) {
		this->id = 0;
		return;
	}
}

void measure::start(vector<Vec4i> rows) {
	Denoiser den(org);
	Mat img = den.denoise_morphology();
	try {
		recNum(img, rows);
		if (!savepic) {
			recTime(rows);
		}
	}
	catch (Err ex) {
		switch (ex.id)
		{
		case 0: break;									//包含不合理数据
		case 1:											//timeValue越界
			imdebug(ex.what(), org);
			break;
		case 6:											//没有竖直结构用以判断时值
			imdebug(ex.what(), org);
			break;
		default: throw ex; break;
		}
	}
}