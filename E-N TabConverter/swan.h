#pragma once
#include "tinyxml2.h"
#include "music.h"

using std::string;

class saveDoc {
private:
	tinyxml2::XMLDocument backup;
public:
	saveDoc(const string& title, const string& composer, const string& lyricist, const string& artist, const string& tabber, const string& irights);
	int save(string xmlPath);
	void saveMeasure(MusicMeasure toSave);
};