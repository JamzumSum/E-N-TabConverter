#pragma once
#include "tinyxml2.h"
#include "../Cuckoo/Cuckoo.h"

class saveDoc {
private:
	tinyxml2::XMLDocument backup;
public:
	saveDoc(string title, const char* composer, const char* lyricist, const char* artist, const char* tabber, const char* irights);
	int save(string xmlPath);
	void saveMeasure(measure toSave);
};