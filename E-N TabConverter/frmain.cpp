/*
	frmain.cpp 控制程序的GUI
*/
#pragma once
#include "stdafx.h"
#include "type.h"
#include "GUI.hpp"
#include <commdlg.h>

using namespace std;

#define IDI_ICON1 101
#define IDI_WINDOW1 102
#define IDB_BITMAP1 106

static char prog[4];
extern bool savepic;
constexpr const char* PROJECT = "E-N TabConverter";

extern int go(string f, bool cut, function<void(string)>, function<void(int)>);
extern void TrainMode();

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR szCmdLine, int iCmdShow) {
	bool isCut = false;
	int pix = 80;
	char f[MAX_PATH] = {};
	auto conc = [](string n, char p[4]) -> string {
		return n + "------" + p + "%";
	};

	form main(NULL, "form", PROJECT, 240, 240, 840, 528);
	Button scan(&main, 5 * pix, 200, 112, 56, "Go!");
	Button home(&main, 8, 0, 112, 56, "Home");
	Button history(&main, 8, 64, 112, 56, "History");
	Button setting(&main, 8, 128, 112, 56, "Settings");
	Button Exit(&main, 8, 400, 112, 56, "Exit");
	Label info(&main, 8, 464, 560, 24, "Press \"Go\" to begin.");
	Checkbox save(&main, 760, 432, 56, 28, "save");
	Checkbox cut(&main, 760, 456, 56, 28, "cut");
	//control declare end

	main.setIcon(MAKEINTRESOURCE(IDI_WINDOW1), MAKEINTRESOURCE(IDI_ICON1));
	//main.bitmapName = MAKEINTRESOURCE(IDB_BITMAP1);
	main.Event_Window_Resize = [&scan, &pix, &main]() {
		pix = main.w / 12;
		scan.move(5 * pix, 0);
		main.forAllControl([&pix, &main](control* me) {
			double t = round(me->w / (double)pix);
			me->resize(int(t * pix), 0);
		});
	};

	scan.Event_On_Click = [&]() {
		OPENFILENAME ofn;
		memset(&ofn, 0, sizeof(OPENFILENAME));
		ofn.lStructSize = sizeof(OPENFILENAME);
		ofn.hwndOwner = main.hWnd();
		ofn.lpstrFile = f;
		ofn.nMaxFile = MAX_PATH;
		ofn.lpstrFilter = "图片\0*.bmp;*.jpg;*.JPG;*.jpeg;*.png;*.gif\0\0";
		ofn.nFilterIndex = 0;
		ofn.lpstrTitle = "选择乐谱：";
		ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;

		info.name = "Then choose a tab.";

		if (GetOpenFileName(&ofn)) {
			info.name = f;
			try {
				string noti;
				char prog[4];
				go(string(f), isCut, 
				[&noti, &prog, &conc, &info](string n) {
					noti = n;
					info.name = conc(noti, prog).c_str();
				}, 
				[&noti, &prog, &conc, &info](int p) {
					_itoa_s(p, prog, 10);
					info.name = conc(noti, prog).c_str();
				});
			}
			catch (Err ex) {
				switch (ex.id)
				{
				case 3:
					//不支持的格式
					info.name = ex.what();
					return;
				default:
					info.name = ex.what();
					break;
				}
			}
			main.top();
		}
		else info.name = "Press \"Go\" to begin.";
	};

	setting.Event_On_Click = [&info]() {
		info.name = "Train start. ";
		TrainMode();
		info.name = "Train end. ";
	};

	main.Event_Load_Complete = [&pix, &main]() {
		pix = main.w / 12;
		main.forAllControl([](control * me) {
			me->setFont("微软雅黑", 19); 
		});
	};

	home.Event_On_Click = [&scan]() {
		scan.show();
	};
	Exit.Event_On_Click = [&main]() {
		main.close();
	};
	cut.Event_On_Click = [&isCut, &cut]() {
		isCut = cut.Value;
	};
	save.Event_On_Click = [&save]() {
		savepic = save.Value;
	};
	main.run();
	main.msgLoop.join();
}