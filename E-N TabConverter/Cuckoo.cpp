#include "stdafx.h"
#include "Cuckoo.h"
#include "Dodo.h"
#include "global.hpp"
#include "imgproc.hpp"
#include "opencv.hpp"

using namespace cv;

#define find_in(vector, lambda) find_if(vector##.begin(), vector##.end(), lambda)
#if _DEBUG
#define Showrectangle if(0)
#define Showline if(0)
#define Showdenoise if(0)
#define draw(func, img, from, to, color) Show##func func(img, from, to, color)
#define imdebug(title, img) imshow((title), img); cv::waitKey()
#else 
#define draw(func, img, from, to, color)
#define Showrectangle /##/
#define Showline /##/
#define Showdenoise /##/
#define imdebug(img, title)
#endif

bool savepic = 0;
extern notify<string> notification;

static int count(Mat img, Vec4i range, int delta) {
	bool lock = false;
	int sum = 0, x = range[delta > 0 ? 2 : 0] + delta;
	//int blocksize = 2 * (int)max(1.0, round(global->col / 1000.0)) + 1;
	for (int y = (range[1] + range[3]) / 2; y <= range[3]; y++)
	{
		uchar *ptr1 = img.ptr<uchar>(y);
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
			&& tmp.width < 0.8* denoised.cols
			) {
			region.emplace_back(tmp);
		}
	}
	cont.clear();

	for (Rect& i: region) {
		//限定筛选
		
		if (i.height < 5 * i.width
			&& (global->characterWidth ? (i.width > global->characterWidth / 2) : 1)
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
			if (countBlack(number) > 0.8* i.area()) continue;

			cvtColor(number, number, CV_GRAY2BGR);

			number = perspect(number, 8, 14);
			if (savepic) {
				savePic(picFolder, number);													//保存数字样本
				continue;
			}

			newNote.string = whichLine(i, rows);											//几何关系判断string
			if (!newNote.string) continue;
			try {
				newNote.fret = rec(number, newNote.possible, newNote.safety);				//识别数字fret
				if (newNote.fret < 0) continue;												//较大的几率不是数字
			}
			catch (runtime_error ex) {
				throw ex;
			}

			draw(rectangle, ccolor, i.tl(), i.br(), Scalar(0, 0, 255));

			newNote.pos = i.width / 2 + i.x;
			global->characterWidth += i.width;

			maxCharacterWidth = max(maxCharacterWidth, i.width);
			maxCharacterHeight = max(maxCharacterHeight, i.height);
			notes[0].chords.emplace_back(newNote);
		}
	}
	for (Rect& i : possible) {
		//TODO: blocked, like 3--3
		if (i.area() < 0.8* maxCharacterHeight* maxCharacterWidth) continue;
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
	notes.erase(notes.begin());
}

