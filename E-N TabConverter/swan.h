#pragma once
#include "tinyxml2.h"
#include "music.h"

class saveDoc {
private:
	tinyxml2::XMLDocument backup;
public:
	saveDoc(std::string title, const char* composer, const char* lyricist, const char* artist, const char* tabber, const char* irights);
	int save(std::string xmlPath);
	void saveMeasure(MusicMeasure toSave);
};