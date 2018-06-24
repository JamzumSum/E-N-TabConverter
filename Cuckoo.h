#include"myheader.h"
#include "global.h"
using namespace std;

int count(cv::Mat img, cv::Vec4i range, int delta);
int valueSignalLen;

inline void measure::recNum(cv::Mat section, std::vector<cv::Vec4i> rows) {
	/*
	* 函数：Dsection::recNum
	* 功能：从传入图像中提取数字等
	* 参数：
	section，Mat，传入图像
	rows，Vec4i，传入的网格信息（谱线）
	*/
	//imshow("2", section); cvWaitKey();
	std::vector<std::vector<cv::Point>> cont;
	cv::Mat inv = 255 - section;
	//cvtColor(section, ccolor, CV_GRAY2BGR);
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
			&& tmp[3] - tmp[1] > tmp[2] - tmp[0]						//形状限定
			&& (tmp[3] - tmp[1])*(tmp[2] - tmp[0]) > 9)					//大小限定
		{
			note newNote;
			cv::Mat number = section(cv::Range(tmp[1], tmp[3] + 1), cv::Range(tmp[0], tmp[2] + 1)).clone();
			//rectangle(ccolor,cv::Point(tmp[0],tmp[1]),cv::Point(tmp[2],tmp[3]),cv::Scalar(0,0,255));
			cvtColor(number, number, CV_GRAY2BGR);
			if (number.cols != 8 || number.rows != 10) number = perspect(number, 8, 10);

			//savePic("C:\\Users\\Administrator\\Desktop\\oh", number);

			newNote.notation.technical.string = whichLine(tmp, rows);										//几何关系判断string
			if (!newNote.notation.technical.string) continue;
			newNote.notation.technical.fret = rec(number, newNote.possible);								//识别数字fret
			if (!SUCCEED(newNote.notation.technical.fret)) {
				//TODO: 识别错误
				return;
			}
			newNote.pos = (tmp[0] + tmp[2]) / 2;
			this->maxCharacterWidth = max(maxCharacterWidth, tmp[2] - tmp[0]);
			this->noteBottom = max(noteBottom, tmp[3]);
			this->notes.push_back(newNote);
		}
	}
	
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
				cerr << "fret合并：不可置信条件. 检查模型" << endl;
			}
			m->pos = (m->pos + n->pos) / 2;
			m->notation.technical.fret = 10 + n->notation.technical.fret;
			notes.erase(n);
		}
	}

	sort(this->notes.begin(), this->notes.end(), [](const note x, const note y) -> bool {
		return x.pos < y.pos || (x.pos == y.pos && x.notation.technical.string < y.notation.technical.string);
	});
	for (size_t i = 1; i < notes.size(); i++) {
		if (notes[i].pos - notes[i - 1].pos <= 1) {
			notes[i].chord = true;
		}
	}
	//imshow("2", ccolor); cvWaitKey();
}
inline measure::measure(cv::Mat org, cv::Mat img, vector<cv::Vec4i> rows,int id) {
	this->id = id;
	if (org.cols < colLenth / 5) {
		this->id = -1;
		return;
	}
	int predLen = 0;
	recNum(img, rows);
	cv::Mat picValue = org(cv::Range(max(noteBottom, rows[5][1]) + 1, img.rows), cv::Range::all()).clone();
	cv::Mat inv;
	std::vector<std::vector<cv::Point>> cont;
	if (org.rows < rowLenth * 2 && org.rows > rowLenth / 2) {
		inv = 255 - Morphology(picValue, picValue.rows / 3, false, true);
		cv::findContours(inv, cont, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_NONE);

		//imshow("2", picValue); cvWaitKey();
		cv::Vec2i temp = { picValue.rows,0 };
		int tr = 4;
		while (!cont.size()) {
			cv::Mat inv = 255 - Morphology(picValue, picValue.rows / tr++, false, true);
			cv::findContours(inv, cont, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_NONE);
			if (picValue.rows / tr < 2) {
				imshow("2", org); cvWaitKey();
				return;
			}
		}

		for (int i = 0; i < cont.size(); i++) {
			for (int j = 0; j < cont[i].size(); j++) {
				temp[0] = std::min(temp[0], cont[i][j].y);
				temp[1] = std::max(temp[1], cont[i][j].y);
			}
			predLen = std::max(predLen, temp[1] - temp[0]);
		}
		valueSignalLen = ((valueSignalLen ? valueSignalLen : predLen) + predLen) / 2;
	}
	else {
		predLen = valueSignalLen;
	}
	inv = 255 - Morphology(picValue, predLen / 3, false, true);
	cv::findContours(inv, cont, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_NONE);
	//一个规模庞大。。。倾我毕生所学的筛选算法，去掉节拍记号再往下的乱七八糟的东西
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
	vector<int> time;
	for (int i = 0; i < cont.size(); i++) {
		cv::Vec4i tmp = { picValue.cols,picValue.rows,0,0 };
		for (int j = 0; j < cont[i].size(); j++) {
			tmp[0] = std::min(tmp[0], cont[i][j].x);
			tmp[2] = std::max(tmp[2], cont[i][j].x);
			tmp[1] = std::min(tmp[1], cont[i][j].y);
			tmp[3] = std::max(tmp[3], cont[i][j].y);
		}
		int sum1 = 0, sum2 = 0, sum3 = 0;
		
		/*bool lock1 = false, lock2 = false, lock3 = false;
		for (int y = (tmp[1] + tmp[3]) / 2; y < tmp[3]; y++)
		{
			uchar *ptr1 = picValue.ptr<uchar>(y);
			uchar *ptr2 = picValue.ptr<uchar>(y + 1);

			if (ptr1[tmp[0] - 2] && !ptr2[tmp[0] - 2]) {
				if (!lock1) {
					lock1 = true;
					y++;
				}
			}
			else if (!ptr1[tmp[0] - 2] && ptr2[tmp[0] - 2]) {
				if (lock1) {
					sum1++;
					lock1 = false;
				}
			}
		}
		for (int y = tmp[1]; y < tmp[3]; y++)
		{
			uchar *ptr1 = picValue.ptr<uchar>(y);
			uchar *ptr2 = picValue.ptr<uchar>(y + 1);

			if (ptr1[tmp[2] + 2] && !ptr2[tmp[2] + 2]) {
				if (!lock2) {
					lock2 = true;
					y++;
				}
			}
			else if (!ptr1[tmp[2] + 2] && ptr2[tmp[2] + 2]) {
				if (lock2) {
					sum2++;
					lock2 = false;
				}
			}
		}
		for (int y = tmp[1]; y < tmp[3]; y++)
		{
			uchar *ptr1 = picValue.ptr<uchar>(y);
			uchar *ptr2 = picValue.ptr<uchar>(y + 1);

			if (ptr1[tmp[2] + 1] && !ptr2[tmp[2] + 1]) {
				if (!lock3) {
					lock3 = true;
					y++;
				}
			}
			else if (!ptr1[tmp[2] + 1] && ptr2[tmp[2] + 1]) {
				if (lock3) {
					sum3++;
					lock3 = false;
				}
			}
		}
		if (lock1) sum1++;
		if (lock2) sum2++;
		if (lock3) sum3++;*/
		sum1 = count(picValue, tmp, -2);
		sum2 = count(picValue, tmp, 2);
		sum3 = count(picValue, tmp, 1);
		if (tmp[3] - tmp[1] <= predLen / 2 && !sum1 && !sum2) {
			sum1--; sum2--;
		}
		time.push_back((int)(this->time.beat_type * pow(2, max(sum1, !sum3 && sum2 ? sum2 - 1 : sum2)) * (!sum3 && sum2 ? 1.5 : 1)));
	}
	for (int k = 0, i = 0; i < notes.size(); i++) {
		if (notes[i].chord) {
			notes[i].timeValue = notes[i - 1].timeValue;
		}
		else {
			if (k == time.size()) {
#if _DEBUG
				imshow("2", org); cvWaitKey();
				cerr << "time 越界，自动跳出，建议检查该小节 notes 的 chord 划分" << endl;
#endif
				break;
			}
			notes[i].timeValue = (Value)time[k++];
		}
	}
	
}

int count(cv::Mat img,cv::Vec4i range,int delta) {
	bool lock = false;
	int sum = 0, x = range[delta > 0 ? 2 : 0] + delta;
	int blocksize = 2 * (int)max(1.0, round(col / 1000.0)) + 1;
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