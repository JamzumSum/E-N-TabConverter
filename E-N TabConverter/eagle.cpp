#include "stdafx.h"
#include "Dodo.h"
#include "eagle.h"
#include "tools.h"
#include "type.h"
#include <fstream>

#define IDR_ML_CSV1 103

/**
	识别传入的字符. 
	@name NumReader::rec
	@param	character cv::Mat, the character's image. 
				requires size of the Mat equals to preferWidth * preferHeight. 
	@param	threshold, float, if the possibility is greater than the threshold, the result will
				be pushed into the return val. 
	@retval	map of(-1->-1) is returned if 'reshape' throws an error; or no result has a possibility 
				greater than threshold. 
*/
map<char, float> NumReader::rec(Mat character, float threshold) {
	cvtColor(character, character, CV_GRAY2BGR);
	character = perspect(character, preferWidth, preferHeight);
	
	Mat res, tmp, neighbour, dist;
	//dist: wrong recgonization, 33.244, 47.31, 45.299
	(character.isContinuous() ? character : character.clone()).
		reshape(1, 1).convertTo(tmp, CV_32FC1, 1.0 / 255.0);
	
	load(defaultCSV);
	assert(knn->isTrained());
	knn->findNearest(tmp, 5, res, neighbour, dist);

	map<char, float> ret;
	for (int j = 0; j < dist.cols ; j++) {
		float a = dist.at<float>(0, j);
		if (a < threshold) {
			ret.insert(make_pair(static_cast<char>(neighbour.at<float>(0, j)), a));
		}
		else break;
	}
	return ret.empty() ? map<char, float>({ {-1, -1.0f} }) : ret;
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
			Label.push_back(c);										//与trainData对应的标记
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

