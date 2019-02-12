#pragma once
#pragma comment(linker,"/manifestdependency:\"type='win32' "\
"name='Microsoft.Windows.Common-Controls' version='6.0.0.0' "\
"processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

#include "prec.hpp"
#include <tchar.h>
#include <functional>
#include <string>
#include <algorithm>
#include <Commctrl.h>

#define GWL_WNDPROC -4
#define ID_MENU 9001

static windowSet fset;
static LRESULT CALLBACK WinProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);

typedef std::function<void(void)> vvEvent;

class Text {
private:
	HWND& hwnd;
	void setName(LPTSTR newName) {
		if (hwnd) SetWindowText(hwnd, newName);
		_tcscpy_s(Name, newName);
	}
	LPTSTR getName() {
		GetWindowText(hwnd, Name, MAXSIZE);
		return Name;
	}
protected:
	TCHAR Name[MAXSIZE] = {};
public:
	Property<Text, LPTSTR, readWrite> name;
	Text(HWND& hWnd) : hwnd(hWnd) {
		name.setContainer(this);
		name.setter(&Text::setName);
		name.getter(&Text::getName);
	}
	void setFont(const TCHAR* fontName, int size) {
		HFONT hFont = CreateFont(size, 0, 0, 0, FW_THIN, false, false, false,
			CHINESEBIG5_CHARSET, OUT_CHARACTER_PRECIS,
			CLIP_CHARACTER_PRECIS, DEFAULT_QUALITY,
			FF_MODERN, fontName);
		SendMessage(hwnd, WM_SETFONT, (WPARAM)hFont, TRUE);//发送设置字体消息
	}
};

class window: public Text {
	//window, 表示可见控件
protected:
	HMENU Menu = NULL;
	HWND Hwnd = NULL;
	window* Parent = NULL;
	LPTSTR Classname = NULL;
private:
	
public:

	int x = 0;
	int y = 0;
	int w = 0;
	int h = 0;

	char type = 0;
	size_t id = 0;				//生效的id > 0

	long feature = 0;

	window(char type, LPTSTR classname, window* parent, int x = 0, int y = 0, int w = 0, int h = 0);
	void hide() {
		ShowWindow(Hwnd, 0);
	}
	virtual void show() {
		ShowWindow(Hwnd, 1);
	}
	void resize(int w, int h) {
		if (w) this->w = w;
		if (h) this->h = h;
		MoveWindow(Hwnd, x, y, this->w, this->h, true);
	}
	void move(int x, int y) {
		if (x) this->x = x;
		if (y) this->y = y;
		MoveWindow(Hwnd, this->x, this->y, this->w, this->h, true);
	}
	window* parent() {
		return Parent;
	}
	const LPTSTR classname() {
		return Classname;
	}
	const HWND hWnd() {
		return Hwnd;
	}
	virtual size_t create();
};

class form: public window {
private:
	//窝已经尽量私有了QAQ

	void setMenu(int ID) {
		Menu = LoadMenu(hi, MAKEINTRESOURCE(ID));
	}

	std::vector<LPTSTR> menuList;
	std::vector<vvEvent> menuEventList;

	HMENU RBmenu = NULL;
	LPTSTR icon = IDI_APPLICATION;
	LPTSTR smallIcon = IDI_APPLICATION;

	bool regist();
	void Menulist_pushback(vvEvent Event_Menu_Click, size_t ID) {
		this->menuList.push_back(MAKEINTRESOURCE(ID));
		this->menuEventList.push_back(Event_Menu_Click);
	}
	//===================================================================================================
	//===================================================================================================
	//===================================================================================================
public:
	//构造
	form(form* parent, const TCHAR* className, const TCHAR* title, 
		int x = CW_USEDEFAULT, int y = CW_USEDEFAULT, int w = CW_USEDEFAULT, int h = CW_USEDEFAULT);
	~form();
	//属性
	windowSet tab;
	
	int brush = 0;
	HDC hdc = NULL;
	LPTSTR bitmapName = NULL;
	//方法

	LRESULT CALLBACK winproc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
	void setIcon(LPTSTR smallIconName, LPTSTR iconName = NULL) {
		if (iconName) this->icon = iconName;
		smallIcon = smallIconName;
	}
	void pushMenu(vvEvent Event_Menu_Click, LPTSTR menu) {
		if (!this->RBmenu) RBmenu = CreatePopupMenu();
		if (AppendMenu(this->RBmenu, MF_STRING, ID_MENU + this->menuList.size(), menu)) {
			Menulist_pushback(Event_Menu_Click, ID_MENU + this->menuList.size());
		}
	}
	void Event_Menu_Click(WORD ID) {
		auto r = find_if(menuList.begin(), menuList.end(), [ID](LPTSTR x) ->bool {return (WORD)(ULONG_PTR)x == ID; });
		assert(r != menuList.end());
		menuEventList[r - menuList.begin()]();
		//for (size_t i = 0; i < menuList.size(); i++) if ((WORD)(ULONG_PTR)menuList[i] == ID) menuEventList[i]();
	}
	void minimum() {
		ShowWindow(Hwnd, SW_SHOWMINNOACTIVE);
	}
	void close() {
		PostMessage(Hwnd, WM_CLOSE, 0, 0);
	}
	void top() {
		SwitchToThisWindow(Hwnd, true);
	}
	void cls(RECT* area = NULL) {
		InvalidateRect(Hwnd, area, true);
	}

