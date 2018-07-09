/*
	与图像处理无关的过程
*/
#include <Windows.h>
#include <vector>
#include <algorithm>
#include <string>
#include "type.h"

using namespace std;

#define judge(x, statement) [](auto x) ->bool {return statement; }
#define dA(type, x, pool, p) always<type>(pool,judge(x,p))
#define dE(type,x, pool, p) exist<type>(pool,judge(x,p))

string GBKToUTF8(const char* strGBK) {
	/*
		函数名：GBKToUTF8
		功能：将GBK编码转成UTF8，主要应对tinyxml编码处理的问题
	*/
	int len = MultiByteToWideChar(CP_ACP, 0, strGBK, -1, NULL, 0);
	wchar_t* wstr = new wchar_t[len + 1];
	memset(wstr, 0, len + 1);
	MultiByteToWideChar(CP_ACP, 0, strGBK, -1, wstr, len);
	len = WideCharToMultiByte(CP_UTF8, 0, wstr, -1, NULL, 0, NULL, NULL);
	char* str = new char[len + 1];
	memset(str, 0, len + 1);
	WideCharToMultiByte(CP_UTF8, 0, wstr, -1, str, len, NULL, NULL);
	string strTemp = str;
	if (wstr) delete[] wstr;
	if (str) delete[] str;
	return strTemp;
}

string getPath() {
	/*
		函数名：getPath
		功能：得到程序所在目录
	*/
	char szFilePath[MAX_PATH + 1] = { 0 };
	GetModuleFileNameA(NULL, szFilePath, MAX_PATH);
	(strrchr(szFilePath, '\\'))[0] = 0;								// 删除文件名，只获得路径
	string path = szFilePath;

	return path;
}

void fname(const char* path, char* name) {
	/*
		函数名：fname
		功能：得到给定全路径指向的文件名
		返回值： char* name
	*/
	int start, end;
	int n = (int)strnlen_s(path, 260);
	for (int i = n - 1; i >= 0; i--) {
		if (path[i] == '.' || path[i] == '\\') {
			end = i;
			for (i = i - 1; i >= 0; i--) {
				if (path[i] == '\\') {
					break;
				}
			}
			start = i + 1;
			goto copy;
		}
	}
	strncpy(name, path, n);
	return;
copy:
	strncpy(name, &path[start], end - start);
	return;
}

bool FreeResFile(DWORD dwResName, LPCSTR lpResType, LPCSTR lpFilePathName){
	/*
		功能：释放资源文件
		参数：
			DWORD dwResName   资源ID，如IDR_ML_CSV1
			LPCSTR lpResType 指定释放的资源的资源类型，如"ML_CSV"
			LPCSTR lpFilePathName 释放路径（包含文件名）

		返回值：成功则返回TRUE,失败返回FALSE
	*/
	HMODULE hInstance = ::GetModuleHandle(NULL);//得到自身实例句柄  

	HRSRC hResID = ::FindResource(hInstance, MAKEINTRESOURCE(dwResName), lpResType);//查找资源  
	HGLOBAL hRes = ::LoadResource(hInstance, hResID);//加载资源  
	LPVOID pRes = ::LockResource(hRes);//锁定资源  

	if (pRes == NULL)//锁定失败  
	{
		return false;
	}
	DWORD dwResSize = ::SizeofResource(hInstance, hResID);//得到待释放资源文件大小  
	HANDLE hResFile = CreateFile(lpFilePathName, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);//创建文件  

	if (INVALID_HANDLE_VALUE == hResFile)
	{
		//TRACE("创建文件失败！");  
		return false;
	}

	DWORD dwWritten = 0;//写入文件的大小     
	WriteFile(hResFile, pRes, dwResSize, &dwWritten, NULL);//写入文件  
	CloseHandle(hResFile);//关闭文件句柄  

	return (dwResSize == dwWritten);//若写入大小等于文件大小，返回成功，否则失败  
}

void ls(const char* lpPath, std::vector<std::string> &fileList){
	char szFind[MAX_PATH];
	WIN32_FIND_DATA FindFileData;

	strcpy_s(szFind, lpPath);
	strcat_s(szFind, "\\*.jpg");

	HANDLE hFind = ::FindFirstFile(szFind, &FindFileData);
	if (INVALID_HANDLE_VALUE == hFind)    return;

	while (true)
	{
		if (FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
		{
			if (FindFileData.cFileName[0] != '.')
			{
				char szFile[MAX_PATH];
				strcpy_s(szFile, lpPath);
				strcat_s(szFile, "\\");
				strcat_s(szFile, (char*)(FindFileData.cFileName));
				ls(szFile, fileList);
			}
		}
		else
		{
			//std::cout << FindFileData.cFileName << std::endl;  
			char szFile[MAX_PATH];
			strcpy_s(szFile, lpPath);
			strcat_s(szFile, "\\");
			strcat_s(szFile, FindFileData.cFileName);
			fileList.push_back(std::string(szFile));
		}
		if (!FindNextFile(hFind, &FindFileData))    break;
	}
	FindClose(hFind);
}

template<typename va>
bool exist(vector<va> pool, bool (*p)(va x)) {
	return pool.end() != find_if(pool.begin(), pool.end(), p);
}

template<typename va>
bool always(vector<va> pool, bool(*p)(va x)) {
	return pool.end() == find_if(pool.begin(), pool.end(), !(p));
}

void interCheck(vector<space> &collection, vector<int> &f) {
	size_t n = collection.size();

	if (n <= 1) return;
	f.clear();
	int **pool = new int*[n];
	//初始化截止
	for (int i = 0; i < n; i++) pool[i] = new int[n]();
	for (int i = 0; i < n; i++) {
		for (int j = 0; j < n; j++) {
			if (i == j) continue;
			pool[i][j] = abs(collection[i].length - collection[j].length);
		}
	}
	//初值设置完毕
	//校验开始
	for (int i = 0; i < n; i++) {
		int min = 99999, max = 0;
		for (int j = 0; j < n; j++) {
			if (j == i) continue;
			if (min > pool[i][j]) {
				min = pool[i][j];
			}
			for (int q = 0; q < n; q++) {
				if (q == i) continue;
				if (max < pool[q][j]) {
					max = pool[q][j];
				}
			}
		}
		if (max < min) {
			collection.erase(collection.begin() + i);
			for (int k = 0; k < n; k++) delete[] pool[k];
			delete[] pool;
			f.push_back(i);
			interCheck(collection, f);
			return;
		}
	}
	for (int k = 0; k < n; k++) delete[] pool[k];
	delete[] pool;
	return;
}