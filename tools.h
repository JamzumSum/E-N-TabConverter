#pragma once
#include <string>
#include <vector>

using namespace std;

typedef unsigned long DWORD;
typedef const char * LPCSTR;

string getPath();
void fname(const char* path, char* name);
void makedir(string folder);
void ls(const char* lpPath, vector<string> &fileList);
bool FreeResFile(DWORD dwResName, LPCSTR lpResType, LPCSTR lpFilePathName);
bool isExist(string filepath);
string GBKToUTF8(const char* strGBK);
void fname(const char* path, char* name);