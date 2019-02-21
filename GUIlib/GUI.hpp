#pragma once
#pragma comment(linker,"/manifestdependency:\"type='win32' "\
"name='Microsoft.Windows.Common-Controls' version='6.0.0.0' "\
"processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

#include "prec.hpp"
#include "Event.h"
#include <tchar.h>
#include <string>
#include <algorithm>
#include <thread>
#include <Commctrl.h>

#define GWL_WNDPROC -4
#define ID_MENU 9001

static windowSet fset;
static LRESULT CALLBACK WinProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);


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

class window: public Text, public Event {
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

	window(char type, const TCHAR* classname, window* parent, int x = 0, int y = 0, int w = 0, int h = 0);
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

class menuX {
	union menuid {
		HMENU menu = NULL;
		UINT id;
	};
private:
	menuid id;
	vvEvent On_Click = NULL;
	std::string name;
public:
	menuX() {}
	menuX(HWND owner, bool pop) {
		id.menu = pop ? CreatePopupMenu() : CreateMenu();
		SetMenu(owner, id.menu);
	}
	menuX(menuX parent, std::string name) : name(name) {
		id.menu = CreateMenu();
		AppendMenu(parent, MF_POPUP, (LONG)id.menu, name.c_str());
	}
	menuX(menuX parent, std::string name, UINT id, vvEvent click) : name(name), On_Click(click) {
		this->id.id = id;
		AppendMenu(parent, MF_STRING, id, name.c_str());
	}
	operator HMENU() {
		return id.menu;
	}
	operator UINT() {
		return id.id;
	}
	operator std::string() {
		return name;
	}
	auto operator() () {
		assert(On_Click);
		return On_Click();
	}
};

class form: public window {
private:
	//窝已经尽量私有了QAQ
	LPTSTR icon = IDI_APPLICATION;
	LPTSTR smallIcon = IDI_APPLICATION;
	bool created = false;
	std::vector<menuX> menuleaves;

	bool regist();
	//===================================================================================================
	//===================================================================================================
	//===================================================================================================
public:
	//构造
	form(form* parent, const TCHAR* className, const TCHAR* title, 
		int x = CW_USEDEFAULT, int y = CW_USEDEFAULT, int w = CW_USEDEFAULT, int h = CW_USEDEFAULT);
	~form();
	//属性
	std::thread msgLoop;
	menuX rootMenu = menuX(Hwnd, false);
	menuX RBmenu;
	windowSet tab;
	
	int brush = 0;
	HDC hdc = NULL;
	LPTSTR bitmapName = NULL;
	//message queqe
	LRESULT CALLBACK winproc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
	//icon
	void setIcon(LPTSTR smallIconName, LPTSTR iconName = NULL) {
		assert(!created);
		if (iconName) this->icon = iconName;
		smallIcon = smallIconName;
	}
	//menu
	menuX createMenu(menuX parent, std::string name, vvEvent Event_Menu_Click = NULL) {
		static UINT id = 1001;
		if (Event_Menu_Click) {
			menuX newm(parent, name, id++, Event_Menu_Click);
			menuleaves.emplace_back(newm);
			return newm;
		}
		else return menuX(parent, name);
	}
	void Event_Menu_Click(WORD ID) {
		auto r = find_if(menuleaves.begin(), menuleaves.end(), [ID](menuX x) ->bool {return (UINT)x == ID; });
		assert(r != menuleaves.end());
		(*r)();
	}
	void enableRBmenu() {
		RBmenu = menuX(Hwnd, true);
	}
	//form operation
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
	const bool isCreated() { return created; }
	void run();
	void paintLine(int x1, int y1, int x2, int y2);
	void paintLine(int x1, int y1, int x2, int y2, RECT* rect);
	control* getControl(HWND controlHwnd);
	void forAllControl(std::function<void(control*)>);
	//事件
	vvEvent Event_On_Create = NULL;
	vvEvent Event_On_Unload = NULL;
	vvEvent Event_Load_Complete = NULL;
	vvEvent Event_Window_Resize = NULL;
	vvEvent Event_On_Paint = NULL;
};

class control: public window {				//继承类
private:
	
public:
	//属性
	std::string tag;
	//方法
	control(char type, const TCHAR* clsname, window* parent, int x = 0, int y = 0, int w = 0, int h = 0);
	form* parent() {
		return (form*)window::Parent;
	}
	void push() {
		form* t = parent();
		assert(t);
		t->tab.add(this);
	}
	size_t create() {
		assert(parent()->isCreated());
		window::create();
		return id;
	}
};

class Button: public control, public Clickable{
public:
	Button(form* parent, int x, int y, int w, int h, const TCHAR* Name);
};

class Label: public control, public Clickable {
public:

	Label(form* parent, int x, int y, int w, int h, const TCHAR* Name);
};

class Picture: public control, public Clickable {
	//.bmp only
public:
	Picture(form* parent, int x, int y, int w, int h, const TCHAR* picPath);
};

class Textbox :public control {
private:
	long preProc = NULL;
	bool multiline = true;
	static LRESULT proc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
public:
	Textbox(form* parent, int x, int y, int w, int h, const LPTSTR Name, bool Multiline = true);
	size_t create() {
		control::create();
		preProc = SetWindowLong(Hwnd, GWL_WNDPROC, (long)proc);
		return id;
	}
	
	std::function<void()> Event_Text_Change = NULL;
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
		setPos((int)SendMessage(Hwnd, PBM_GETRANGE, false, NULL));
	}
	void setColor(ULONG color = CLR_DEFAULT) {
		//back to default if void
		SendMessage(Hwnd, PBM_SETBARCOLOR, 0, color);
	}
};

class Radio: public control, public Clickable {
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
};

class Checkbox: public control, public Clickable {
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
	vvEvent Event_Timer = NULL;

	Timer(form* parent, UINT interval, vvEvent Event, bool enabled);
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