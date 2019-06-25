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

void TrainMode() {
	NumReader::train(defaultCSV);
}

int go(const vector<string>& pics, bool isCut, function<void(string)> notify, function<void(int)> progress, string outputDir) {
	atomic_int prog = 0;
	bool flag = false;
	string f = pics[0];
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
	
	size_t n = piece.size();
	vector<measure> sections;					//按行存储
	vector<Mat> info;							//其余信息
	
	
	for (Mat& i: piece) {
		vector<Vec4i> rows;
		vector<int> thick;
		LineFinder finder(i, 10);
		finder.findRow(rows);
		if (rows.size() == 6) {
			vector<Vec4i> lines;

			finder.findCol(lines);											//
			vector<Mat> origin;												//切割并存储
			if (!lines.empty()) cut(i, lines, 0, origin, true);				//

			getkey(rowLenth) += i.rows;

			for (size_t j = 0, pre = sections.size(); j < origin.size(); j++) {
				sections.emplace_back(origin[j], pre + j + 1);
				assert(sections[pre + j].id != 0);
				sections[pre + j].start(rows);
			}
		}
		else info.emplace_back(i);
		prog += 80 / int(n);
		progress(prog);
	}
	piece.clear(); piece.shrink_to_fit();

	//sort(sections.begin(), sections.end());
	notify("扫描完成，准备写入文件");
	progress((prog = 80));
	global.save();
	string name = fname(f);
	saveDoc finish(name, "unknown", "unknown", "unknown", PROJECT, "Internet");
	for (measure& i : sections) {
		if(SUCCEED(i.id))
		finish.saveMeasure(i.getNotes()); 
		prog += 20 / (int)sections.size();
		progress(prog);
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