#include "Cuckoo.h"
#include "Dodo.h"
#include "global.h"
#include "imgproc.hpp"
#include "opencv.hpp"

#define optimize 1

#if _DEBUG
#define Showrectangle 0
#define Showline 0
#define draw(func, img, from, to, color) if(Show##func) func(img, from, to, color)
#define imdebug(img, title) imshow((img), title); cvWaitKey()
#else 
#define draw(func, img, from, to, color)
#define imdebug(img, title)
#endif

static int count(cv::Mat img, cv::Vec4i range, int delta) {
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

void measure::recNum(cv::Mat section, std::vector<cv::Vec4i> rows) {
	/*
	* 函数：measure::recNum
	* 功能：从传入图像中提取数字等
	* 参数：
		section，Mat，传入图像
		rows，Vec4i，传入的网格信息（谱线）
	*/
	//imshow("2", section); cvWaitKey();
	std::vector<std::vector<cv::Point>> cont;
	cv::Mat inv = 255 - section;

	cv::Mat ccolor;
	if (Showrectangle) cvtColor(section, ccolor, CV_GRAY2BGR);

	cv::findContours(inv, cont, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_NONE);
	for (int q = 0; q < cont.size(); q++) {
		cv::Vec4i tmp = { section.cols,section.rows,0,0 };
		for (int k = 0; k < cont[q].size(); k++) {
			tmp[0] = std::min(tmp[0], cont[q][k].x);
			tmp[2] = std::max(tmp[2], cont[q][k].x);
			tmp[1] = std::min(tmp[1], cont[q][k].y);
			tmp[3] = std::max(tmp[3], cont[q][k].y);
		}
		//限定筛选
		if (tmp[3] - tmp[1] < rows[1][1] - rows[0][1]					//网格限定
			&& tmp[3] - tmp[1] < 5 * (tmp[2] - tmp[0])
			&& tmp[3] - tmp[1] > tmp[2] - tmp[0]						//形状限定
			&& (global->characterWidth ? (tmp[2] - tmp[0] > global->characterWidth / 2) : 1)
			&& (tmp[3] - tmp[1])*(tmp[2] - tmp[0]) > 9)					//大小限定
		{
			note newNote;
			cv::Mat number = section(cv::Range(tmp[1], tmp[3] + 1), cv::Range(tmp[0], tmp[2] + 1)).clone();
			int sum = 0;
			for (int y = 0; y < number.rows; y++) {
				uchar *ptr = number.ptr<uchar>(y);
				for (int w = 0; w < number.cols; w++) {
					if (!ptr[w]) sum++;
				}
			}
			if (sum > 0.8 * number.rows * number.cols) continue;
			cvtColor(number, number, CV_GRAY2BGR);
			if (number.cols != 8 || number.rows != 10) number = perspect(number, 8, 10);
#if savepic
			savePic(picFolder, number);
#endif

			newNote.notation.technical.string = whichLine(tmp, rows);										//几何关系判断string
			if (!newNote.notation.technical.string) continue;
			newNote.notation.technical.fret = rec(number, newNote.possible);								//识别数字fret
			if (!SUCCEED(newNote.notation.technical.fret)) {
				//TODO: 识别错误
				return;
			}

			draw(rectangle, ccolor, cv::Point(tmp[0], tmp[1]), cv::Point(tmp[2], tmp[3]), cv::Scalar(0, 0, 255));


			newNote.pos = (tmp[0] + tmp[2]) / 2;

			global->characterWidth += tmp[2] - tmp[0];

			this->maxCharacterWidth = max(maxCharacterWidth, tmp[2] - tmp[0]);
			this->noteBottom = max(noteBottom, tmp[3]);
			this->notes.push_back(newNote);
		}
	}
	if (Showrectangle) imdebug("2", ccolor);

	int t = maxCharacterWidth;
	auto n = notes.end();
	auto m = notes.end();
	while (1) {
		//合并相邻的fret version2
	mfind:
		m = find_if(notes.begin(), notes.end(), [this, t, &n](const note x) ->bool {
			n = find_if(notes.begin(), notes.end(), [x, t](const note y) ->bool {
				return x.pos != y.pos
					&& y.notation.technical.string == x.notation.technical.string
					&& abs(x.pos - y.pos) <= 2 * t;
			});
			return n != notes.end();
		});
		if (m == notes.end()) break;
		else {
			if (m->pos > n->pos) swap(m, n);
			if (m->notation.technical.fret > 1) {
				for (int i : m->possible) {
					if (i < 2) {
						m->pos = (m->pos + n->pos) / 2;
						m->notation.technical.fret = 10 * i + n->notation.technical.fret;
						notes.erase(n);
						goto mfind;
					}
				}
				err ex = { 0,__LINE__,"fret合并：不可置信条件. 检查模型" };
				throw ex;
			}
			m->pos = (m->pos + n->pos) / 2;
			m->notation.technical.fret = 10 + n->notation.technical.fret;
			notes.erase(n);
		}
	}

	std::sort(this->notes.begin(), this->notes.end(), [](const note x, const note y) -> bool {
		return x.pos < y.pos || (x.pos == y.pos && x.notation.technical.string < y.notation.technical.string);
	});
	for (size_t i = 1; i < notes.size(); i++) {
		if (notes[i].pos - notes[i - 1].pos <= t / 2) {
			notes[i].chord = true;
		}
	}
	//imshow("2", ccolor); cvWaitKey();
}

void measure::recTime(std::vector<cv::Vec4i> rows) {
	int predLen = 0;
	cv::Mat picValue = org(cv::Range(max(noteBottom, rows[5][1]) + 1, org.rows), cv::Range::all()).clone();
	cv::Mat inv;
	std::vector<std::vector<cv::Point>> cont;
	if (org.rows < global->rowLenth * 2 && org.rows > global->rowLenth / 2) {
		inv = 255 - Morphology(picValue, picValue.rows / 3, false, true);
		cv::findContours(inv, cont, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_NONE);

		//imshow("2", picValue); cvWaitKey();
		cv::Vec2i temp = { picValue.rows,0 };
		int tr = picValue.rows / 4;
		while (!cont.size()) {
			cv::Mat inv = 255 - Morphology(picValue, tr--, false, true);
			cv::findContours(inv, cont, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_NONE);
			if (tr < 2) {
				for (int i = 0; i < picValue.rows; i++) {
					if (!isEmptyLine(picValue, i, 0.04)) {
						err ex = { 6,__LINE__,"未扫描到纵向结构" };
						throw ex;
					}
				}
				return;
			}
		}

		for (int i = 0; i < cont.size(); i++) {
			for (int j = 0; j < cont[i].size(); j++) {
				temp[0] = std::min(temp[0], cont[i][j].y);
				temp[1] = std::max(temp[1], cont[i][j].y);
			}
			predLen = max(predLen, temp[1] - temp[0]);
		}
		global->valueSignalLen += predLen;
	}
	else {
		predLen = global->valueSignalLen;
	}
	inv = 255 - Morphology(picValue, round(predLen * 0.3), false, true);
	cv::findContours(inv, cont, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_NONE);

	//去掉节拍记号再往下的乱七八糟的东西
	auto m = cont.end();
	while (1) {
		m = find_if(cont.begin(), cont.end(), [cont](vector<cv::Point> x) ->bool {
			auto l = [](cv::Point m, cv::Point n) ->bool {
				return m.y < n.y;
			};
			int up = min_element(x.begin(), x.end(), l)->y;
			int len = max_element(x.begin(), x.end(), l)->y - up;
			return find_if(cont.begin(), cont.end(), [cont, x, l, up, len](vector<cv::Point> y) ->bool {
				int max = max_element(y.begin(), y.end(), l)->y;
				return max < up
					&& max - min_element(y.begin(), y.end(), l)->y > len;
			}) != cont.end();
		});
		if (m == cont.end()) break;
		else cont.erase(m);
	};

	sort(cont.begin(), cont.end(), [](vector<cv::Point> x, vector<cv::Point> y) ->bool {
		return x[0].x < y[0].x;
	});

	vector<int> timeValue;
	vector<int> timePos;
	for (int i = 0; i < cont.size(); i++) {
		cv::Vec4i tmp = { picValue.cols,picValue.rows,0,0 };
		for (int j = 0; j < cont[i].size(); j++) {
			tmp[0] = std::min(tmp[0], cont[i][j].x);
			tmp[2] = std::max(tmp[2], cont[i][j].x);
			tmp[1] = std::min(tmp[1], cont[i][j].y);
			tmp[3] = std::max(tmp[3], cont[i][j].y);
		}

		int sum1 = 0, sum2 = 0, sum3 = 0;
		sum1 = count(picValue, tmp, -2);
		sum2 = count(picValue, tmp, 2);
		sum3 = count(picValue, tmp, 1);
		if (tmp[3] - tmp[1] <= predLen / 2 && !sum1 && !sum2) {
			sum1--; sum2--;
		}
		timeValue.push_back((int)(this->time.beat_type * pow(2, max(sum1, !sum3 && sum2 ? sum2 - 1 : sum2)) * (!sum3 && sum2 ? 1.5 : 1)));
		timePos.push_back((tmp[0] + tmp[2]) / 2);
	}
	for (int i : timeValue) {
		if (i != quarter) {
			goto distribute;
		}
	}
	for (int &i : timeValue) {
		i = half;
	}
distribute:
	/*for (int k = 0, i = 0; i < notes.size(); i++) {
		if (notes[i].chord) {
			notes[i].timeValue = notes[i - 1].timeValue;
		}
		else {
			if (k == timeValue.size()) {
				err ex = { 1,__LINE__,"time 越界，自动跳出，建议检查该小节 notes 的 chord 划分" };
				throw ex;
			}
			notes[i].timeValue = (Value)timeValue[k++];
		}
	}*/
	vector<note*> noValue;
	for (note &i : notes) {
		noValue.push_back(&i);
	}
	for (int i = 0; i < timeValue.size(); i++) {
		int pos = timePos[i];
		int t = maxCharacterWidth;
		for (;;) {
			auto s = find_if(noValue.begin(), noValue.end(), [pos, t](note* x)->bool {
				return abs(x->pos - pos) < t;
			});
			if (s == noValue.end()) break;
			(*s)->timeValue = (Value)timeValue[i];
			noValue.erase(s);
		}
	}
	if (noValue.size()) {
		err ex = { 1,__LINE__,"有未分配时值的乐符" };
		throw ex;
	}
}

measure::measure(cv::Mat org, cv::Mat img, vector<cv::Vec4i> rows, int id) {
	this->id = id;
	this->org = org;
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
		case 0:
			//包含不合理数据
			break;
		case 6:
			//没有竖直结构用以判断时值
			imdebug("2", org); cvWaitKey();
			break;
		default:
			throw ex;
			break;
		}
	}
}

