#include "pch.h"
#include "Cuckoo.h"
#include "Dodo.h"
#include "eagle.h"
#include "converter.h"

using namespace std;
using namespace cv;

#define Showrectangle if constexpr(0)
#define Showline if constexpr(0)
#define draw(func, img, from, to, color) Show##func func(img, from, to, color)

extern thread_local const Converter* root;

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

template<class T>
using tIterator = typename map<unsigned, T>::const_iterator;
/*
	access an interval determined by key and interval. 
	@param uMap		Map has unsigned key. no repeative keys.  
	@param key
	@param interval	the interval is (key - interval, key + interval)
	@return the key-value pair that key is in the interval. 
*/
template<class T>
tIterator<T> intervalAccess(const map<unsigned, T>& uMap, unsigned key, unsigned interval) {
	auto ub = uMap.lower_bound(key);

	if (ub != uMap.end() && ub->first - key < interval) {
		return ub;
	}
	else if (ub != uMap.begin()
		&& key - (--ub)->first < interval)
	{
		return ub;
	}
	return uMap.end();
}

/**
	从谱线区域提取文字/符号等
	@name	Measure::recStaffLines
*/
void Measure::recStaffLines() {
	
	vector<vector<Point>> cont;
	vector<Rect> region, possible, arc;
	vector<EasyNote> tmp;
	Mat ccolor;
	
	Showrectangle cvtColor(denoised, ccolor, CV_GRAY2BGR);

	findContours(denoised, cont, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_NONE);
	for (auto i = cont.begin(); i < cont.end(); ) {								//convert contour to rect
		Rect tmp = boundingRect(*i);
		if (tmp.area() > 9
			&& tmp.height > 3
			&& tmp.width > 2
			&& tmp.height < 0.8 * denoised.rows
			&& tmp.width < 0.8 * denoised.cols
			) {
			i = cont.erase(i);
			region.emplace_back(tmp);
		}
		else i++;
	}
	vector<vector<Point>>().swap(cont);

	for (Rect& i : region) {
		if (i.height < 5 * i.width
			&& (global["characterWidth"].isInit() ? (i.width > global["characterWidth"] / 2) : 1)
			) {
			if (i.height > rows[1][1] - rows[0][1]
				|| i.height < i.width
				|| i.height > 2.7 * i.width
				) {
				//TODO: 形状异常
				if (i.height * 2.4 < i.width) {
					arc.push_back(i);
					continue;
				}
				if (!whichLine(i, rows)) continue;
				if (i.height > maxCharacterHeight / 2) {
					possible.emplace_back(i); 
					continue;
				}
			}
			auto newNote = recNum(i);
			if (newNote.string < 0) continue;
			tmp.emplace_back(newNote);
		}
	}
	for (Rect& i : possible) {
		//TODO: blocked, like 3--3
		if (i.area() < 0.8 * maxCharacterHeight * maxCharacterWidth) continue;
		EasyNote newNote = dealWithIt(i);
		if (newNote.fret > 0) tmp.emplace_back(newNote);
	}
	possible.clear();
	Showrectangle imdebug("Showrectangle", ccolor);

	sort(tmp.begin(), tmp.end(), [](const EasyNote x, const EasyNote y) -> bool {
		return x.pos < y.pos || (x.pos == y.pos && x.string < y.string);
	});

	mergeFret(tmp);
	fillTimeAndPos(tmp);

	if (notes.empty()) {
		return;
	}

	//rec notations. 
	for (const auto& i : arc) {
		findContours(denoised(i), cont, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_NONE);
		cont.erase(remove_if(cont.begin(), cont.end(), 
			[](const vector<Point>& v) {return v.size() < 10; }), cont.end());
		if (!cont.empty()) {
			auto a = recArc(cont[0], i.tl());
			dealWithLink(a);
			cont.clear();
		}
	}
}

