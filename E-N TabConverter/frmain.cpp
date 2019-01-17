#include <string>
#include "type.h"
#include "..\GUIlib\GUI.h"

using namespace std;

/*
	frmain.cpp 控制程序的GUI
*/

#define IDI_ICON1 101
#define IDI_WINDOW1 102
#define IDB_BITMAP1 106

int pix = 80;
static string noti;
char f[MAX_PATH];
char prog[4];
bool isCut = false;

static form main(NULL, "form", "E-Land Chord Converter", 240, 240, 840, 528);
static Button scan(&main, 5 * pix, 200, 112, 56, "Go!");
static Button home(&main, 8, 0, 112, 56, "Home");
static Button history(&main, 8, 64, 112, 56, "History");
static Button setting(&main, 8, 128, 112, 56, "Settings");
static Button Exit(&main, 8, 400, 112, 56, "Exit");
static Label info(&main, 4, 464, 560, 24, "Press \"Go\" to begin.");
static Checkbox cut(&main, 760, 450, 56, 32, "Cut");

extern int go(string f,bool);
static string conc(string n, char p[4]);


notify<int> progress([](int p) {
	char num[4];
	_itoa_s(p, num, 10);
	strncpy_s(prog, num, 4);
	info.name = (TCHAR*) conc(noti, prog).c_str();
});

notify<string> notification([](string n) {
	noti = n;
	info.name = (TCHAR*) conc(noti,prog).c_str();
});

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR szCmdLine, int iCmdShow) {
	main.setIcon(MAKEINTRESOURCE(IDI_WINDOW1), MAKEINTRESOURCE(IDI_ICON1));
	//main.bitmapName = MAKEINTRESOURCE(IDB_BITMAP1);
	main.create();
	main.Event_Window_Resize = [](form* me) {
		pix = me->w / 12;
	};
	
	scan.Event_On_Click = [](Button* me) {
		OPENFILENAME ofn;
		ZeroMemory(&ofn, sizeof(ofn));
		ofn.lStructSize = sizeof(ofn);
		ofn.hwndOwner = main.hWnd;
		ofn.lpstrDefExt = 0;
		ofn.lpstrFile = f;
		ofn.lpstrFile[0] = '\0';
		ofn.nMaxFile = MAX_PATH;
		ofn.lpstrFilter = "图片文件\0*.bmp;*.jpg;*.JPG;*.jpeg;*.png;*.gif\0\0";
		ofn.nFilterIndex = 0;
		ofn.lpstrInitialDir = 0;
		ofn.lpstrTitle = "选择乐谱：";
		ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

		info.name = (TCHAR*) "Then choose a tab.";

		if (GetOpenFileName(&ofn))
		{
			info.name = f;
			try {
				go(string(f), isCut);
			}
			catch (err ex) {
				switch (ex.id)
				{
				case 3:
					//不支持的格式
					info.name = (TCHAR*) ex.description.insert(0,"Failure: ").c_str();
					return;
				default:
					info.name = (TCHAR*) ex.description.insert(0, "Error: ").c_str();
					break;
				}
			}
			notification = "Success";
			main.top();
		}
	};
	main.Event_Load_Complete = [](form* me) {
		pix = me->w / 12;
	};
	

	home.Event_On_Click = [](Button* me) {
		scan.show();
	};
	Exit.Event_On_Click = [](Button* me) {
		void* p = me->parent;
		((form*)p)->close();
	};
	cut.Event_On_Check = [](Checkbox* me) {
		isCut = me->Value;
	};


	main.run();
}

string conc(string n,char p[4]) {
	return n + "------" + p + "%";
}