void cutter::KClassify(vector<bool> &classifier) {
	//函数名：KClassify
	//功能：分类对象是空白区域 struct _space，分类依据为space.lenth
	//输出：classifier 算法将各区域分入两类，一类为false，一类为true

	//k1 15:1800    k2 5:1800  （原始数据 此行无效）
	assert(collection.size() == classifier.size());
	int k1 = 18, k2 = 5;
	int min = collection[0].length, max = min;
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

int cutter::split() {
	int r = 1;
	bool flag = false;
	for (int st = org.rows, i = 0; i < org.rows; i++) {
		//初步设为0.04 以后再说
		if (isEmptyLine(org, i, 0.004)) {
			if (!flag) {
				st = i;
				flag = true;
			}
		}
		else {
			if (flag) {
				if (i - 1 - st > 0) {
					collection.push_back({ st,i - 1 - st });
				}
				flag = false;
			}
		}
	}
	if (collection.size() < 2) {
		collection.clear();
		r = 2;
		//cout << "裁剪失败，等待二次裁剪" << endl;
		//二次裁剪为缩减判断空行的范围，从之前的从像素x=0 至x=col到检测到的横线的x1至x2
		vector<cv::Vec4i> rows;
		vector<int> thick;
		cv::Mat toOCR;
		findRow(org, toOCR, CV_PI / 18, rows, thick);
		if (rows.size() > 5) {
			//cout << "二次裁剪开始." << endl;
			flag = false;
			for (int st = org.rows, i = min(rows[0][1], rows[0][3]); i < org.rows; i++) {
				if (isEmptyLine(org, i, max(min(rows[0][0], rows[0][2]), min(rows[5][0], rows[5][2])),
					min(max(rows[0][0], rows[0][2]), max(rows[5][0], rows[5][2])), 0.01)) {
					if (!flag) {
						st = i;
						flag = true;
					}
				}
				else {
					if (flag) {
						if (i - 1 - st > 0) collection.push_back({ st,i - 1 - st });
						flag = false;
					}
				}
			}

		}
		if (collection.size() < 2) {
			err ex = { 4,__LINE__,"二次裁剪失败，请手动处理" };

			return 3;
		}
		/*cv::Mat ccolor;
		for (int i = 0; i < coll.size(); i++) {
			cvtColor(img, ccolor, CV_GRAY2BGR);
			line(ccolor, CvPoint(0, coll[i].start), CvPoint(img.cols, coll[i].start), CvScalar(0, 0, 255));
			line(ccolor, CvPoint(0, coll[i].start + coll[i].length), CvPoint(img.cols, coll[i].start + coll[i].length), CvScalar(255, 0, 0));
		}
		imshow("2", ccolor); cvWaitKey();*/
	}
	return r;
}

void cutter::interCheck(vector<int> &f) {
#if optimize
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
			unsigned tmp = pool[j] < (unsigned)collection[i].length ? collection[i].length - pool[j] : pool[j] - collection[i].length;
			mindist = tmp < mindist ? tmp : mindist;
		}

		if (mindist > max - min) {
			collection.erase(collection.begin() + i);
			f.push_back(i);
		}
	}
	delete[] pool;
