/*
	global.h 仿佛整个算法的神经系统，它使得各个measure的识别不再相互独立，后面的可以借鉴前面的“经验”
	实际上是一种显式的信息传递。
*/
#pragma once
#include "type.h"
#include <algorithm>
#include <fstream>

//#pragma warning(disable : 4244)

inline  GlobalPool::GlobalPool(std::string path,int col) {
	this->col = col;
	this->path = path;
	std::fstream fso;
	fso.open(path, std::ios::in);
	if (fso.fail()) return;					//文件不存在

	int buffer;

	GlobalUnit*  const pu[5] = { &lineThickness ,&rowLenth ,&colLenth ,&valueSignalLen ,&characterWidth };

	std::vector<int> cols;
	int mp = -1, mp2 = -1;

	for (;;) {
		fso >> buffer;
		if (buffer) cols.push_back(buffer);
		else break;
	}
	if (cols.size() > 1) {
		std::vector<int> tmp(cols);
		for (int &i : tmp) {
			i = abs(i - col);
		}
		auto minele = min_element(tmp.begin(), tmp.end());
		auto min2elel = (tmp.begin() == minele) ? tmp.begin() : min_element(tmp.begin(), minele);
		auto min2eler = (minele + 1 == tmp.end()) ? min2elel : min_element(minele + 1, tmp.end());
		mp2 = (*min2elel < *min2eler) ? int(min2elel - tmp.begin()) : int(min2eler - tmp.begin());
		mp = int(minele - tmp.begin());
	}
	else if (cols.size() == 1) if (col == cols[0]) {
		mp = mp2 = 0;
	}
	

	for (int j = 0; j < 5;j++) {
		std::vector<int> vals;
		for (int i = 0;i<cols.size();i++) {
			fso >> buffer;
			vals.push_back(buffer);
		}

		if (mp == -1 && mp2 == -1) continue;		//少于两个点无法预测
		else if (mp == 0 && mp2 == 0) (*(pu[j])) += vals[mp];
		else  (*(pu[j])) += (col - cols[mp2]) * (vals[mp2] - vals[mp]) / (cols[mp2] - cols[mp]) + vals[mp2];
	}
	fso.close();
}

inline GlobalPool::~GlobalPool() {
	std::fstream fso;
	std::vector<int> cols, vals[5];
	GlobalUnit*  const pu[5] = { &lineThickness ,&rowLenth ,&colLenth ,&valueSignalLen ,&characterWidth };
	int buffer = 0;
	int pos = 0;
	fso.open(path, std::ios::in);

	if (fso.fail()) {
		cols.push_back(col);
		for (int i = 0; i < 5; i++) {
			vals[i].push_back(*pu[i]);
		}
		goto gblwrite;								//直接跳到写
	}

	for (;;) {
		fso >> buffer;
		if (buffer) {
			if (col < buffer) {
				if (!pos) {
					pos = (int)cols.size() + 1;
					cols.push_back(col);
				}
				cols.push_back(buffer);
			}
			else if (col == buffer) {
				pos = -((int)cols.size()) - 1;
				cols.push_back(buffer);
			}
			else cols.push_back(buffer);
		}
		else {
			if (!pos) {
				pos = (int)cols.size() + 1;
				cols.push_back(col);
			}
			break;
		}
	}
	for (int j = 0; j < 5; j++) {
		for (int i = 0;i < (int)cols.size() - ((pos > 0) ? 1 : 0);i++) {
			fso >> buffer;
			vals[j].push_back(buffer);
		}
		if (pos > 0) {
			if (pos - 1 == vals[j].size()) vals[j].push_back(*pu[j]);
			else vals[j].insert(vals[j].begin() + pos - 1, *pu[j]);
		}
		else {
			vals[j].at(-pos - 1) = *pu[j];
		}
	}
	fso.close();

gblwrite:
	fso.open(path, std::ios::out | std::ios::trunc);			//输出，直接截断

	for (int i : cols) fso << i << '\t';
	fso << 0 << std::endl;
	for (int j = 0; j < 5; j++) {
		for (int i = 0; i < (int)vals[j].size()-1; i++) {
			fso << vals[j].at(i) << '\t';
		}
		fso << vals[j].at(vals[j].size() - 1) << std::endl;
	}
	fso.close();
}

extern GlobalPool* global;