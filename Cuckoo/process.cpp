#include "pch.h"
#include "Cuckoo.h"
#include "Dodo.h"
#include <atomic>
#include "../E-N TabConverter/global.h"

using namespace cv;

#if _DEBUG
#define Showdenoise if(0)
#else
#define Showdenoise /##/
#endif

template<class cls>
vector<vector<cls>> classifyContinuous(const vector<cls>& arr, function<const int(cls)> getInt);

void Splitter::start(vector<Mat>& piece) {
	auto hMORPH = [this](Mat& r, short threadNum) {
		//terriblly slow... so multi-thread...
		atomic_int cnt = threadNum;
		int y = 0;
		int step = org.rows / threadNum;
		Mat hkernel = getStructuringElement(MORPH_RECT, Size(max(org.cols / 2, 1), 1));
		auto forthread = [&](const int y, const int len) {
			Rect roi(0, y, org.cols, len);
			morphologyEx(255 - org(roi), r(roi), MORPH_CLOSE, hkernel);
			cnt--;
		};
		for (short i = 1; i < threadNum; i++) {
			thread t(forthread, y, step);
			t.detach();
			y += step;
		}
		thread t(forthread, y, org.rows - y);
		t.join();
		while (cnt > 0) this_thread::yield();
	};
	
	Mat r(org.rows, org.cols, org.type());
	hMORPH(r, 4);
	r = Morphology(r, org.cols / 100, false, true);
	Mat ccolor;
	cvtColor(org, ccolor, CV_GRAY2BGR);
	vector<vector<Point>> cont;
	findContours(r, cont, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_NONE);

	size_t n = cont.size();
	vector<Rect> region(n);
	piece.resize(n);

	std::transform(cont.begin(), cont.end(), region.begin(), [](vector<Point> x) -> Rect {return boundingRect(x); });
	std::sort(region.begin(), region.end(), [](const Rect x, const Rect y) -> bool {return x.y < y.y; });

	std::transform(region.begin(), region.end(), piece.begin(), [this](Rect x) -> Mat {return org(x).clone(); });

	remove_if(piece.begin(), piece.end(), [](Mat & x) -> bool {return x.empty(); });
	piece.shrink_to_fit();
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


/*
	* @brief			概率霍夫变换提取横线
	* @param[in]		lines，筛选后的结果
	*/
void LineFinder::findRow(vector<Vec4i> & lines) {
	HoughLinesP(255 - org, lines, 1, CV_PI / 180, 100, org.cols * 2 / 3, org.cols / 50);


	//角度筛选
	double t = tan(range);
	remove_if(lines.begin(), lines.end(), [t, this](const Vec4i x) -> bool {
		return x[2] == x[0] 
			|| abs(((double)x[3] - x[1]) / (x[2] - x[0])) > t
			|| isDotLine(x);
	});

	//sort
	if (!lines.size()) return;
	sort(lines.begin(), lines.end(), [](const Vec4i x, const Vec4i y) ->bool {
		return x[1] < y[1];
	});


	//去除连续
	auto rcls = classifyContinuous<Vec4i>(lines, [](const Vec4i x) -> int {return x[1]; });
	vector<Vec4i> copy(lines);
	thickness.resize(rcls.size());
	lines.resize(rcls.size());
	std::transform(rcls.begin(), rcls.end(), thickness.begin(), [](const vector<Vec4i> x) -> int { int i = (int)x.size();  getkey(lineThickness) += i; return int(i); });
	std::transform(rcls.begin(), rcls.end(), lines.begin(), [](const vector<Vec4i> x) -> Vec4i {return x[x.size() / 2]; });

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


/*
	 * 函数：findCol
	 * @brief				概率霍夫变换提取竖线
	 * param[in]			lines，筛选后的结果
	*/
void LineFinder::findCol(vector<Vec4i> & lines) {
	int length = upper - lower;
	int thick = max(1, *max_element(thickness.begin(), thickness.end()));
	Mat toScan = org(Range(lower, upper), Range::all()).clone();
	toScan = Morphology(toScan, max(thick, 3), false, false);
	Mat inv = 255 - Morphology(toScan, max(toScan.rows / 2, 1), false, true);


	//80:140
	HoughLinesP(inv, lines, 1, CV_PI / 180, (toScan.rows - 2 * thick) / 2, toScan.rows - 2 * thick, thick + 2);

	//range filter
	double t = tan(CV_PI / 2.0 - range);
	remove_if(lines.begin(), lines.end(), [t](const Vec4i x) -> bool {
		return x[0] == x[2] || abs(((double)x[3] - x[1]) / (x[2] - x[0])) < t; 
	});

	//sort it
	sort(lines.begin(), lines.end(), [](const Vec4i x, const Vec4i y) ->bool {
		return x[0] < y[0];
	});


	//去除连续
	auto rcls = classifyContinuous<Vec4i>(lines, [](const Vec4i x) ->int {return x[0]; });
	lines.resize(rcls.size());
	std::transform(rcls.begin(), rcls.end(), lines.begin(), [](const vector<Vec4i> x) -> Vec4i {return x[x.size() / 2]; });
	lines.shrink_to_fit();


	for (size_t i = 1; i < lines.size(); i++) {
		if (lines[i][0] - lines[i - 1][0] >= max(16, getkey(colLenth) / 5)) {
			getkey(colLenth) += lines[i][0] - lines[i - 1][0];
		}
	}

#if 1
	Mat ccolor;
	cvtColor(org, ccolor, CV_GRAY2BGR);
	for (Vec4i& i : lines) {
		line(ccolor, Point(i[0], 0), Point(i[2], org.rows), Scalar(0, 0, 255));
	}
	imdebug("Cuckoo Debug: Show cols", ccolor);
#endif
}


bool LineFinder::isDotLine(cv::Vec4i line) {
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

template<class cls>
vector<vector<cls>> classifyContinuous(const vector<cls>& arr, function<const int(cls)> getInt) {
	size_t n = arr.size();
	if (n < 1) return vector<vector<cls>>();

	int t, t1 = getInt(arr[0]);
	vector<vector<cls>> r{ {arr[0]} };

	for (size_t i = 1; i < n; i++) {
		t = t1;
		t1 = getInt(arr[i]);
		if (t + 1 == t1) r.back().emplace_back(arr[i]);
		else r.emplace_back(vector<cls>{ arr[i] });
	}

	return r;
}