/*
	rescan a region, with the purpose of searching any missed notes. 
	@param pos, unsigned. pos to rescan, the exact area is [pos - maxCharacterWidth, pos + maxCharacterWidth). 
	@return true if any notes are detected. false if no note is detected. 
*/
bool Measure::rescan(unsigned pos, Value v) {
	Rect rect(pos - maxCharacterWidth, rows[0][1] - maxCharacterHeight / 2, 
		2 * maxCharacterWidth, rows[5][1] - rows[0][1] + maxCharacterHeight / 2);
	rect &= Rect(Point(0, 0), denoised.size());
	Mat img = denoised(rect);
	vector<vector<Point>> cont;
	vector<Rect> region;

	findContours(img, cont, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_NONE);
	region.resize(cont.size());
	transform(cont.begin(), cont.end(), region.begin(), [](const vector<Point>& c) {return boundingRect(c); });
	region.resize(distance(region.begin(), remove_if(region.begin(), region.end(), 
		[&rect](const Rect& x) {return x.area() > 0.9 * rect.area(); })));
	vector<vector<Point>>().swap(cont);
	vector<EasyNote> tmp;

	for (const auto& i : region) {
		if(countBlack(img(i)) > 0.9 * i.area()) continue;

		EasyNote newNote = recNum(Rect(rect.tl() + i.tl(), i.size()));
		if (newNote.string < 0) continue;
		tmp.emplace_back(newNote);
	}
	if (tmp.empty()) return false;

	sort(tmp.begin(), tmp.end(), [](const EasyNote x, const EasyNote y) -> bool {
		return x.pos < y.pos || (x.pos == y.pos && x.string < y.string);
	});
	mergeFret(tmp);
	for (auto& i : tmp) i.time = v;

	fillTimeAndPos(tmp);
	return true;
}