#else
	//O(what???)
	size_t n = collection.size();

	if (n <= 1) return;
	f.clear();
	int **pool = new int*[n];
	//初始化截止
	for (int i = 0; i < n; i++) pool[i] = new int[n]();
	for (int i = 0; i < n; i++) 
		for (int j = 0; j < n; j++) 
			pool[i][j] = abs(collection[i].length - collection[j].length);
	//初值设置完毕

	//校验开始
	for (int i = 0; i < n; i++) {
		int min = _CRT_INT_MAX, max = 0;
		for (int j = 0; j < n; j++) {
			if (j == i) continue;
			if (min > pool[i][j]) {
				min = pool[i][j];
			}
			for (int q = 0; q < n; q++) {
				if (q == i) continue;
				if (max < pool[q][j]) {
					max = pool[q][j];
				}
			}
		}
		if (max < min) {
			collection.erase(collection.begin() + i);
			for (int k = 0; k < n; k++) delete[] pool[k];
			delete[] pool;
			f.push_back(i);
			interCheck(f);
			return;
		}
	}
	for (int k = 0; k < n; k++) delete[] pool[k];
	delete[] pool;
	return;
#endif
}

cutter::cutter(cv::Mat img) {
	this->org = img;
}

void cutter::start(vector<cv::Mat>& piece) {
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

	cv::Mat ccolor;
	if (Showline) cvtColor(org, ccolor, CV_GRAY2BGR);

	for (int i = 0; i < n; i++) {
		if (r[i]) {

			draw(line, ccolor, CvPoint(0, collection[i].start), CvPoint(org.cols, collection[i].start), CvScalar(0, 0, 255));
			draw(line, ccolor, CvPoint(0, collection[i].start + collection[i].length), CvPoint(org.cols, collection[i].start + collection[i].length), CvScalar(0, 0, 255));

			toCut.push_back(collection[i]);
		}
	}
	if (Showline) imdebug("2", ccolor);
	piece.clear();
	piece.resize(n);

	if (toCut.size() > 2) collection.clear();
	else toCut.swap(collection);

	org(cv::Range(0, toCut[0].start), cv::Range(0, org.cols)).copyTo(piece[0]);
	for (size_t i = 1; i < n; i++) {
		org(cv::Range(toCut[i - 1].start + toCut[i - 1].length + 1, toCut[i].start),
			cv::Range(0, org.cols)).copyTo(piece[i]);
	}
	org(cv::Range(toCut[n - 1].start + toCut[n - 1].length + 1, org.rows), cv::Range(0, org.cols)).copyTo(piece[n]);

	for (cv::Mat& i: piece) i = trim(i);				//每个分割出来的行再剪去空白
}