#pragma once
#include <string>
#include <vector>

using namespace std;

typedef unsigned long DWORD;
typedef const char * LPCSTR;

string getPath();
void makedir(string folder);
void ls(const char* lpPath, vector<string> &fileList);
bool FreeResFile(DWORD dwResName, LPCSTR lpResType, LPCSTR lpFilePathName);
bool isExist(string filepath);
string GBKToUTF8(string strGBK);
string fname(string path);
int prompt(void* hWnd, string text, string caption, unsigned flag);
