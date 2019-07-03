#pragma once
#include <string>
#include <vector>

using std::string;
using std::vector;
using std::pair;

typedef unsigned long DWORD;
typedef char TCHAR;


string getPath();
void makedir(string folder);
void ls(const char* lpPath, vector<string> &fileList);
bool FreeResFile(DWORD dwResName, const TCHAR* lpResType, const string lpFilePathName);
bool isExist(string filepath);
string GBKToUTF8(string strGBK);
string Utf8ToGbk(const char* src_str);
string fname(string path);
int prompt(void* hWnd, string text, string caption, unsigned flag);
pair<float, float> getScaleFactor();
pair<int, int> getWorkspaceSize();
