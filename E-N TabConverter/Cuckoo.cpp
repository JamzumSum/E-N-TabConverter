#include "Cuckoo.h"
#include "Dodo.h"
#include "global.h"
#include "imgproc.hpp"
#include "opencv.hpp"

using namespace std;
using namespace cv;

#if _DEBUG
#define Showrectangle if(0)
#define Showline if(0)
#define draw(func, img, from, to, color) Show##func func(img, from, to, color)
#define imdebug(img, title) imshow((img), title); cv::waitKey()
#else 
#define draw(func, img, from, to, color)
#define Showrectangle /##/
#define Showline /##/
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

void measure::recNum(Mat section, vector<Vec4i> rows) {
	/*
	* 函数：measure::recNum
	* 功能：从传入图像中提取数字等
	* 参数：
		section，Mat，传入图像
		rows，Vec4i，传入的网格信息（谱线）
	*/
	vector<vector<Point>> cont;
	vector<Rect> region, possible;
	Mat inv = 255 - section;
	Mat ccolor;
	auto mergeFret = [this](int t) {
		auto n = notes[0].chords.end();
		auto m = notes[0].chords.end();
		while (1) {
			//合并相邻的fret version2
		mfind:
			m = find_if(notes[0].chords.begin(), notes[0].chords.end(), [this, t, &n](const easynote x) ->bool {
				n = find_if(notes[0].chords.begin(), notes[0].chords.end(), [x, t](const easynote y) ->bool {
					return x.pos != y.pos
						&& y.string == x.string
						&& abs(x.pos - y.pos) <= 2 * t;
				});
				return n != notes[0].chords.end();
			});
			if (m == notes[0].chords.end()) break;
			else {
				if (m->pos > n->pos) swap(m, n);
				if (m->fret > 1) {
					for (int i : m->possible) {
						if (i < 2) {
							m->pos = (m->pos + n->pos) / 2;
							m->fret = 10 * i + n->fret;
							notes[0].chords.erase(n);
							goto mfind;
						}
					}
					err ex = { 0,__LINE__,"fret合并：不可置信条件. 检查模型" };
					throw ex;
				}
				m->pos = (m->pos + n->pos) / 2;
				m->fret = 10 + n->fret;
				notes[0].chords.erase(n);
			}
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
	Showrectangle cvtColor(section, ccolor, CV_GRAY2BGR);

	notes.resize(1);

	findContours(inv, cont, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_NONE);
	for (vector<Point>& i : cont) {								//convert contour to rect
		Rect tmp = boundingRect(i);
		if (tmp.area() > 9
			&& tmp.height > 3
			&& tmp.width > 3
			&& tmp.height < 0.8 * section.rows
			&& tmp.width < 0.8* section.cols
			) {
			region.emplace_back(tmp);
		}
	}
	cont.shrink_to_fit();

	for (Rect& i: region) {
		//限定筛选
		if (i.height < rows[1][1] - rows[0][1]					//网格限定
			&& i.height < 5 * i.width
			&& (global->characterWidth ? (i.width > global->characterWidth / 2) : 1)
		) {
			if (i.height <= i.width) {
				//TODO: 形状异常
				if(i.height > maxCharacterHeight / 2) possible.emplace_back(i);
				continue;
			}
			easynote newNote;
			Mat number = section(i).clone();
			if (countBlack(number) > 0.8* i.area()) continue;

			cvtColor(number, number, CV_GRAY2BGR);

			number = perspect(number, 8, 10);
			if (savepic) savePic(picFolder, number);										//保存数字样本

			newNote.string = whichLine(i, rows);											//几何关系判断string
			if (!newNote.string) continue;
			try {
				newNote.fret = rec(number, newNote.possible);								//识别数字fret
			}
			catch (err ex) {
				switch (ex.id) {
				case 5:
				default: throw ex; break;
				}
			}


			draw(rectangle, ccolor, i.tl(), i.br(), Scalar(0, 0, 255));

			newNote.pos = i.width / 2 + i.x;
			global->characterWidth += i.width;

			maxCharacterWidth = max(maxCharacterWidth, i.width);
			maxCharacterHeight = max(maxCharacterHeight, i.height);
			notes[0].chords.emplace_back(newNote);
		}
	}
	/*for (Rect& i : possible) {
		Mat what = section(i);
		imdebug("possible number", what);
	}*/
	Showrectangle imdebug("Showrectangle", ccolor);


	mergeFret(maxCharacterWidth);
	sort(notes[0].chords.begin(), notes[0].chords.end(), [](const easynote x, const easynote y) -> bool {
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
						if (!isEmptyLine(picValue, i, 0.04)) {
							err ex = { 6,__LINE__,"未扫描到纵向结构" };
							throw ex;
						}
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
			m = find_if(cont.begin(), cont.end(), [cont](vector<Point> x) ->bool {
				auto l = [](Point m, Point n) ->bool {
					return m.y < n.y;
				};
				int up = min_element(x.begin(), x.end(), l)->y;
				int len = max_element(x.begin(), x.end(), l)->y - up;
				return find_if(cont.begin(), cont.end(), [cont, x, l, up, len](vector<Point> y) ->bool {
					int max = max_element(y.begin(), y.end(), l)->y;
					return max < up
						&& max - min_element(y.begin(), y.end(), l)->y > len;
				}) != cont.end();
			});
			if (m == cont.end()) break;
			else cont.erase(m);
		};
		sort(cont.begin(), cont.end(), [](vector<Point> x, vector<Point> y) ->bool {
			return x[0].x < y[0].x;
		});
	};
	//==========================================local and lambda=================================================
	//==========================================local and lambda=================================================
	//==========================================local and lambda=================================================
	int predLen = predictLenth();
	if (!predLen) return;

	inv = 255 - Morphology(picValue, round(predLen * 0.3), false, true);
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
			auto s = find_if(noValue.begin(), noValue.end(), [pos, t](const ChordSet* x)->bool {
				return abs(x->avrpos - pos) < t;
			});
			if (s == noValue.end()) break;
			for (easynote& j : (**s).chords) {
				j.time = TimeValue[i].time;
				j.time.dot = TimeValue[i].dot;
			}
			noValue.erase(s);
		}
	}
	if (!noValue.empty()) {
		err ex = { 1,__LINE__,"有未分配时值的乐符" };
		throw ex;
	}
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

measure::measure(Mat org, Mat img, vector<Vec4i> rows, int id)
	:id(id), org(org)
{
	maxCharacterWidth = global->characterWidth / 2;
	maxCharacterHeight = maxCharacterWidth + 1;
	if (org.cols < global->colLenth / 5) {
		this->id = -1;
		return;
	}
	try {
		recNum(img, rows);
		recTime(rows);
	}
	catch (err ex) {
		switch (ex.id)
		{
		case 0: break;									//包含不合理数据
		case 1:											//timeValue越界
			imdebug(ex.description, org);
			break;
		case 6:											//没有竖直结构用以判断时值
			imdebug(ex.description, org);
			break;
		default: throw ex; break;
		}
	}
}

void splitter::KClassify(vector<bool> &classifier) {
	//函数名：KClassify
	//功能：分类对象是空白区域 struct _space，分类依据为space.lenth
	//输出：classifier 算法将各区域分入两类，一类为false，一类为true

	//k1 15:1800    k2 5:1800  （原始数据 此行无效）
	assert(collection.size() == classifier.size());
	int k1 = 18, k2 = 5;
	unsigned min = collection[0].length, max = min;
	for (int i = 0; i < collection.size(); i++) {
		if (collection[i].length > max) {
			max = collection[i].length;
		}
		else if (collection[i].length < min) {
			min = collection[i].length;
		}
	}
	for (int i = 0; i < collection.size(); i++) {
		if (min + k1 > collection[i].length) {
			classifier[i] = false;
		}
		else if (max - k2 < collection[i].length) {
			classifier[i] = true;
		}
		else {
			if (collection[i].length - min - k1 < max - k2 - collection[i].length) {
				k1 = collection[i].length - min + 1;
				classifier[i] = false;
			}
			else if (collection[i].length - min - k1 > max - k2 - collection[i].length) {
				k2 = max - collection[i].length + 1;
				classifier[i] = true;
			}
			else {
				//要是等于就不要他了hhh
				classifier[i] = false;
			}
		}
	}
}

void splitter::split() {
	bool flag = false;
	unsigned row = org.rows;

	for (unsigned st = org.rows, i = 0; i < row; i++) {
		//初步设为0.04 以后再说
		if (isEmptyLine(org, i, 0.004)) {
			if (!flag) {
				st = i;
				flag = true;
			}
		}
		else {
			if (flag) {
				if (i - 1 - st > 0) collection.push_back({ st,i - 1u - st });
				flag = false;
			}
		}
	}
	if (collection.size() < 2) {
		collection.clear();
		notification = "裁剪失败，等待二次裁剪";
		//二次裁剪为缩减判断空行的范围，从之前的从像素x=0 至x=col到检测到的横线的x1至x2
		vector<Vec4i> rows;
		vector<int> thick;
		Mat toOCR;
		findRow(org, toOCR, CV_PI / 18, rows, thick);
		if (rows.size() > 5) {
			notification =  "二次裁剪开始.";
			flag = false;
			for (unsigned st = org.rows, i = min(rows[0][1], rows[0][3]); i < row; i++) {
				if (isEmptyLine(org, i, max(min(rows[0][0], rows[0][2]), min(rows[5][0], rows[5][2])),
					min(max(rows[0][0], rows[0][2]), max(rows[5][0], rows[5][2])), 0.01)) {
					if (!flag) {
						st = i;
						flag = true;
					}
				}
				else {
					if (flag) {
						if (i - 1 - st > 0) collection.push_back({ st,i - 1u - st });
						flag = false;
					}
				}
			}

		}
		if (collection.size() < 2) {
			err ex = { 4,__LINE__,"二次裁剪失败，请手动处理" };
			throw ex;
		}
		/*Mat ccolor;
		for (int i = 0; i < coll.size(); i++) {
			cvtColor(img, ccolor, CV_GRAY2BGR);
			draw(line, ccolor, CvPoint(0, coll[i].start), CvPoint(img.cols, coll[i].start), CvScalar(0, 0, 255));
			draw(line, ccolor, CvPoint(0, coll[i].start + coll[i].length), CvPoint(img.cols, coll[i].start + coll[i].length), CvScalar(255, 0, 0));
		}
		imdebug("2", ccolor);*/
	}
}

void splitter::interCheck(vector<int> &f) {
	//O(n^2)
	unsigned *pool = new unsigned[collection.size()];
	size_t n;
	for (unsigned i = 0; i < (n = collection.size()); i++) {
		unsigned k = 0;
		for (unsigned j = 0; j < n; j++) if(i - j) pool[k++] = collection[j].length;

		unsigned min = INT_MAX, max = 0, mindist = INT_MAX;
		for (unsigned j = 0; j < k; j++) {
			min = pool[j] < min ? pool[j] : min;
			max = pool[j] > max ? pool[j] : max;
			unsigned tmp = pool[j] < collection[i].length ? collection[i].length - pool[j] : pool[j] - collection[i].length;
			mindist = tmp < mindist ? tmp : mindist;
		}

		if (mindist > max - min) {
			collection.erase(collection.begin() + i);
			f.emplace_back(i);
		}
	}
	delete[] pool;
}

splitter::splitter(Mat img) {
	this->org = img;
}

void splitter::start(vector<Mat>& piece) {
	try { split(); }
	catch (err ex) {
		switch (ex.id)
		{
		case 4:
			throw ex;
		default:
			break;
		}
	}
	size_t n = collection.size();
	vector<int> t;
	vector<bool> r(n, false);
	vector<space> toCut;

	interCheck(t);
	for (size_t i = 0; i < n; i++) r[i] = false;
	interCheck(t);
	n = collection.size();

	vector<bool> classifier(n, false);
	for (int i : t) r[i] = true;
	KClassify(classifier);
	for (size_t k = 0, i = 0; i < n; i++) {
		if (!r[i]) {
			r[i] = classifier[k++];
		}
	}
	n = collection.size();

	Mat ccolor;
	Showline cvtColor(org, ccolor, CV_GRAY2BGR);

	for (int i = 0; i < n; i++) {
		if (r[i]) {

			draw(line, ccolor, CvPoint(0, collection[i].start), CvPoint(org.cols, collection[i].start), CvScalar(0, 0, 255));
			draw(line, ccolor, CvPoint(0, collection[i].start + collection[i].length), CvPoint(org.cols, collection[i].start + collection[i].length), CvScalar(0, 0, 255));

			toCut.emplace_back(collection[i]);
		}
	}
	Showline imdebug("2", ccolor);

	if (toCut.size() > 2) collection.shrink_to_fit();
	else toCut.swap(collection);

	n = toCut.size();
	piece.clear();
	piece.resize(n+1);

	org(Range(0, toCut[0].start), Range(0, org.cols)).copyTo(piece[0]);
	for (size_t i = 1; i < n; i++) {
		org(Range(toCut[i - 1].start + toCut[i - 1].length + 1, toCut[i].start),
			Range(0, org.cols)).copyTo(piece[i]);
	}
	org(Range(toCut[n - 1].start + toCut[n - 1].length + 1, org.rows), Range(0, org.cols)).copyTo(piece[n]);

	for (Mat& i: piece) i = trim(i);				//每个分割出来的行再剪去空白
}