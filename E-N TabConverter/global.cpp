#include "stdafx.h"
#include "global.h"

using namespace std;

GlobalUnit& GlobalPool::get(string key) {
	return table[key];
}

GlobalPool::GlobalPool(string path): path(path) {
	
}

/*
 * @retval -1 when error, else predicted value. 
*/
int GlobalPool::predictValue(int col, map<int, int> col2value) {
	if (col2value.size() < 2) return -1;

	float avrx = 0, avry = 0, avrxy = 0, avrx2 = 0;
	for (auto i : col2value) {
		avrx += i.first;
		avry += i.second;
		avrxy += i.first * i.second;
		avrx2 += i.first * i.first;
	}
	
	float n = (float)col2value.size();
	avrx /= n;
	avry /= n;
	avrxy /= n;
	avrx2 /= n;
	float k = (avrxy - avrx * avry) / (avrx2 - avrx * avrx);
	return (int) round(avry + k * (col - avrx));
}

void GlobalPool::setCol(int col) {
	/* like this...
		<GLOBAL POOL>
			<rowlength col = 1980>5</rowlength>
			<rowlength col = 3160>7</rowlength>
			<collength col = 1980>3</collength>
			<collength col = 3160>5</collength>
		</GLOBAL POOL>
	*/
	using namespace tinyxml2;

	this->col = col;
	XMLDocument doc;
	XMLElement* root;
	XMLError errXml = doc.LoadFile(path.c_str());

	//then get root
	if (XML_SUCCESS == errXml) root = doc.RootElement();
	else return;
	

	map<string, map<int, int>> all;
	for (XMLElement* elmkey = root->FirstChildElement(); elmkey; elmkey = elmkey->NextSiblingElement()) {
		all[elmkey->Name()][elmkey->IntAttribute("col")] = atoi(elmkey->GetText());
	}
	//all data in `all` yet. 

	for (auto i : all) {
		int p = predictValue(col, i.second);
		table[i.first] = p > 0 ? GlobalUnit(p) : GlobalUnit();
	}

}

void GlobalPool::save() {
	using namespace tinyxml2;

	XMLDocument doc;
	XMLElement* root;
	XMLError errXml = doc.LoadFile(path.c_str());

	//then get root
	if (XML_SUCCESS == errXml) {
		root = doc.RootElement();
	}
	else {
		XMLDeclaration* declaration = doc.NewDeclaration();
		doc.InsertFirstChild(declaration);
		root = doc.NewElement("GLOBALPOOL");
		doc.InsertEndChild(root);
	}

	//search for: <key col = this.col>
	for (auto i : table) {
		XMLElement* elmkey = NULL;
		for (elmkey = root->FirstChildElement(i.first.c_str()); elmkey; elmkey = root->NextSiblingElement(i.first.c_str())) {
			if (elmkey->IntAttribute("col") == this->col) break;
		}
		if (elmkey) {
			elmkey->SetText(int(i.second));
		}
		else {
			elmkey = doc.NewElement(i.first.c_str());
			elmkey->SetAttribute("col", this->col);
			elmkey->SetText(int(i.second));
			root->InsertEndChild(elmkey);
		}
	}

	doc.SaveFile(path.c_str());
}