void measure::recTime(vector<Vec4i> rows) {
	typedef struct {
		value time = whole;
		bool dot = false;
		int pos = 0;
	}timeComb;
	int t = maxCharacterWidth;
	Mat picValue = org(Range(max(rows[5][1], rows[5][3]) + maxCharacterHeight / 2, org.rows), Range::all()).clone();
	Mat inv;
	vector<vector<Point>> cont;
	vector<timeComb> TimeValue;
	auto predictLenth = [&inv, &picValue, &cont, this]() -> int {
		int predLen = 0;
		if (org.rows < global->rowLenth * 2 && org.rows > global->rowLenth / 2) {
			inv = 255 - Morphology(picValue, picValue.rows / 3, false, true);
			findContours(inv, cont, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_NONE);

			//imdebug("2", picValue);
			Vec2i temp = { picValue.rows,0 };
			int tr = picValue.rows / 4;
			while (!cont.size()) {
				inv = 255 - Morphology(picValue, tr--, false, true);
				findContours(inv, cont, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_NONE);
				if (tr < 2) {
					for (int i = 0; i < picValue.rows; i++) {
						if (!isEmptyLine(picValue, i, 0.04)) raiseErr("未扫描到纵向结构", 6);
					}
					return 0;
				}
			}

			for (int i = 0; i < cont.size(); i++) {
				for (int j = 0; j < cont[i].size(); j++) {
					temp[0] = min(temp[0], cont[i][j].y);
					temp[1] = max(temp[1], cont[i][j].y);
				}
				predLen = max((int)predLen, temp[1] - temp[0]);
			}
			global->valueSignalLen += predLen;
		}
		else {
			predLen = global->valueSignalLen;
		}
		return predLen;
	};
	auto time_denoise = [&cont]() {
		auto m = cont.end();
		while (1) {
			m = find_in(cont, ([cont](vector<Point> x) ->bool {
				auto l = [](Point m, Point n) ->bool {
					return m.y < n.y;
				};
				int up = min_element(x.begin(), x.end(), l)->y;
				int len = max_element(x.begin(), x.end(), l)->y - up;
				return find_in(cont, ([cont, x, l, up, len](vector<Point> y) ->bool {
					int max = max_element(y.begin(), y.end(), l)->y;
					return max < up
						&& max - min_element(y.begin(), y.end(), l)->y > len;
				})) != cont.end();
			}));
			if (m == cont.end()) break;
			else cont.erase(m);
		};
		std::sort(cont.begin(), cont.end(), [](vector<Point> x, vector<Point> y) ->bool {
			return x[0].x < y[0].x;
		});
	};
	//==========================================local and lambda=================================================
	//==========================================local and lambda=================================================
	//==========================================local and lambda=================================================
	int predLen = predictLenth();
	if (!predLen) return;

	inv = 255 - Morphology(picValue, int(round(predLen * 0.3)), false, true);
	findContours(inv, cont, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_NONE);

	time_denoise();						//去掉节拍记号再往下的乱七八糟的东西

	for (int i = 0; i < cont.size(); i++) {
		Vec4i tmp = { picValue.cols,picValue.rows,0,0 };
		for (int j = 0; j < cont[i].size(); j++) {
			tmp[0] = min(tmp[0], cont[i][j].x);
			tmp[2] = max(tmp[2], cont[i][j].x);
			tmp[1] = min(tmp[1], cont[i][j].y);
			tmp[3] = max(tmp[3], cont[i][j].y);
		}

		int sum1 = 0, sum2 = 0, sum3 = 0;
		sum1 = count(picValue, tmp, -2);
		sum2 = count(picValue, tmp, 2);
		sum3 = count(picValue, tmp, 1);
		if (tmp[3] - tmp[1] <= predLen / 2 && !sum1 && !sum2) {
			sum1--; sum2--;
		}
		timeComb newc;
		newc.dot = !sum3 && sum2;
		newc.time = time.beat_type / (int)pow(2, max(sum1, !sum3 && sum2 ? sum2 - 1 : sum2));
		newc.pos = (tmp[0] + tmp[2]) / 2;
		TimeValue.emplace_back(newc);
	}
	Value kk = TimeValue[0].time;
	for (unsigned i = 1; i < TimeValue.size(); i++) {
		if (TimeValue[i].time != kk) goto distribute;
	}
	kk = time.beat_type * (time.beats / (float)TimeValue.size());		//防止4个全音符或者2个四分音符之类的情况
	for (timeComb& i : TimeValue) i.time = kk;

distribute:
	if (TimeValue.size() == 1 && notes.size() == 1) {
		//全音符
		kk = time.beat_type * (float)time.beats;
		for (easynote& i : notes[0].chords) i.time = kk;
		return;
	}
	//不应该含全音符


	vector<ChordSet*> noValue;
	for (ChordSet& i : notes) noValue.emplace_back(&i);
	
	for (int i = 0; i < TimeValue.size(); i++) {
		int pos = TimeValue[i].pos;
		for (;;) {
			auto s = find_in(noValue, ([pos, t](const ChordSet* x)->bool {
				return abs(x->avrpos - pos) < t;
			}));
			if (s == noValue.end()) break;
			for (easynote& j : (**s).chords) {
				j.time = TimeValue[i].time;
				j.time.dot = TimeValue[i].dot;
			}
			noValue.erase(s);
		}
	}
	if (!noValue.empty()) raiseErr("有未分配时值的乐符", 1);
}

