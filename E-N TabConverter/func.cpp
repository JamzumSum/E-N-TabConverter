#include "stdafx.h"
#include "../Cuckoo/Dodo.h"
#include "eagle.h"
#include "global.h"
#include "../Cuckoo/Cuckoo.h"
#include "swan.h"
#include "tools.h"
#include "converter.h"

using namespace std;
using namespace cv;

GlobalPool global(cfgPath);
extern bool savepic;

void Converter::Train() {
	NumReader::train(defaultCSV);
}

int Converter::scan (function<void(string)> notify, function<void(int)> progress) {
	bool flag = false;
	string f = picPath[0];
	Mat img = imread(f.c_str(), 0);
	if (img.empty()) raiseErr("Wrong format.", 3);
	Mat trimmed = threshold(img);
	
	trimmed = trim(trimmed);
	auto dpi = getScaleFactor();
	auto screen = getWorkspaceSize();
	float screenCols = screen.first / dpi.first;
	
	if (trimmed.cols > screenCols) {
		float toRows = screenCols / trimmed.cols * trimmed.rows;
		trimmed = perspect(trimmed, int(screenCols), int(toRows));
		trimmed = threshold(trimmed);
	}
	global.setCol(trimmed.cols);
	vector<Mat> piece;
	
	Splitter piccut(trimmed);
	piccut.start(piece);
	
	progress(1);
	notify("过滤算法正常");
	
	int n = static_cast<int>(piece.size());
	vector<Mat> info;							//其余信息
	
	using general = tuple<vector<Vec4i>, vector<Vec4i>, Mat>;
	vector<general> sectionsGrid; sectionsGrid.reserve(n);

	atomic_int cnt = 0;
	auto scanGrid = [&cnt, &sectionsGrid, &n, &info](Mat i, int index) {
		vector<Vec4i> rows;
		LineFinder finder(i, 10);
		finder.findRow(rows);
		if (rows.size() == 6) {
			vector<Vec4i> lines;
			finder.findCol(lines);
			while (cnt < index - 1) this_thread::yield();
			sectionsGrid.emplace_back(rows, lines, i);
			n += static_cast<int>(lines.size());
			cnt++;
		}
		else {
			while (cnt < index - 1) this_thread::yield();
			info.push_back(i);
			cnt++;
		}
	};
	int tmpsize = static_cast<int>(n);
	n = 0;
	for (int k = 0; k < tmpsize; k++) {
		thread t(scanGrid, piece[k], k + 1);
		t.detach();						//race!
	}
	while (cnt < tmpsize) {
		this_thread::yield();
	}
	piece.clear(); piece.shrink_to_fit(); sectionsGrid.shrink_to_fit();
	progress(30);

	vector<measure> sections;												//按行存储
	sections.reserve(n);
	cnt = 0;
	atomic_int cnt2 = 0;
	tmpsize = static_cast<int>(sectionsGrid.size());
	auto scanMeasure = [&cnt, &cnt2, &sections](const general& i, int index) {
		vector<Mat> origin;
		if (!get<1>(i).empty()) cut(get<2>(i), get<1>(i), 0, origin, true);	//切割

		getkey(rowLenth) += get<2>(i).rows;

		while (cnt < index) this_thread::yield();
		size_t pre = sections.size();
		for (size_t j = 0; j < origin.size(); j++) {
			sections.emplace(sections.begin() + pre + j, origin[j], pre + j + 1);
		}
		cnt++;

		for (size_t j = 0; j < origin.size(); j++) {
			sections[pre + j].start(get<0>(i));
		}
		cnt2++;
	};
	for (int i = 0; i < tmpsize; i++) {
		thread t(scanMeasure, ref(sectionsGrid[i]), i);
		t.join();
	}
	while (cnt2 < tmpsize) {
		progress(cnt2 / tmpsize / 2 + 30);
		this_thread::yield();
	}
	piece.clear(); piece.shrink_to_fit();

	notify("扫描完成，准备写入文件");
	progress(80);
	global.save();
	string name = fname(f);
	saveDoc finish(name, "unknown", "unknown", "unknown", PROJECT, "Internet");
	n = static_cast<int>(sections.size());
	for (int i = 0; i < n; i++) {
		assert(sections[i].id > 0);
		finish.saveMeasure(sections[i].getNotes()); 
		progress(80 + i / n);
	}
	string fn = outputDir;
	if (fn.back() != '\\') fn.append("\\");
	fn.append(name).append(".xml");
	if (isExist(fn)) if (prompt(NULL, fn + " 已经存在，要替换吗？", PROJECT, 0x34) == 7) {
		notify("用户放弃了保存");
		return 1;
	}
	if (0 == finish.save(fn)) {
		progress(100);
		notify("Success");
	}
	else notify("Error when saving doc. ");
	cv::destroyAllWindows();
	return 0;
}