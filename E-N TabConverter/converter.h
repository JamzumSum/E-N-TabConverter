#pragma once

/*
	converter.h
	function to export. 
*/
#include <functional>
#include <string>
#include <vector>

using std::string;
using std::function;
using std::vector;

class Converter {
private:
	vector<string> picPath;
	bool ifCut;
	string outputDir;
public:
	Converter(const vector<string>& pics) {
		picPath = pics;
	}

	static void Train();

	int scan(function<void(string)> notify, function<void(int)> progress);

	void setCut(bool ifCut) {
		this->ifCut = ifCut;
	}

	bool getCut() { return ifCut; }

	void setOutputDir(string dir) {
		outputDir = dir;
	}

	string getOutputDir() { return outputDir; }


};