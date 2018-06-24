#pragma once


#include"Cuckoo.h"
#include "swan.h"
using namespace std;
#define mode 0  //0-main program  1-train  2-test

int col;

void fname(const char* path,char* name) {
	int start, end;
	int n = (int)strnlen_s(path, 260);
	for (int i = n - 1; i >= 0; i--) {
		if (path[i] == '.' || path[i] == '\\') {
			end = i;
			for (i = i - 1; i >= 0; i--) {
				if (path[i] == '\\') {
					break;
				}
			}
			start = i + 1;
			goto copy;
		}
	}
	strncpy(name,path,n);
	return;
copy:
	strncpy(name, &path[start], end - start);
	return;
}

int go(string f,bool isCut) {
#if mode == 1
	train();
	return 0;
#endif
	size_t n;
	int cutTimes;
	bool flag = false, dog = false;
	std::vector<space> coll;
	std::vector<space> toCut;
#if mode == 2
	std::vector<int> poss;
	std::cout << rec(cv::imread(f),poss) << std::endl;
	system("pause");
	return 0;
#endif
	cv::Mat img = threshold(f);
	col = img.cols;
	if (img.empty()) {
		std::cout << "Wrong format." << std::endl;
		system("pause");
		return 1;
	}
	cv::Mat trimmed = trim(img);
	//imshow("2", trimmed); cvWaitKey();
	
	
	cutTimes = split(trimmed, coll);
	if (cutTimes == 3) {
		return 1;
	}
	n = coll.size();
	bool *r = new bool[n];
	for (size_t i = 0; i < n; i++) r[i] = false;
	for (;;) {
		int a = interCheck(coll);
		if (a == -1) break;
		else r[a] = true;
	};
	bool* mm = KClassify(coll);
	for (size_t k = 0,i = 0; i < n; i++) {
		if (!r[i]) {
			r[i] = mm[k++];
		}
	}
	delete[] mm;
	n = coll.size();
	
	/*cv::Mat ccolor;
	cvtColor(trimmed, ccolor, CV_GRAY2BGR);*/
	for (int i = 0; i < n; i++) {
		if (r[i]) {
			
			
			/*line(ccolor, CvPoint(0, coll[i].start), CvPoint(trimmed.cols, coll[i].start), CvScalar(0, 0, 255));
			line(ccolor, CvPoint(0, coll[i].start + coll[i].length), CvPoint(trimmed.cols, coll[i].start + coll[i].length), CvScalar(0, 0, 255));
			
			*/
			toCut.push_back(coll[i]);
		}
	}
	//imwrite("as.jpg",ccolor);
	if (toCut.size() > 2) {
		std::cout << "过滤算法正常" << std::endl;
		coll.clear();
	}
	else {
		toCut.swap(coll);
		dog = true;
	}
	n = toCut.size();
	delete[] r;

	std::vector<cv::Mat> piece;
	std::vector<cv::Mat> chords;
	std::vector<measure> sections;
	std::vector<cv::Mat> timeValue;
	std::vector<cv::Mat> info;
	std::vector<cv::Mat> notes;
	std::vector<cv::Vec4i> pos;
	std::vector<cv::Mat> nums;
	cv::Mat toOCR;

	for (int i = 0; i <= n; i++) {
		cv::Mat tmp;
		piece.push_back(tmp);
	}
	//cut<space>(trimmed, toCut, [](space x) ->int& {return x.start; }, 1, piece);
	trimmed(cv::Range(0, toCut[0].start), cv::Range(0, trimmed.cols)).copyTo(piece[0]);
	for (int i = 1; i < n; i++) {
		trimmed(cv::Range(toCut[i - 1].start + toCut[i - 1].length + 1, toCut[i].start),
			cv::Range(0, trimmed.cols)).copyTo(piece[i]);
	}
	trimmed(cv::Range(toCut[n - 1].start + toCut[n - 1].length + 1, trimmed.rows), cv::Range(0, trimmed.cols)).copyTo(piece[n]);
	
	if (dog && cutTimes == 2) {
		std::cout << "运行修补算法" << std::endl;
		std::vector<space> toJoin;
		for (int i = 1; i <= n; i++) toJoin.push_back({0,piece[i].rows});
		bool* b = new bool[toCut.size()+1]();
		do{
			int a = interCheck(toJoin);
			if(SUCCEED(a)) b[a] = true;
			else break;
		}while(1);
		r = KClassify(toJoin);
		for (int k=0,i = 1; i <= n; i++) {
			if (b[i]) b[i] = false;
			else b[i] = r[k++];
		}
		delete[] r;
		for (size_t i = n; i > 0; i--) {
			if (!b[i]) {
				//将piece[i]与piece[i-1]拼接在一起
				vconcat(piece[i-1],piece[i], piece[i - 1]);
				piece[i].release();
			}
		}
		delete[] b;
	}
	
	for (int i = 0; i <= n; i++) {
		piece[i] = trim(piece[i]);
	}
	int k = 1;
	for (int i = 0; i <= n; i++) {
		if (piece[i].empty()) continue;
		std::vector<cv::Vec4i> rows;
		int thick = 0;
		findRow(piece[i], CV_PI / 18, rows,thick);
		if (rows.size() > 5) {
			flag = true;
			chords.push_back(piece[i]);
			//为OCR去掉横线
			//用形态学腐蚀得到mask 将mask上的点置0
			cv::Mat dilated;
			dilated = 255 - Morphology(piece[i],piece[i].cols/50,true,true);
			dilated = Morphology(dilated, 2, true, true);
			//imshow("2", dilated); cvWaitKey();
			toOCR = cv::max(dilated, piece[i]);
			
			std::vector<cv::Vec4i> lines;
			int max = std::min(rows[5][1], rows[5][3]);
			int min = std::max(rows[0][1], rows[0][3]);

			findCol(piece[i], CV_PI / 18 * 8, max, min, thick, lines);
			std::vector<cv::Mat> origin;
			std::vector<cv::Mat> section;
			if (lines.size()) {
				cut(toOCR, lines, 0, section, true);
				cut(piece[i], lines, 0, origin, true);
			}
			if (rowLenth) { 
				if(piece[i].rows > rowLenth / 2 && piece[i].rows < rowLenth * 2)
					rowLenth = (rowLenth + piece[i].rows) / 2;
			}
			else  rowLenth = piece[i].rows;
			for (size_t j = 0; j < section.size(); j++) {
				measure newSec(origin[j],section[j],rows,(int)k);
				if (SUCCEED(newSec.id)) {
					k++;
				}
				sections.push_back(newSec);
			}
		}
		else {
			if(flag) notes.push_back(piece[i]);
			else info.push_back(piece[i]);
		}
	}
	piece.clear();

	//saveNums("C:\\Users\\Administrator\\Desktop\\oh", nums);
	//system("pause");

	/*system("pause");
	system("cls");
	for (measure &i : sections) {
		for (note &j : i.notes) {
			if(!j.chord) cout << " | " << j.timeValue << "   ";
			cout << j.notation.technical.string << "弦" << j.notation.technical.fret << "品";
		}
		cout << endl << endl;
	}
	system("pause");*/
	char name[256] = "";
	fname(f.c_str(),name);
	saveDoc finish(name,"unknown","unknown","unknown","chordConverter","Escapeland");
	for (measure& i : sections) {
		if(SUCCEED(i.id)) finish.saveMeasure(i);
	}
	string fn = name;
	fn = fn + ".xml";
	finish.save(fn.c_str());
	return 0;
}