	size_t create();
	void run();
	void paintLine(int x1, int y1, int x2, int y2);
	void paintLine(int x1, int y1, int x2, int y2, RECT* rect);
	control* getControl(HWND controlHwnd);
	void forAllControl(std::function<void(control*)>);
	//事件
	std::function<void(form*)> Event_On_Create = NULL;
	std::function<void(form*)> Event_On_Unload = NULL;
	std::function<void(form*)> Event_Load_Complete = NULL;
	std::function<void(form*)> Event_Window_Resize = NULL;
	std::function<void(form*)> Event_On_Paint = NULL;
};

class control :public window {				//继承类
private:
	
public:
	//属性
	std::string tag;
	//方法
	control(char type, LPTSTR clsname, window* parent, int x = 0, int y = 0, int w = 0, int h = 0);
	form* parent() {
		return (form*)Parent;
	}
	void push() {
		form* t = parent();
		assert(t);
		t->tab.add(this);
	}
};

class Button :public control{
public:
	Button(form* parent, int x, int y, int w, int h, const TCHAR* Name);
	std::function<void(Button*)> Event_On_Click = NULL;
};

class Label :public control {
public:

	Label(form* parent, int x, int y, int w, int h, const TCHAR* Name);
	std::function<void(Label*)> Event_On_Click = NULL;
};

class Picture :public control {
	//.bmp only
public:
	LPTSTR path = NULL;
	Picture(form* parent, int x, int y, int w, int h, const LPTSTR Name, const LPTSTR picPath);
	void(*Event_On_Click)(Picture*) = NULL;
};

class Textbox :public control {
private:
	long preProc = NULL;
	bool multiline = true;
	static LRESULT proc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) {
		Textbox * p = NULL;
		form* t = (form*)fset.find(GetParent(hwnd));
		if (!t) return 0;

		p = (Textbox*)t->tab.find(hwnd);
		if (!p) return 0;

		switch (message) {
		case WM_CHAR:
		case WM_PASTE:
		{
			//WM_CHANGE
			LRESULT r = CallWindowProc((WNDPROC)p->preProc, hwnd, message, wParam, lParam);
			if (p->Event_Text_Change) p->Event_Text_Change(p);
			return r;
		}
		default:
			return CallWindowProc((WNDPROC)p->preProc, hwnd, message, wParam, lParam);
		}
	};
public:
	Textbox(form* parent, int x, int y, int w, int h, const LPTSTR Name, bool Multiline = true);
	size_t create() {
		control::create();
		preProc = SetWindowLong(Hwnd, GWL_WNDPROC, (long)proc);
		return id;
	}
	
	std::function<void(Textbox*)> Event_Text_Change = NULL;
};

class ProgressBar :public control {
public:
	int step = 10;
	int range = 100;
	ProgressBar(form* parent, int x, int y, int w, int h, const LPTSTR Name);
	void stepIn() {
		SendMessage(Hwnd, PBM_STEPIT, 0, 0);
	}
	int setStep(int newStep = 0) {
		if (newStep) {
			SendMessage(Hwnd, PBM_SETSTEP, (WPARAM)newStep, 0);
			step = newStep;
		}
		return step;
	}
	int setRange(int newRange = 0) {
		if (newRange) {
			SendMessage(Hwnd, PBM_SETRANGE, 0, MAKELPARAM(0, newRange));
			range = newRange;
		}
		return range;
	}
	void setPos(int pos) {
		SendMessage(Hwnd, PBM_SETPOS, pos, 0);
	}
	void empty() {
		setPos(0);
	}
	void full() {
		setPos((int)SendMessage(Hwnd, PBM_GETPOS, 0, 0));
	}
	void setColor(ULONG color = CLR_DEFAULT) {
		//back to default if void
		SendMessage(Hwnd, PBM_SETBARCOLOR, 0, color);
	}
};

class Radio :public control {
private:
	bool getCheck() {
		return (int)SendMessage(Hwnd, BM_GETCHECK, 0, 0);
	}
	void setCheck(bool value) {
		SendMessage(Hwnd, BM_SETCHECK, (int)value, 0);
	}
public:
	//属性
	bool head = false;
	Property<Radio, bool, readWrite> Value;
	//方法
	Radio(form* parent, int x, int y, int w, int h, const LPTSTR Name, bool head = false);
	//事件
	std::function<void(Radio*)> Event_On_Check = NULL;
};

class Checkbox :public control {
private:
	bool getCheck() {
		return (int)SendMessage(Hwnd, BM_GETCHECK, 0, 0);
	}
	void setCheck(bool value) {
		SendMessage(Hwnd, BM_SETCHECK, (int)value, 0);
	}
public:
	//Property
	Property<Checkbox, bool, readWrite> Value;
	//Event
	std::function<void(Checkbox*)> Event_On_Check = NULL;
	//Function
	Checkbox(form* parent, int x, int y, int w, int h, const TCHAR* Name);
};

class Timer {
private:
	size_t id = 0;
	form* parent = NULL;
	bool value = false;
	void setTimer(bool value);
	bool getTimer() {
		return this->value;
	}
	void setInterval(UINT value);
	UINT getInterval() {
		return Interval;
	}
public:
	unsigned int Interval = 0;
	std::function<void(form*)> Event_Timer = NULL;

	Timer(form* parent, UINT interval, void(*Event)(form*), bool enabled);
	~Timer() {
		this->enabled = false;
	}
	size_t create() {
		return id;
	}
	void show() {
		return;
	}
	Property<Timer, bool, readWrite> enabled;
	Property<Timer, UINT, readWrite> interval;
};