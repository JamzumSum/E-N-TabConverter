#include "stdafx.h"
#include "eagle.h"
#include "tools.h"
#include "type.h"
#include <fstream>

#define IDR_ML_CSV1 103

int NumReader::rec(Mat character, vector<int> &possible, vector<float>& safety, float thresh) {
	Mat res, tmp, neighbour, dist;
	//dist: wrong recgonization, 33.244, 47.31, 45.299
	try {
		character.reshape(1, 1).convertTo(tmp, CV_32FC1, 1.0 / 255.0);
	}
	catch (exception ex) {
		return -1;
	}
	load(defaultCSV);
	if (!knn->isTrained()) throw runtime_error("knn网络读取失败");
	
	knn->findNearest(tmp, 5, res, neighbour, dist);

	possible.clear();
	safety.clear();
	for (int j = 0; j < dist.cols ; j++) {
		float a = dist.at<float>(0, j);
		if (a < thresh) {
			safety.emplace_back(a);
			possible.emplace_back((int)neighbour.at<float>(0, j));
		}
		else break;
	}
	sort(possible.begin(), possible.end());
	possible.erase(unique(possible.begin(),possible.end()),possible.end());		//去重
	return possible.empty() ? -1 : (int)res.at<float>(0, 0);
}

void NumReader::train(string save) {
	//trainData 个数*大小
	//Labels 个数*10
	Mat trainData, Label ,CSV;
	//录入训练样本和标记
	int num;															//num 是样本是什么数字
	vector<string> fileList;
	string path = samplePath;
	for (num = 0; num < 10; num++) {
		char c = num + '0';
		ls((path + c).c_str(), fileList);
		for (int i = 0; i < (int)fileList.size(); i++) {
			Mat tmp = imread(string(fileList[i]));
			trainData.push_back(tmp.reshape(1, 1));
			Label.push_back(num);										//与trainData对应的标记
		}
		fileList.clear();
	}
	Label.convertTo(Label,CV_32F);
	trainData.convertTo(trainData, CV_32FC1, 1.0 / 255.0);
	hconcat(Label, trainData, CSV);
	//Ptr<TrainData> tData = TrainData::create(trainData, ROW_SAMPLE, Label);
	ofstream file(save);
	file << format(CSV, Formatter::FMT_CSV);
	file.close();
}

void NumReader::load(string csv) {
	if (knn->isTrained()) return;											//避免重复
	
	if(!isExist(csv)) FreeResFile(IDR_ML_CSV1, "ML_CSV", defaultCSV);

	Ptr<TrainData> trainData = TrainData::loadFromCSV(csv, 0, 0, -1);

	if (trainData.empty()) {
		FreeResFile(IDR_ML_CSV1, "ML_CSV", defaultCSV);
		Ptr<TrainData> trainData = TrainData::loadFromCSV(csv, 0, 0, -1);
	}
	knn->setDefaultK(5);
	knn->setIsClassifier(true);
	Mat Label;
	trainData->getResponses().convertTo(Label,CV_32S);
	knn->train(trainData->getSamples(),ROW_SAMPLE,Label);
}

