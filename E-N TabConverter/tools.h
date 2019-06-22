#pragma once
#include <string>
#include <vector>

using namespace std;

typedef unsigned long DWORD;
typedef char TCHAR;

string getPath();
void makedir(string folder);
void ls(const char* lpPath, vector<string> &fileList);
bool FreeResFile(DWORD dwResName, const TCHAR* lpResType, const string lpFilePathName);
bool isExist(string filepath);
string GBKToUTF8(string strGBK);
string fname(string path);
int prompt(void* hWnd, string text, string caption, unsigned flag);
pair<float, float> getScaleFactor();
pair<int, int> getWorkspaceSize();
