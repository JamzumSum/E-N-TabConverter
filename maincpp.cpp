#pragma once
#include"Cuckoo.h"
#include "swan.h"

using namespace std;

int col = 0;

#if workMode
int main() {
	train();
	return 0;
}
#else

extern notify<int>progress;
extern notify<std::string>notification;

int go(string f,bool isCut) {
	size_t n;
	int cutTimes;
	bool flag = false, dog = false;
	std::vector<space> coll;
	std::vector<space> toCut;
	cv::Mat img = threshold(f);

	col = img.cols;
	if (img.empty()) {
		err ex = { 3,__LINE__,"Wrong format." };
		throw ex;
		return 1;
	}
	cv::Mat trimmed = trim(img);
	//imshow("2", trimmed); cvWaitKey();
	
	try {
		cutTimes = split(trimmed, coll);
	}
	catch (err ex) {
		switch (ex.id)
		{
		case 4:
			imshow("WTF", trimmed);cvWaitKey();
			cvDestroyAllWindows();
			return 1;
		default:
			break;
		}
	}

	//此时所有条件满足，算法开始

	n = coll.size();
	vector<int> t;
	vector<bool> r(n,false);
	
	for (size_t i = 0; i < n; i++) r[i] = false;
	interCheck(coll, t);
	n = coll.size();

	vector<bool> mm(n, false);
	for (int i:t) r[i] = true;
	KClassify(coll,mm);
	for (size_t k = 0,i = 0; i < n; i++) {
		if (!r[i]) {
			r[i] = mm[k++];
		}
	}
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
		notification = "过滤算法正常";
		coll.clear();
		progress = 1;
	}
	else {
		toCut.swap(coll);
		dog = true;
	}
	n = toCut.size();

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
		/*
			这个if块里是一种用于补救的手段。。如果分割效果不好会触发这个条件。。。
			但是现在分割效果还算好，而且本身这个补救手段就很欠考虑。。。
			另外，在多次算法改进之后，这个if块是否还正确亦未可知。。。
		*/
		notification = "运行修补算法";
		std::vector<space> toJoin;
		for (int i = 1; i <= n; i++) toJoin.push_back({0,piece[i].rows});
		bool* b = new bool[toCut.size()+1]();
		interCheck(toJoin, t);
		for (int i : t) r[i] = true;
		KClassify(toJoin,r);
		for (int k=0,i = 1; i <= n; i++) {
			if (b[i]) b[i] = false;
			else b[i] = r[k++];
		}
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
		//每个分割出来的行再剪去空白
		piece[i] = trim(piece[i]);
	}

	int k = 1;
	for (int i = 0; i <= n; i++) {
		if (piece[i].empty()) continue;
		std::vector<cv::Vec4i> rows;
		vector<int> thick;
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
			
			vector<cv::Vec4i> lines;
			int max = min(rows[5][1], rows[5][3]);
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
				try {
					measure newSec(origin[j], section[j], rows, (int)k);
					if (SUCCEED(newSec.id)) {
						k++;
					}
					sections.push_back(newSec);
				}
				catch (err ex) {
					switch (ex.id)
					{
					case 1:
						//timeValue越界
					case 6:
						//没有竖直结构用以判断时值
						#if _DEBUG
							imshow("2", origin[j]); cvWaitKey();
						#endif
						break;
					default:
						throw ex;
						break;
					}
				}
			}
		}
		else {
			if(flag) notes.push_back(piece[i]);
			else info.push_back(piece[i]);
		}
		progress = progress + 49 / (int)(n + 1);
	}
	piece.clear();

	notification = "扫描完成，准备写入文件";
	progress = 50;
	char name[256] = "";
	fname(f.c_str(),name);
	saveDoc finish(name,"unknown","unknown","unknown","chordConverter","Escapeland");
	for (measure& i : sections) {
		if(SUCCEED(i.id))
		try { 
			finish.saveMeasure(i); 
		}
		catch (err ex) {
			switch (ex.id)
			{
			case 3:
				//unexpected timeValue
				break;
			default:
				break;
			}
		}
		progress = progress + 50 / (int)sections.size();
	}
	string fn = name;
	fn = fn + ".xml";
	finish.save(fn.c_str());
	progress = 100;
	cvDestroyAllWindows();
	return 0;
}

#endif