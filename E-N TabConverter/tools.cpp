/*
	与图像处理无关的过程
*/
#include "stdafx.h"
#include <algorithm>
#include <Windows.h>
#include "tools.h"
#include <io.h>
#include "type.h"

using namespace std;

string GBKToUTF8(string strGBK) {
	/*
		函数名：GBKToUTF8
		功能：将GBK编码转成UTF8，主要应对tinyxml编码处理的问题
	*/
	int len = MultiByteToWideChar(CP_ACP, 0, strGBK.c_str(), -1, NULL, 0);
	wchar_t* wstr = new wchar_t[len + 1];
	memset(wstr, 0, len + 1);
	MultiByteToWideChar(CP_ACP, 0, strGBK.c_str(), -1, wstr, len);
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

string fname(string path) {
	/*
	 * @brief 得到给定全路径指向的文件名，无扩展名
	 * @param[in] path, string, fullpath
	 * @retval string name
	*/
	size_t pos = path.rfind('/');
	if (pos == string::npos) pos = -1;
	size_t dot = path.rfind('.');
	if (dot == string::npos) return path.substr(pos);
	else return path.substr(pos + 1, dot - pos - 1);
}

bool FreeResFile(DWORD dwResName, const TCHAR* lpResType, const string lpFilePathName){
	/*
	 * @brief 释放资源文件
	 * @param[in]
		DWORD  dwResName   资源ID，如IDR_ML_CSV1
		LPCSTR lpResType 指定释放的资源的资源类型，如"ML_CSV"
		LPCSTR lpFilePathName 释放路径（包含文件名）
	 * @retval 成功则返回TRUE,失败返回FALSE
	*/
	HMODULE hInstance = GetModuleHandle(NULL);//得到自身实例句柄  

	HRSRC hResID = ::FindResource(hInstance, MAKEINTRESOURCE(dwResName), lpResType);//查找资源  
	if (!hResID) return false;
	HGLOBAL hRes = ::LoadResource(hInstance, hResID);//加载资源  
	LPVOID pRes = ::LockResource(hRes);//锁定资源  

	if (pRes == NULL)//锁定失败  
	{
		return false;
	}
	DWORD dwResSize = ::SizeofResource(hInstance, hResID);//得到待释放资源文件大小  
	HANDLE hResFile = CreateFile(lpFilePathName.c_str(), GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);//创建文件  

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

void makedir(string folder) {
	if (_access(folder.c_str(), 0) == -1) {
		system(("md " + folder).c_str());
	}
}

bool isExist(string filepath) {
	return _access(filepath.c_str(), 0) >= 0;
}

int prompt(void* hWnd, string text, string caption, unsigned flag) {
	return MessageBox(hWnd ? *(HWND*)hWnd: NULL, text.c_str(), caption.c_str(), flag);
}

/*
@return pair<int, int>, first as horizontal DPI, second as vertical DPI. contains 0 if error. 
*/
pair<float, float> getScaleFactor() {
	auto dc = shared_ptr<HDC__>(GetDC(nullptr), [](const HDC dc) { ReleaseDC(nullptr, dc); });
	auto f = [](const int dpi) -> float {switch (dpi) {
	case 96: return 1.0f;
	case 120: return 1.25f;
	case 144: return 1.5f;
	case 192: return 2.0f;
	default: return 0;
	}};
	return make_pair(f(GetDeviceCaps(&(*dc), LOGPIXELSX)), f(GetDeviceCaps(&(*dc), LOGPIXELSY)));
}

pair<int, int> getWorkspaceSize() {
	auto dc = shared_ptr<HDC__>(GetDC(nullptr), [](const HDC dc) { ReleaseDC(nullptr, dc); });
	return make_pair(GetDeviceCaps(&(*dc), HORZRES), GetDeviceCaps(&(*dc), VERTRES));
}