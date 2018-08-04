#include "myheader.h"
#include "global.h"
using namespace std;

#define savepic 0
#define picFolder "C:\\Users\\Administrator\\Desktop\\oh"
#if _DEBUG
#define showRectangle 1
#endif

int count(cv::Mat img, cv::Vec4i range, int delta);

inline void measure::recNum(cv::Mat section, std::vector<cv::Vec4i> rows) {
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
#if showRectangle
	cv::Mat ccolor;
	cvtColor(section, ccolor, CV_GRAY2BGR);
#endif
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
			#if showRectangle
				rectangle(ccolor, cv::Point(tmp[0], tmp[1]), cv::Point(tmp[2], tmp[3]), cv::Scalar(0, 0, 255));
			#endif
			newNote.pos = (tmp[0] + tmp[2]) / 2;
			
			global->characterWidth += tmp[2] - tmp[0];
			
			this->maxCharacterWidth = max(maxCharacterWidth, tmp[2] - tmp[0]);
			this->noteBottom = max(noteBottom, tmp[3]);
			this->notes.push_back(newNote);
		}
	}
#if showRectangle
	imshow("2", ccolor); cvWaitKey();
#endif
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
				err ex = {0,__LINE__,"fret合并：不可置信条件. 检查模型" };
				throw ex;
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
		if (notes[i].pos - notes[i - 1].pos <= t / 2) {
			notes[i].chord = true;
		}
	}
	//imshow("2", ccolor); cvWaitKey();
}
inline void measure::recTime(std::vector<cv::Vec4i> rows) {
	int predLen = 0;
	cv::Mat picValue = org(cv::Range(max(noteBottom, rows[5][1]) + 1, org.rows), cv::Range::all()).clone();
	cv::Mat inv;
	std::vector<std::vector<cv::Point>> cont;
	if (org.rows < global->rowLenth * 2 && org.rows > global->rowLenth / 2) {
		inv = 255 - Morphology(picValue, picValue.rows / 3, false, true);
		cv::findContours(inv, cont, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_NONE);

		//imshow("2", picValue); cvWaitKey();
		cv::Vec2i temp = { picValue.rows,0 };
		int tr = 4;
		while (!cont.size()) {
			cv::Mat inv = 255 - Morphology(picValue, picValue.rows / tr++, false, true);
			cv::findContours(inv, cont, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_NONE);
			if (picValue.rows / tr < 2) {
				for (int i = 0; i < picValue.rows; i++) {
					if (!isEmptyLine(picValue, i, 0)) {
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
	inv = 255 - Morphology(picValue, predLen / 3, false, true);
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
		sum1 = count(picValue, tmp, -2);
		sum2 = count(picValue, tmp, 2);
		sum3 = count(picValue, tmp, 1);
		if (tmp[3] - tmp[1] <= predLen / 2 && !sum1 && !sum2) {
			sum1--; sum2--;
		}
		time.push_back((int)(this->time.beat_type * pow(2, max(sum1, !sum3 && sum2 ? sum2 - 1 : sum2)) * (!sum3 && sum2 ? 1.5 : 1)));
	}
	for (int i : time) {
		if (i != quarter) {
			goto distribute;
		}
	}
	for (int &i : time) {
		i = half;
	}
distribute:
	for (int k = 0, i = 0; i < notes.size(); i++) {
		if (notes[i].chord) {
			notes[i].timeValue = notes[i - 1].timeValue;
		}
		else {
			if (k == time.size()) {
				err ex = { 1,__LINE__,"time 越界，自动跳出，建议检查该小节 notes 的 chord 划分" };
				throw ex;
			}
			notes[i].timeValue = (Value)time[k++];
		}
	}
}
inline measure::measure(cv::Mat org, cv::Mat img, vector<cv::Vec4i> rows,int id) {
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
			#if _DEBUG
				imshow("2", org); cvWaitKey();
			#endif
			break;
		default:
			throw ex;
			break;
		}
	}
	
	
}

int count(cv::Mat img,cv::Vec4i range,int delta) {
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