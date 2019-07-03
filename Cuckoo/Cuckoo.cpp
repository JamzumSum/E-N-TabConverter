#include "pch.h"
#include "Cuckoo.h"
#include "Dodo.h"
#include "eagle.h"

using namespace std;
using namespace cv;

#define find_in(vector, lambda) find_if((vector).begin(), (vector).end(), lambda)
#define Showrectangle if constexpr(0)
#define Showline if constexpr(0)
#define draw(func, img, from, to, color) Show##func func(img, from, to, color)

bool savepic = 0;
static knnReader reader;

static int count(Mat img, Rect range, int delta) {
	assert(img.isContinuous());
	bool lock = false;
	int sum = 0, x = range.x + (delta > 0 ? range.width : 0) + delta;
	//int blocksize = 2 * (int)max(1.0, round(getkey(col / 1000.0)) + 1;
	for (int y = range.height / 2; y < range.height; y++)
	{
		uchar* ptr1 = img.ptr<uchar>(range.y + y);
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

/**
	从传入图像中提取数字等
	@name	Measure::recNum
	@param	denoised，Mat，传入图像
	@param	rows，Vec4i，传入的网格信息（谱线）
*/
void Measure::recNum(Mat denoised, const vector<Vec4i>& rows) {
	
	vector<vector<Point>> cont;
	vector<Rect> region, possible;
	Mat inv = 255 - denoised;
	Mat ccolor;
	auto mergeFret = [this](unsigned t) {				//mergeFret v3
		//note[0] is sorted. 

		for (auto i = notes[0].begin(); i != notes[0].end();) {
			for (auto j = i + 1; j != notes[0].end(); j++) {
				if (bool flag = false; j->pos - i->pos <= ((unsigned)maxCharacterWidth << 1)) {
					if (i->string != j->string) {			//can be aligned.
						i->pos = j->pos = (i->pos + j->pos) >> 1;
						continue;
					}
					for (auto k : i->possible) {				//can be merged. 
						if (k.second > '1') continue;
						i->fret = 10 * (k.second - '0') + j->fret;
						i->pos = (i->pos + j->pos) >> 1;
						j = notes[0].erase(j) - 1;
						flag = true;
						break;
					}
					if(!flag) raiseErr("fret合并：不可置信条件. 检查模型", 0);
				}
				else break;
			}
			i++;
		}
	};
	auto fillTimeAndPos = [this](unsigned t) {
		for (EasyNote& i : notes[0]) {
			auto _1 = notes.begin();
			auto add = find_if(++_1, notes.end(), [i, t](pair<const unsigned, ChordSet> x) -> bool {
				return abs(i.pos - x.first) <= t / 2;
			});
			if (add == notes.end()) {
				ChordSet newc({i});
				notes[i.pos] = newc;
			}
			else {
				add->second.emplace_back(i);
				unsigned sum = 0;
				for (EasyNote& i : add->second) sum += i.pos;
				sum /= static_cast<unsigned>(add->second.size());
				if (add->first != sum) {
					notes.insert(make_pair(sum, add->second));
					notes.erase(add);
				}
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
			EasyNote newNote;
			Mat number = org(i).clone();
			if (countBlack(number) > 0.8 * i.area()) continue;

			newNote.string = whichLine(i, rows);											//几何关系判断string
			if (!newNote.string) continue;
			try {
				newNote.acceptRecData(reader.rec(number));									//识别数字fret
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
			notes[0].emplace_back(newNote);
			if (savepic) {
				savePic(picFolder, number);													//保存数字样本
			}
		}
	}
	for (Rect& i : possible) {
		//TODO: blocked, like 3--3
		if (i.area() < 0.8 * maxCharacterHeight * maxCharacterWidth) continue;
		EasyNote newNote = dealWithIt(org, i, rows);
		if (newNote.fret > 0) notes[0].emplace_back(newNote);
	}
	possible.clear();
	Showrectangle imdebug("Showrectangle", ccolor);

	std::sort(notes[0].begin(), notes[0].end(), [](const EasyNote x, const EasyNote y) -> bool {
		return x.pos < y.pos || (x.pos == y.pos && x.string < y.string);
	});
	mergeFret(maxCharacterWidth);
	

	fillTimeAndPos(maxCharacterWidth);
	//assert(notes.size() > 1);
	notes.erase(notes.begin());
}

void Measure::recTime(const vector<Vec4i>& rows) {
	map<int, Value> timeValue;
	unsigned t = maxCharacterWidth;
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
		Value newc = time.beat_type / (int)pow(2, max(sum1, !sum3 && sum2 ? sum2 - 1 : sum2));
		newc.dot = !sum3 && sum2;
		timeValue[i.x + i.width / 2] = newc;
	}
	assert(!timeValue.empty());
	Value kk = timeValue.begin()->second;

	if (all_of(timeValue.begin(), timeValue.end(), [kk](const pair<int, Value>& x) -> bool {return x.second == kk; })) {
		kk = time.beat_type * (time.beats / (int)timeValue.size());		//防止4个全音符或者2个四分音符之类的情况
		for (auto& i : timeValue) i.second = kk;
	}
	
	if (timeValue.empty() && notes.size() == 1) {
		//全音符
		kk = time.beat_type * time.beats;
		for (EasyNote& i : notes[0]) i.time = kk;
		return;
	}
	//不应该含全音符
	
	for (auto& i : notes) {
		auto range = timeValue.equal_range(i.first);

		if (range.second != timeValue.end() && abs(range.second->first - i.first) < t) {
			for (auto& j : i.second) j.time = range.second->second;
			timeValue.erase(range.second);
		}
		else if(range.second != timeValue.begin()
			&& abs((--range.second)->first - i.first) < t)
		{
			for (auto& j : i.second) j.time = range.second->second;
			timeValue.erase(range.second);
		}
		else 
			for (auto& j : i.second) j.time = Value::whole;				//错误: 找不到对应的时值. 处理方案: 时值置whole. 
	}

	for (auto i : timeValue) {
		notes.insert(make_pair(i.first, ChordSet::rest(i.first, i.second)));
	}
}

MusicMeasure Measure::getNotes() const {
	MusicMeasure r;
	r.time = this->time;
	r.key = this->key;
	r.id = this->id;
	for (auto& i : notes) {
		if (i.second.empty()) continue;
		note newn;
		newn.chord = false;  newn.timeValue = i.second[0].time;
		newn.notation.technical.string = i.second[0].string;
		newn.notation.technical.fret = i.second[0].fret;
		r.notes.emplace_back(newn);
		newn.chord = true;
		for (unsigned j = 1; j < i.second.size(); j++) {
			newn.timeValue = i.second[j].time;
			newn.notation.technical.string = i.second[j].string;
			newn.notation.technical.fret = i.second[j].fret;
			r.notes.emplace_back(newn);
		}
	}
	return r;
}

EasyNote Measure::dealWithIt(const Mat& org, const Rect& region, const vector<Vec4i>& rows) {
	Mat img = org(region);
	auto tooWide_MORPH = [this, &img, &region, &rows]() -> EasyNote {
		int more = img.cols - maxCharacterWidth;
		Mat mask = Morphology(255 - img, more - 1, true, false);
		img |= mask;
		vector<vector<Point>> cont;
		vector<Rect> re;
		findContours(img, cont, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_NONE);
		for (vector<Point>& i : cont) {								//convert contour to rect
			Rect tmp = boundingRect(i);
			if (tmp.area() > 9
				&& tmp.height > 3
				&& tmp.width > 3
				) {
				re.emplace_back(tmp);
			}
		}
		cont.clear(); cont.shrink_to_fit();
		for (Rect& i : re) {
			if (i.height < 2.5 * i.width
				&& (getkey(characterWidth) ? (i.width > getkey(characterWidth) / 2) : 1)
				&& i.height > i.width) {
				EasyNote newNote;
				newNote.acceptRecData(reader.rec(img(i), 45.0f));
				newNote.string = whichLine(Rect(region.tl() + i.tl(), i.size()), rows);
				if (newNote.fret > 0 && newNote.string) {
					newNote.pos = region.x + i.x + i.width / 2;
					return newNote;
				}
				else continue;
			}
		}
		return EasyNote::invalid();
	};
	auto tooHigh_MORPH = [this, &img, &region, &rows]() -> EasyNote {
		int more = img.rows - maxCharacterHeight;
		Mat mask = Morphology(255 - img, more - 1, true, false);
		img |= mask;
		vector<vector<Point>> cont;
		vector<Rect> re;
		findContours(img, cont, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_NONE);
		for (vector<Point>& i : cont) {								//convert contour to rect
			Rect tmp = boundingRect(i);
			if (tmp.area() > 9
				&& tmp.height > 3
				&& tmp.width > 3
				) {
				re.emplace_back(tmp);
			}
		}
		cont.clear();
		for (Rect& i : re) {
			if (i.height < 2.5 * i.width
				&& (getkey(characterWidth) ? (i.width > getkey(characterWidth) / 2) : 1)
				&& i.height > i.width) {
				EasyNote newNote;
				newNote.acceptRecData(reader.rec(img(i), 45.0f));
				newNote.string = whichLine(Rect(region.tl() + i.tl(), i.size()), rows);
				if (newNote.fret > 0 && newNote.string) {
					newNote.pos = region.x + i.x + i.width / 2;
					return newNote;
				}
				else continue;
			}
		}
		return EasyNote::invalid();
	};
	auto tooWide_Rec = [this, &img, &region, &rows]() -> EasyNote {
		int trueWidth = static_cast<int>(float(img.rows) / maxCharacterHeight * maxCharacterWidth);
		for (int i = 0; i < img.cols - trueWidth; i++) {
			Mat test = img(Range::all(), Range(i, i + trueWidth));
			EasyNote newNote;
			newNote.string = whichLine(region, rows);
			newNote.pos = i + trueWidth / 2;
			newNote.acceptRecData(reader.rec(test, 25.0f));
			if (newNote.fret > 0 && newNote.string) return newNote;
		}
		return EasyNote::invalid();
	};
	auto tooHigh_Rec = [this, &img, &region, &rows]() -> EasyNote {
		int trueHeight = int(float(img.rows) / maxCharacterHeight * maxCharacterWidth);
		for (int i = 0; i < img.rows - trueHeight; i++) {
			Mat test = img(Range(i, i + trueHeight), Range::all());
			EasyNote newNote;
			newNote.acceptRecData(reader.rec(test, 25.0f));
			if (newNote.fret > 0) {
				newNote.string = whichLine(Range(i, i + trueHeight), rows);
				if (newNote.string) {
					newNote.pos = region.x + region.width / 2;
					return newNote;
				}
			}
		}
		return EasyNote::invalid();
	};
	if (img.cols > maxCharacterWidth && img.rows < maxCharacterHeight) {
		if (img.cols > 2 * maxCharacterWidth) return tooWide_Rec();
		else return tooWide_MORPH();
	}
	else if (img.cols < maxCharacterWidth && img.rows > maxCharacterHeight) {
		if (img.rows > 2 * maxCharacterWidth) return tooHigh_Rec();
		else return tooHigh_MORPH();
	}
	else return EasyNote::invalid();
}

Measure::Measure(Mat origin, size_t id)
	: id(id), ImageProcess(origin)
{
	maxCharacterWidth = getkey(characterWidth) / 2;
	maxCharacterHeight = maxCharacterWidth + 1;
	
	//TODO: id = 0 is deprecated
	if (org.cols < getkey(colLenth) / 5) {
		this->id = 0;
		return;
	}
}

void Measure::start(const vector<Vec4i>& rows) {
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