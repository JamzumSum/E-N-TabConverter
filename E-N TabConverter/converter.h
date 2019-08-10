#pragma once

/*
	converter.h
	function to export. 
*/
#include <functional>
#include <string>
#include <vector>
#include "eagle.h"
#include "Accessor.hpp"
#include "global.h"

using std::string;
using std::function;
using std::vector;

constexpr bool TesseractEnabled = false;
class CharReader;

class Converter {
private:
	vector<string> picPath;
	string outputDir;
	function<string(void)> selectSaveStrategy;
	GlobalPool global;
	const Property& prop = global.Prop;
	CharReader* reader = nullptr;

	void* api;
	bool ifCut;
	void* doc = nullptr;
	bool ocrReady = false;

	auto scan(const int startWith, string path, function<void(string)> notify, function<void(int)> progress);
	void title(const vector<cv::Mat>& info);
public:
	Converter(const vector<string>& pics);
	Converter() {}

	void Train();

	static const std::tuple<int, int, int> cvVersion();
	static const std::tuple<int, int, int> TinyXMLVersion();
	static const std::tuple<int, int, int> Version();

	string scan(function<void(string)> notify, function<void(int)> progress);

	Accessor<bool, ReadWrite> Cut = prop.ifCut;
	Accessor<string, ReadWrite> OutputDir = outputDir;
	Accessor<function<string(void)>, WriteOnly> SelectSaveStrategy = selectSaveStrategy;
	Accessor<vector<string>, WriteOnly> PicPath = picPath;
	Accessor<string, ReadWrite> CfgPath = prop.cfgPath;
	Accessor<string, ReadWrite> CSVPath = prop.defaultCSV;
	Accessor<string, ReadWrite> PicSavePath = prop.picFolder;
	Accessor<string, ReadWrite> SamplePath = prop.samplePath;
	Accessor<bool, ReadWrite> SavePic = prop.savePic;
	Accessor<Property, ReadOnly> Prop = prop;
	Accessor<GlobalPool, ReadOnly> Global = global;
	Accessor<CharReader*, ReadOnly> Reader = reader;

	~Converter();
};