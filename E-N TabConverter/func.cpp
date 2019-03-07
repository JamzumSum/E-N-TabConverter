#pragma once
#include "stdafx.h"
#include "Dodo.h"
#include "eagle.h"
#include "global.h"
#include "Cuckoo.h"
#include "opencv.hpp"
#include "swan.h"
#include "tools.h"
#include <thread>
#include <functional>

using namespace std;
using namespace cv;

#define imdebug(img, title) imshow((img), (title)); cv::waitKey()

constexpr const char* PROJECT = "E-N TabConverter";
GlobalPool global(cfgPath);

void TrainMode() {
	NumReader::train(defaultCSV);
}
int go(string f, bool isCut, function<void(string)> notify, function<void(int)> progress) {
	int prog = 0;
	bool flag = false;
	Mat img = imread(f.c_str(), 0);
	Mat trimmed = threshold(img);
	if (img.empty()) raiseErr("Wrong format.", 3);
	
	trimmed = trim(trimmed);
	float screenCols = 1919 / 1.25f;				//1919, 最大显示宽度；1.25， win10 系统缩放比
													//TODO：不知道怎么获取QAQ
	if (trimmed.cols > screenCols) {
		float toRows = screenCols / trimmed.cols * trimmed.rows;
		trimmed = perspect(trimmed, int(screenCols), int(toRows));
		trimmed = threshold(trimmed);
	}
	vector<Mat> piece;
	
	Splitter piccut(trimmed);
	piccut.start(piece);
	
	progress(1);
	notify("过滤算法正常");
	
	global.setCol(trimmed.cols);
	vector<measure> sections;					//按行存储
	vector<Mat> info;							//其余信息
	vector<Mat> notes;							//小节
	

	size_t n = piece.size();
	Mat toOCR;

	for (Mat& i: piece) {
		if (i.empty()) continue;
		vector<Vec4i> rows;
		vector<int> thick;
		LineFinder finder(i, 10);
		finder.findRow(rows);
		if (rows.size() == 6) {
			flag = true;
			vector<Vec4i> lines;

			finder.findCol(lines);											//
			vector<Mat> origin;												//切割并存储
			if (lines.size()) cut(i, lines, 0, origin, true);				//

			getkey(rowLenth) += i.rows;

			for (Mat& j: origin) {
				try {
					measure newSec(j, rows, sections.size() + 1);
					if (newSec.id) sections.emplace_back(newSec);
				}
				catch (Err ex) {
					switch (ex.id){
					case 1:
					default: throw ex; break;
					}
				}
			}
		}
		else {
			if(flag) notes.emplace_back(i);
			else info.emplace_back(i);
		}
		prog += 80 / int(n);
		progress(prog);
	}
	piece.clear();

	notify("扫描完成，准备写入文件");
	progress((prog = 80));
	global.save();
	string name = fname(f);
	saveDoc finish(name, "unknown", "unknown", "unknown", PROJECT, "Internet");
	for (measure& i : sections) {
		if(SUCCEED(i.id))
		try { 
			finish.saveMeasure(i); 
		}
		catch (Err ex) {
			switch (ex.id)
			{
			case 3: break;				//unexpected timeValue
										//impossible
			default: break;
			}
		}
		prog += 20 / (int)sections.size();
		progress(prog);
	}
	string fn = name;
	fn += ".xml";
	if (isExist(fn)) if (prompt(NULL, fn + " 已经存在，要替换吗？", PROJECT, 0x34) == 7) {
		notify("用户放弃了保存");
		return 1;
	}
	finish.save(fn);
	progress(100);
	notify("Success");
	destroyAllWindows();
	return 0;
}