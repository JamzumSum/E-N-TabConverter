#include"GUI.h"
#define IDI_ICON1 101
#define IDI_WINDOW1 102
#define IDB_BITMAP1 106
form main("form", "E-Land Chord Converter",240,240,840,528);
int pix = 80;
char f[MAX_PATH];

button scan(&main, 5 * pix, 200, 112, 56, "Go!");
Label info(&main, 0, 464, 560, 24, "");
extern int go(std::string f,bool);
bool isCut = false;

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR szCmdLine, int iCmdShow) {
	main.setIcon(MAKEINTRESOURCE(IDI_WINDOW1), MAKEINTRESOURCE(IDI_ICON1));
	//main.bitmapName = MAKEINTRESOURCE(IDB_BITMAP1);
	main.create();
	main.Event_Window_Resize = [](form* me) {
		pix = me->w / 12;
	};
	
	scan.Event_On_Click = [](button* me) {
		OPENFILENAME ofn;
		ZeroMemory(&ofn, sizeof(ofn));
		ofn.lStructSize = sizeof(ofn);
		ofn.hwndOwner = main.hWnd;
		ofn.lpstrDefExt = 0;
		ofn.lpstrFile = f;
		ofn.lpstrFile[0] = '\0';
		ofn.nMaxFile = MAX_PATH;
		ofn.lpstrFilter = "Í¼Æ¬ÎÄ¼ş\0*.bmp;*.jpg;*.JPG;*.jpeg;*.png;*.gif\0\0";
		ofn.nFilterIndex = 0;
		ofn.lpstrInitialDir = 0;
		ofn.lpstrTitle = "Ñ¡ÔñÀÖÆ×£º";
		ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
		if (GetOpenFileName(&ofn))
		{
			info.name = f;
			if (go(std::string(f),isCut) == 0) {
				info.name = "success";
			}
			else {
				info.name = "failure";
			}
			main.show();
		}
	};
	main.Event_Load_Complete = [](form* me) {
		pix = me->w / 12;
	};
	button home(&main, 8, 0, 112, 56, "Home");
	button history(&main, 8, 64, 112, 56, "History");
	button setting(&main, 8, 128, 112, 56, "Settings");
	button exit(&main, 8, 400, 112, 56, "Exit");
	Checkbox cut(&main,760,450,56,32,"Cut");

	home.Event_On_Click = [](button* me) {
		scan.show();
	};
	exit.Event_On_Click = [](button* me) {
		void* p = me->parent;
		((form*)p)->close();
	};
	cut.Event_On_Check = [](Checkbox* me) {
		isCut = me->Value;
	};
	scan.create();
	home.create();
	history.create();
	setting.create();
	exit.create();
	info.create();
	cut.create();

	main();
}