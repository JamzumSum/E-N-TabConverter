#include "pch.h"
#include "Cuckoo.h"
#include "Dodo.h"
#include "../E-N TabConverter/global.h"

using namespace cv;

#if _DEBUG
#define Showdenoise if(0)
#else
#define Showdenoise /##/
#endif

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
	auto fullfill = [](Mat & mask) {
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

void LineFinder::findRow(vector<Vec4i> & lines) {
	/*
	* @brief			概率霍夫变换提取横线
	* @param[in]		lines，筛选后的结果
	*/

	//Mat dilated, eroded;
	//Mat hline = getStructuringElement(MORPH_RECT, Size(max(org.cols / 120, 1),1));						//竖直结构
	//erode(org, eroded, hline);																			//腐蚀
	//dilate(eroded, dilated, hline);																		//膨胀
	//Mat inv = 255 - dilated;
	HoughLinesP(255 - org, lines, 1, CV_PI / 180, 100, org.cols * 2 / 3, org.cols / 50);
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
	for (int i : thickness) getkey(lineThickness) += i;
	upper = std::min(lines[5][1], lines[5][3]);
	lower = std::max(lines[0][1], lines[0][3]);
#if 0
	Mat color;
	cvtColor(org, color, CV_GRAY2BGR);
	for (Vec4i& i : lines) {
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
	Mat toScan = org(Range(lower, upper), Range::all()).clone();
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
		if (lines[i][0] - lines[i - 1][0] >= max(16, getkey(colLenth) / 5)) {
			getkey(colLenth) += lines[i][0] - lines[i - 1][0];
		}
	}
#if 0
	Mat ccolor;
	cvtColor(org, ccolor, CV_GRAY2BGR);
	for (Vec4i& i : lines) {
		line(ccolor, Point(i[0], 0), Point(i[2], org.rows), Scalar(0, 0, 255));
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
		if (!org.at<uchar>(y(i), i)) {
			sum++;
		}
	}
	if (sum < 0.7 * (line[2] - line[0])) {
		return true;
	}
	return false;
}