/*
	frmain.cpp 控制程序的GUI
*/
#pragma once
#include "type.h"
#include "..\GUIlib\GUI.hpp"

using namespace std;

#define IDI_ICON1 101
#define IDI_WINDOW1 102
#define IDB_BITMAP1 106

static int pix = 80;
static string noti;
static char f[MAX_PATH];
static char prog[4];
static bool isCut = false;
extern bool savepic;


static form main(NULL, "form", "E-Land Chord Converter", 240, 240, 840, 528);
static Button scan(&main, 5 * pix, 200, 112, 56, "Go!");
static Button home(&main, 8, 0, 112, 56, "Home");
static Button history(&main, 8, 64, 112, 56, "History");
static Button setting(&main, 8, 128, 112, 56, "Settings");
static Button Exit(&main, 8, 400, 112, 56, "Exit");
static Label info(&main, 8, 464, 560, 24, "Press \"Go\" to begin.");
static Checkbox save(&main, 760, 432, 56, 28, "save");
static Checkbox cut(&main, 760, 456, 56, 28, "cut");

extern int go(string f, bool cut);
extern void TrainMode();

notify<int> progress([](int p) {
	auto conc = [](string n, char p[4]) -> string {
		return n + "------" + p + "%";
	};

	char num[4];
	_itoa_s(p, num, 10);
	strncpy_s(prog, num, 4);
	info.name = (TCHAR*) conc(noti, prog).c_str();
});
notify<string> notification([](string n) {
	auto conc = [](string n, char p[4]) -> string {
		return n + "------" + p + "%";
	};

	noti = n;
	info.name = (TCHAR*) conc(noti,prog).c_str();
});

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR szCmdLine, int iCmdShow) {
	main.setIcon(MAKEINTRESOURCE(IDI_WINDOW1), MAKEINTRESOURCE(IDI_ICON1));
	//main.bitmapName = MAKEINTRESOURCE(IDB_BITMAP1);
	main.create();
	main.Event_Window_Resize = [](form * me) {
		pix = me->w / 12;
		scan.move(5 * pix, 0);
		me->forAllControl([](control* me) {
			double t = round(me->w / (double)pix);
			me->resize(int(t * pix), 0);
		});
	};

	scan.Event_On_Click = [](Button * me) {
		OPENFILENAME ofn;
		memset(&ofn, 0, sizeof(ofn));
		ofn.lStructSize = sizeof(ofn);
		ofn.hwndOwner = main.hWnd();
		ofn.lpstrFile = f;
		ofn.nMaxFile = MAX_PATH;
		ofn.lpstrFilter = "图片\0*.bmp;*.jpg;*.JPG;*.jpeg;*.png;*.gif\0\0";
		ofn.nFilterIndex = 0;
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
					info.name = (TCHAR*)ex.description.insert(0, "Failure: ").c_str();
					return;
				default:
					info.name = (TCHAR*)ex.description.insert(0, "Error: ").c_str();
					break;
				}
			}
			main.top();
		}
	};

	setting.Event_On_Click = [](Button * me) {
		progress = 0;
		notification = "Trian start. ";
		TrainMode();
		progress = 100;
		notification = "Train end. ";
	};

	main.Event_Load_Complete = [](form * me) {
		pix = me->w / 12;
		me->forAllControl([](control * me) {
			me->setFont("微软雅黑", 19); 
		});
	};

	home.Event_On_Click = [](Button * me) {
		scan.show();
	};
	Exit.Event_On_Click = [](Button * me) {
		window* p = me->parent();
		((form*)p)->close();
	};
	cut.Event_On_Check = [](Checkbox * me) {
		isCut = me->Value;
	};
	save.Event_On_Check = [](Checkbox * me) {
		savepic = me->Value;
	};

	main.run();
}