#include "ml.hpp"
#include "tools.h"
#include "opencv.hpp"
#include "type.h"
#include <fstream>
#include <string>

#define IDR_ML_CSV1 103
#define samplePath "C:\\Users\\Administrator\\Desktop\\E-N TabConverter\\sample_classified\\"
#define defaultCSV "C:\\Users\\Administrator\\Desktop\\E-N TabConverter\\tData.csv"

typedef unsigned long       DWORD;
typedef _Null_terminated_ const char *LPCSTR;

using namespace cv;
using namespace cv::ml;
using namespace std;

static cv::Ptr<cv::ml::KNearest> &load(string csv, Ptr<KNearest> &knn);

int rec(Mat character, vector<int> &possible, vector<float>& safety, float thresh) {
	Mat res, tmp, neighbour, dist;
	//dist: wrong recgonization, 33.244, 47.31, 45.299
	character.reshape(1, 1).convertTo(tmp, CV_32FC1, 1.0 / 255.0);
	static Ptr<KNearest> knn = KNearest::create();
	knn = load(defaultCSV, knn);
	if (!knn->isTrained()) {
		err ex = {5,__LINE__,"knn网络读取失败"};
		throw ex;
	}
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

void train(string save) {
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

Ptr<KNearest> &load(string csv, Ptr<KNearest> &knn) {
	if (knn->isTrained()) {
		return knn;											//避免重复
	}
	if(!isExist(csv)) FreeResFile(IDR_ML_CSV1, (char*)"ML_CSV", (char*)defaultCSV);

	Ptr<TrainData> trainData = TrainData::loadFromCSV(csv, 0, 0, -1);

	if (trainData.empty()) {
		FreeResFile(IDR_ML_CSV1, (char*)"ML_CSV", (char*)defaultCSV);
		Ptr<TrainData> trainData = TrainData::loadFromCSV(csv, 0, 0, -1);
	}
	knn->setDefaultK(5);
	knn->setIsClassifier(true);
	Mat Label;
	trainData->getResponses().convertTo(Label,CV_32S);
	knn->train(trainData->getSamples(),ROW_SAMPLE,Label);
	return knn;
}