void Measure::recTime() {
	map<unsigned, Value> timeValue;
	unsigned t = maxCharacterWidth;
	Mat picValue = org(Range(max(rows[5][1], rows[5][3]) + maxCharacterHeight / 2, org.rows), Range::all()).clone();
	//TODO: bug if no value area in org. 
	//		present as max(rows[5][1], rows[5][3]) + maxCharacterHeight / 2 > org.rows, and cvException here. 

	Mat inv;
	vector<vector<Point>> cont;
	auto predictLenth = [&inv, &picValue, &cont, this]() -> int {
		int predLen = 0;
		if (org.rows < global["rowLenth"] * 2 && org.rows > global["rowLenth"] / 2) {
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
			const_cast<GlobalUnit&>(global["valueSignalLen"]) += predLen;
		}
		else {
			predLen = global["valueSignalLen"];
		}
		return predLen;
	};
	auto time_denoise = [&cont]() -> vector<Rect>{
		vector<Rect> region(cont.size());
		std::transform(cont.begin(), cont.end(), region.begin(), [](const vector<Point>& x) {return boundingRect(x); });
		while (1) {
			auto m = find_if(region.begin(), region.end(), [&region](Rect x) ->bool {
				return any_of(region.begin(), region.end(), [&](Rect y) ->bool {
					return y.br().y < x.y
						&& y.height > x.height;
				});
			});
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

	for (const Rect &i: region) {
		int sum1 = 0, sum2 = 0, sum3 = 0;
		sum1 = count(picValue, i, -2);
		sum2 = count(picValue, i, 2);
		sum3 = count(picValue, i, 1);
		if (i.height <= predLen / 2 && !sum1 && !sum2) {
			sum1--; sum2--;										//TODO: 我写的这是啥?
																//PS. 这好像是检查二分音符
		}
		Value newc = time.beat_type / (float)pow(2, max(sum1, !sum3 && sum2 ? sum2 - 1 : sum2));
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
	
	Value v = Value::whole;
	for (auto& i : notes) {
		auto ub = intervalAccess(timeValue, i.first, t);
		v = ub == timeValue.end() ? v : ub->second;
		//错误: 找不到对应的时值. 处理方案: 置上一个note的时值. 
		for (auto& j : i.second) j.time = v;
		if(ub != timeValue.end()) timeValue.erase(ub);
	}

	for (auto i : timeValue) {
		if(!rescan(i.first, i.second))
			notes.insert(make_pair(i.first, ChordSet::rest(i.first, i.second)));
	}
}

MusicMeasure Measure::getNotes() const {
	MusicMeasure r;
	r.time = this->time;
	r.key = this->key;
	r.id = this->id;
	for (auto& i : notes) {
		note newn;
		newn.chord = false;
		for (unsigned j = 0; j < i.second.size(); j++) {
			newn.timeValue = i.second[j].time;
			newn.notation.technical.string = i.second[j].string;
			newn.notation.technical.fret = i.second[j].fret;
			newn.notation.notation = i.second[j].getNotation();
			r.notes.emplace_back(newn);
			newn.chord = true;
		}
	}
	return r;
}

EasyNote Measure::dealWithIt(const Rect& region) {
	Mat img = org(region);
	auto tooWide_MORPH = [this, &img, &region]() -> EasyNote {
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
				&& (global["characterWidth"].isInit() ? (i.width > global["characterWidth"] / 2) : 1)
				&& i.height > i.width) {
				EasyNote newNote;
				newNote.acceptRecData(reader->rec(img(i), 45.0f));
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
	auto tooHigh_MORPH = [this, &img, &region]() -> EasyNote {
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
				&& (global["characterWidth"].isInit() ? (i.width > global["characterWidth"] / 2) : 1)
				&& i.height > i.width) {
				EasyNote newNote;
				newNote.acceptRecData(reader->rec(img(i), 45.0f));
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
	auto tooWide_Rec = [this, &img, &region]() -> EasyNote {
		int trueWidth = static_cast<int>(float(img.rows) / maxCharacterHeight * maxCharacterWidth);
		for (int i = 0; i < img.cols - trueWidth; i++) {
			Mat test = img(Range::all(), Range(i, i + trueWidth));
			EasyNote newNote;
			newNote.string = whichLine(region, rows);
			newNote.pos = i + trueWidth / 2;
			newNote.acceptRecData(reader->rec(test, 25.0f));
			if (newNote.fret > 0 && newNote.string) return newNote;
		}
		return EasyNote::invalid();
	};
	auto tooHigh_Rec = [this, &img, &region]() -> EasyNote {
		int trueHeight = int(float(img.rows) / maxCharacterHeight * maxCharacterWidth);
		for (int i = 0; i < img.rows - trueHeight; i++) {
			Mat test = img(Range(i, i + trueHeight), Range::all());
			EasyNote newNote;
			newNote.acceptRecData(reader->rec(test, 25.0f));
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
	: id(id), ImageProcess(origin), reader(root->Reader.operator CharReader*())
{
	maxCharacterWidth = global["characterWidth"] / 2;
	maxCharacterHeight = maxCharacterWidth + 1;
}

void Measure::start(const vector<Vec4i>& rows, const vector<int>& width) {
	assert(rows.size() == 6);
	Denoiser den(org);
	this->denoised = 255 - den.denoise_morphology();
	Denoiser::inpaint(denoised, rows, width);
	this->rows = rows;

	try {
		recStaffLines();
		recTime();
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

/*
	mergeFret v3
	requires: note[0] is sorted. 
*/
void Measure::mergeFret(std::vector<EasyNote>& noteSet) {				
	for (auto i = noteSet.begin(); i != noteSet.end();) {
		for (auto j = i + 1; j != noteSet.end(); j++) {
			if (bool flag = false; j->pos - i->pos <= ((unsigned)maxCharacterWidth << 1)) {
				if (i->string != j->string) {			//can be aligned.
					i->pos = j->pos = (i->pos + j->pos) >> 1;
					continue;
				}
				for (auto k : i->possible) {				//can be merged. 
					if (k.second > '1') continue;
					i->fret = 10 * (k.second - '0') + j->fret;
					i->pos = (i->pos + j->pos) >> 1;
					j = noteSet.erase(j) - 1;
					flag = true;
					break;
				}
				if (!flag) raiseErr("fret合并：不可置信条件. 检查模型", 0);
			}
			else break;
		}
		i++;
	}
};

void Measure::fillTimeAndPos(vector<EasyNote>& noteSet) {
	for (const EasyNote& i : noteSet) {
		auto add = intervalAccess(notes, i.pos, maxCharacterWidth / 2);
		if (add == notes.end()) {
			ChordSet newc({ i });
			notes[i.pos] = newc;
		}
		else {
			unsigned sum = i.pos;
			for (const EasyNote& j : add->second) sum += j.pos;
			sum /= static_cast<unsigned>(add->second.size() + 1);
			if (add->first != sum) {
				auto p = make_pair(sum, add->second);
				p.second.emplace_back(i);
				notes.insert(p);
				notes.erase(add);
			}
		}
	}
};

EasyNote Measure::recNum(const cv::Rect& rect) {
	EasyNote newNote;
	Mat number = org(rect).clone();
	if (countBlack(number) > 0.8 * rect.area()) return EasyNote::invalid();

	newNote.string = whichLine(rect, rows);											//几何关系判断string
	if (!newNote.string) return EasyNote::invalid();
	try {
		newNote.acceptRecData(reader->rec(number));									//识别数字fret
		if (newNote.fret < 0) return EasyNote::invalid();							//较大的几率不是数字
	}
	catch (runtime_error ex) {
		throw ex;
	}

	newNote.pos = rect.width / 2 + rect.x;
	const_cast<GlobalUnit&>(global["characterWidth"]) += rect.width;

	maxCharacterWidth = max(maxCharacterWidth, rect.width);
	maxCharacterHeight = max(maxCharacterHeight, rect.height);
	return newNote;
}

void Measure::dealWithLink(const pair<Vec4i, double>& arcVec) {
	const static Vec4i _0{ 0, 0, 0, 0 };
	if (arcVec.first == _0) return;

	auto lf = notes.lower_bound(arcVec.first[0]);
	auto rt = notes.lower_bound(arcVec.first[2]);

	//bug: cont is eroded. so arcVec is eroded. may effect position relations with rows grid...
	auto ub = upper_bound(rows.begin(), rows.end(), arcVec.first[1], 
		[](int v, const Vec4i& x) {return v < x[1]; }
	);
	int string = static_cast<int>(distance(rows.begin(), ub));

	if (arcVec.second < 0) string--;		//upward
	assert(string >= 0);
	string++;

	auto l_it = intervalAccess(notes, arcVec.first[0], maxCharacterWidth);
	auto r_it = intervalAccess(notes, arcVec.first[2], maxCharacterHeight);

	/*if (l_it == notes.end()) {
		notes.insert(make_pair(arcVec.first[0], ChordSet::rest(arcVec.first[0], Value::none)));
		l_it = notes.lower_bound(arcVec.first[0]);
	}*/
	if (r_it == notes.end() && arcVec.first[2] < denoised.cols) {
		notes.insert(make_pair(arcVec.first[2], ChordSet::rest(arcVec.first[2], Value::none)));
		r_it = notes.lower_bound(arcVec.first[2]);
	}

	auto lnote = l_it == notes.end() ? nullptr : l_it->second.at(string);
	auto rnote = r_it == notes.end() ? nullptr : r_it->second.at(string);

	if (!lnote) {
		return;
	}

	if (rnote) {
		const_cast<EasyNote*>(lnote)->addNotation('H');
		const_cast<EasyNote*>(rnote)->addNotation('h');
	}
	else {
		const_cast<ChordSet&>(r_it->second).removeRest();
		const_cast<ChordSet&>(r_it->second).push_back(*lnote);
		rnote = r_it->second.at(string);
		const_cast<EasyNote*>(rnote)->pos = r_it->first;

		const_cast<EasyNote*>(lnote)->addNotation('L');
		const_cast<EasyNote*>(rnote)->addNotation('l');
	}
}

inline ImageProcess::ImageProcess(cv::Mat origin) : org(origin), global(root->Global) {
	assert(org.empty() != true);
}