vector<note> measure::getNotes() {
	vector<note> r;
	for (ChordSet& i : notes) {
		if (i.chords.empty()) continue;
		note newn;
		newn.chord = false;  newn.timeValue = i.chords[0].time;
		newn.notation.technical.string = i.chords[0].string;
		newn.notation.technical.fret = i.chords[0].fret;
		r.emplace_back(newn);
		newn.chord = true;
		for (unsigned j = 1; j < i.chords.size(); j++) {
			newn.timeValue = i.chords[j].time;
			newn.notation.technical.string = i.chords[j].string;
			newn.notation.technical.fret = i.chords[j].fret;
			r.emplace_back(newn);
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
				&& (global->characterWidth ? (i.width > global->characterWidth / 2) : 1)
				&& i.height > i.width) {
				easynote newNote;
				newNote.fret = rec(img(i), newNote.possible, newNote.safety, 45.0f);
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
			if (i.height < 2.5* i.width
				&& (global->characterWidth ? (i.width > global->characterWidth / 2) : 1)
				&& i.height > i.width) {
				easynote newNote;
				newNote.fret = rec(img(i), newNote.possible, newNote.safety, 45.0f);
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
			newNote.fret = rec(test, newNote.possible, newNote.safety, 25.0f);
			if (newNote.fret > 0) return newNote;
		}
		return easynote::invalid();
	};
	auto tooHigh_Rec = [this, &img]() -> easynote {
		int trueHeight = int(float(img.rows) / maxCharacterHeight * maxCharacterWidth);
		for (int i = 0; i < img.rows - trueHeight; i++) {
			Mat test = img(Range(i, i + trueHeight), Range::all());
			easynote newNote;
			newNote.fret = rec(test, newNote.possible, newNote.safety, 25.0f);
			if (newNote.fret > 0) return newNote;
		}
		return easynote::invalid();
	};
	if (img.cols > maxCharacterWidth&& img.rows < maxCharacterHeight) {
		if (img.cols > 2 * maxCharacterWidth) return tooWide_Rec();
		else return tooWide_MORPH();
	}
	else if (img.cols < maxCharacterWidth&& img.rows > maxCharacterHeight) {
		if (img.rows > 2 * maxCharacterWidth) return tooHigh_Rec();
		else return tooHigh_MORPH();
	}
	else return easynote::invalid();
}

measure::measure(Mat origin, vector<Vec4i> rows, size_t id)
	:id(id), org(origin)
{
	maxCharacterWidth = global->characterWidth / 2;
	maxCharacterHeight = maxCharacterWidth + 1;
	Denoiser den(org);
	Mat img = den.denoise_morphology();
	if (org.cols < global->colLenth / 5) {
		this->id = 0;
		return;
	}
	try {
		recNum(img, rows);
		if (savepic) {
			id = 0;
			return;
		}
		recTime(rows);
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

void Splitter::start(vector<Mat>& piece) {
	Mat r = Morphology(255 - org, org.cols / 2, true, true);
	r = Morphology(r, org.cols / 100, false, true);
	Mat ccolor;
	cvtColor(org, ccolor, CV_GRAY2BGR);
	vector<vector<Point>> cont;
	findContours(r, cont, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_NONE);

	size_t n = cont.size();
	vector<Rect> region(n);
	piece.resize(cont.size());

	for (size_t k = 0; k < n; k++) region[k] = boundingRect(cont[k]);
	std::sort(region.begin(), region.end(), [](const Rect x, const Rect y) -> bool {return x.y < y.y; });

	for (size_t k = 0; k < n; k++) piece[k] = org(region[k]).clone();

}

Mat Denoiser::denoise_morphology() {
	Mat mask1, mask2, r;
	auto fullfill = [](Mat& mask) {
		vector<vector<Point>> cont;
		findContours(mask, cont, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE);
		size_t n = cont.size();
		vector<Rect> region(n);
		for (int i = 0; i < n; i++)  rectangle(mask, boundingRect(cont[i]), Scalar(255), -1);
	};
	mask1 = Morphology(255 - org, org.cols / 10, true, false);
	fullfill(mask1);
	mask2 = Morphology(255 - org, org.rows / 6, false, false);
	fullfill(mask2);
	r = mask1 | mask2 | org;
	
	Showdenoise imdebug("denoise(Morphology)", r);
	return r;
}

Mat Denoiser::denoise_inpaint(vector<Vec4i> lines, double radius) {
	Mat r, mask = Mat(org.size(), CV_8UC1, Scalar::all(0));
	for (Vec4i& i : lines) line(mask, Point(i[0], i[1]), Point(i[2], i[3]), Scalar(255));

	inpaint(org, mask, r, radius, INPAINT_TELEA);
	threshold(r, r, 200, 255, THRESH_BINARY);

	Showdenoise imdebug("denoise(Inpaint)", r);

	return r;
}

void LineFinder::findRow(vector<Vec4i>& lines) {
	/*
	* @brief			概率霍夫变换提取横线
	* @param[in]		lines，筛选后的结果
	*/

	//Mat dilated, eroded;
	//Mat hline = getStructuringElement(MORPH_RECT, Size(max(img.cols / 120, 1),1));						//竖直结构
	//erode(img, eroded, hline);																			//腐蚀
	//dilate(eroded, dilated, hline);																		//膨胀
	//Mat inv = 255 - dilated;
	HoughLinesP(255 - img, lines, 1, CV_PI / 180, 100, img.cols * 2 / 3, img.cols / 50);
	//角度筛选
	for (size_t i = 0; i < lines.size(); i++) {
		if (lines[i][2] == lines[i][0]) lines.erase(lines.begin() + i--);
		else if (abs(((float)lines[i][3] - lines[i][1]) / (lines[i][2] - lines[i][0])) > tan(range)) {
			lines.erase(lines.begin() + i--);
			continue;
		}
	}
	sort(lines.begin(), lines.end(), [](const Vec4i x, const Vec4i y) ->bool {
		return x[1] < y[1];
	});
	if (!lines.size()) return;
	//去除虚线
	for (int i = 0; i < lines.size(); i++) {
		if (isDotLine(lines[i])) {
			lines.erase(lines.begin() + i--);
		}
	}
	//去除连续
	vector<Vec4i> copy(lines);
	for (size_t i = 0; i < lines.size(); i++) {
		if (i == lines.size() - 1) {
			thickness.push_back(1);
			break;
		}
		if (lines[i][1] + 1 >= lines[i + 1][1]) {
			size_t st = i, ed = i + 1;
			for (size_t j = i + 1; j < lines.size() - 1; j++) {
				if (lines[j][1] + 1 >= lines[j + 1][1]) {
					ed = j + 1;
				}
				else {
					break;
				}
			}
			size_t d = (st + ed) / 2;
			thickness.emplace_back(int(ed - st + 1));
			for (int j = (int)ed; j >= (int)st; j--) {
				if (j == d) continue;
				lines.erase(lines.begin() + j);
			}
		}
		else thickness.push_back(1);
	}
	for (int i : thickness) global->lineThickness += i;
	upper = std::min(lines[5][1], lines[5][3]);
	lower = std::max(lines[0][1], lines[0][3]);
#if 0
	Mat color;
	cvtColor(img, color, CV_GRAY2BGR);
	for (Vec4i& i: lines) {
		line(color, Point(i[0], i[1]), Point(i[2], i[3]), Scalar(0, 0, 255));
	}
	imdebug("Cuckoo Debug: Show rows", color);
#endif
}

void LineFinder::findCol(vector<Vec4i> & lines) {
	/*
	 * 函数：findCol
	 * @brief				概率霍夫变换提取竖线
	 * param[in]			lines，筛选后的结果
	*/
	int length = upper - lower;
	int thick = max(1, *max_element(thickness.begin(), thickness.end()));
	Mat toScan = img(Range(lower, upper), Range::all()).clone();
	toScan = Morphology(toScan, max(thick, 3), false, false);
	Mat inv = 255 - Morphology(toScan, max(toScan.rows / 2, 1), false, true);
	//80:140
	HoughLinesP(inv, lines, 1, CV_PI / 180, (toScan.rows - 2 * thick) / 2, toScan.rows - 2 * thick, thick + 2);
	for (size_t i = 0; i < lines.size(); i++) {
		if (lines[i][2] == lines[i][0]) continue;
		else if (abs(((float)lines[i][3] - lines[i][1]) / (lines[i][2] - lines[i][0])) < tan(CV_PI / 2.0 - range))
			lines.erase(lines.begin() + i--);
	}
	sort(lines.begin(), lines.end(), [](const Vec4i x, const Vec4i y) ->bool {
		return x[0] < y[0];
	});
	//去除连续
	for (int i = 0; i < (int)lines.size() - 1; i++) {
		if (lines[i][0] + 1 >= lines[i + 1][0]) {
			size_t st = i, ed = i + 1;
			for (int j = i + 1; (int)j < lines.size() - 1; j++) {
				if (lines[j][0] + 1 >= lines[j + 1][0]) {
					ed = j + 1;
				}
				else {
					break;
				}
			}
			size_t d = (st + ed) / 2;
			Vec4i tmp = lines[d];
			lines.erase(lines.begin() + st, lines.begin() + ed + 1);
			lines.insert(lines.begin() + st, tmp);
		}
	}
	for (size_t i = 1; i < lines.size(); i++) {
		if (lines[i][0] - lines[i - 1][0] >= max(16, global->colLenth / 5)) {
			global->colLenth += lines[i][0] - lines[i - 1][0];
		}
	}
#if 0
	Mat ccolor;
	cvtColor(img, ccolor, CV_GRAY2BGR);
	for (Vec4i& i : lines) {
		line(ccolor, Point(i[0], 0), Point(i[2], img.rows), Scalar(0, 0, 255));
	}
	imdebug("Cuckoo Debug: Show cols", ccolor);
#endif
}


bool LineFinder::isDotLine(Vec4i line) {
	/*
	 * @brief 判断一条直线是否为虚线
	*/
	auto y = [line](int x) -> int {
		return line[1] + (line[3] - line[1]) / (line[2] - line[0]) * (x - line[0]);
	};
	int sum = 0;
	for (int i = line[0]; i <= line[2]; i++) {
		if (!img.at<uchar>(y(i), i)) {
			sum++;
		}
	}
	if (sum < 0.7* (line[2] - line[0])) {
		return true;
	}
	return false;
}