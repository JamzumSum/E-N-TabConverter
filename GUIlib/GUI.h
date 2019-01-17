#pragma once
#pragma  warning(disable : 4302)
#pragma  warning(disable : 4311)
#pragma  warning(disable : 4312)
#include <windows.h>
#include <assert.h>
#include <tchar.h>
#include <vector>
#include <string>
#include <algorithm>

#define Dsetter(na,ty) void set##na(ty na){\
this->##na = na;\
}\

#define Dgetter(na,ty) ty get##na(){\
return this->##na;\
}

#define Dgesetter(na,ty) ty na = NULL;\
Dgetter(na,ty)\
Dsetter(na,ty)

#define GWL_WNDPROC				(-4)
#define CLR_DEFAULT             0xFF000000L
#define PBS_SMOOTH              0x01
#define PBM_SETRANGE            (WM_USER+1)
#define PBM_SETPOS              (WM_USER+2)
#define PBM_SETSTEP             (WM_USER+4)
#define PBM_STEPIT              (WM_USER+5)
#define PBM_GETPOS              (WM_USER+8)
#define PBM_SETBARCOLOR         (WM_USER+9)

#define MAXSIZE 1024
#define ID_MENU 9001

static HINSTANCE hi = GetModuleHandle(0);
static LRESULT CALLBACK WinProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
void popError();
class control;
class form;

enum permission {
	readWrite, readOnly, writeOnly
};

template <typename Container, typename Value, enum permission prop>
class Property {
private:
	Container * me = NULL;
	void (Container::*Set)(Value value) = NULL;
	Value(Container::*Get)() = NULL;
	bool init = true;
public:
	Property() noexcept {}
	Property(bool initialed) noexcept {
		init = initialed;
	}
	void setContainer(Container* x) {
		me = x;
	}
	void setter(void (Container::*pSet)(Value value)) {
		Set = pSet;
	}
	void getter(Value(Container::*pGet)()) {
		if ((prop == readWrite) || (prop == readOnly)) Get = pGet;
	}
	Value operator= (const Value& value)
	{
		assert(me);
		assert(Set);
		init = true;
		if (prop == writeOnly || prop == readWrite) {
			(me->*Set)(value);
			return value;
		}
		else if (prop == readOnly && !init) {
			(me->*Set)(value);
		}
		return value;
	}
	operator Value() {
		assert(me);
		assert(Get);
		return (me->*Get)();
	}
};

class window {
private:
	TCHAR Name[MAXSIZE] = {};
	HMENU Menu = NULL;
	HWND Hwnd = NULL;
	LPTSTR Classname = NULL;
	window* Parent = NULL;

	void setName(LPTSTR newName) {
		if (Hwnd) SetWindowText(Hwnd, newName);
		_tcscpy_s(Name, newName);
	}
	LPTSTR getName() {
		GetWindowText(Hwnd, Name, MAXSIZE);
		return Name;
	}
	void setMenu(int ID) {
		Menu = LoadMenu(hi, MAKEINTRESOURCE(ID));
	}
	Dgetter(Menu, HMENU)

public:
	static std::vector<window*> formSet;

	int x = CW_USEDEFAULT;
	int y = CW_USEDEFAULT;
	int w = CW_USEDEFAULT;
	int h = CW_USEDEFAULT;

	char type = 0;
	size_t id = 0;				//生效的id > 0

	Property<window, LPTSTR, readWrite> name;
	Property<window, int, writeOnly> menu;

	long feature = 0;

	virtual std::vector<window*>& getContainer() = 0;

	window(LPTSTR classname, window* parent, int x = 0, int y = 0, int w = 0, int h = 0);
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
	void push() {
		std::vector<window*>& a = getContainer();
		a.push_back(this);
		id = a.size();
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

class form :public window {
private:
	//窝已经尽量私有了QAQ
	std::vector<LPTSTR> menuList;
	std::vector<void*> menuEventList;

	HMENU RBmenu = NULL;
	LPTSTR icon = IDI_APPLICATION;
	LPTSTR smallIcon = IDI_APPLICATION;

	HMENU CONTEXTMENU() { return this->RBmenu; }
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
	Property<form, HMENU, readOnly> RButtonMenu;
	std::vector<window*> tab;

	int xbias = 0;
	int ybias = 0;
	int brush = 0;
	
	HDC hdc = NULL;
	LPTSTR bitmapName = NULL;
	//方法

	void setIcon(LPTSTR smallIconName, LPTSTR iconName = NULL) {
		if (iconName) this->icon = iconName;
		smallIcon = smallIconName;
	}
	void pushMenu(void(*Event_Menu_Click)(), LPTSTR menu) {
		if (!this->RBmenu) RBmenu = CreatePopupMenu();
		if (AppendMenu(this->RBmenu, MF_STRING, ID_MENU + this->menuList.size(), menu)) {
			LPTSTR A = MAKEINTRESOURCE(ID_MENU + this->menuList.size());
			this->menuList.push_back(MAKEINTRESOURCE(ID_MENU + this->menuList.size()));
			this->menuEventList.push_back(Event_Menu_Click);
		}
	}
	void pushMenu(void(*Event_Menu_Click)(), int ID) {
		this->menuList.push_back(MAKEINTRESOURCE(ID));
		this->menuEventList.push_back(Event_Menu_Click);
	}
	void Event_Menu_Click(WORD ID) {
		for (size_t i = 0; i < menuList.size(); i++) if ((WORD)(ULONG_PTR)menuList[i] == ID) ((void(*)())menuEventList[i])();
	}
	void minimum() {
		ShowWindow(hWnd(), SW_SHOWMINNOACTIVE);
	}
	std::vector<window*>& getContainer() {
		return formSet;
	}
	void close() {
		PostMessage(hWnd(), WM_CLOSE, 0, 0);
	}
	void top() {
		SwitchToThisWindow(hWnd(), true);
	}
	void cls(RECT* area = NULL) {
		InvalidateRect(hWnd(), area, true);
	}

	size_t create();
	static form* getForm(HWND hWnd);
	void run();
	void paintLine(int x1, int y1, int x2, int y2);
	void paintLine(int x1, int y1, int x2, int y2, RECT* rect);
	control* getControl(HWND controlHwnd);
	//事件
	void(*Event_On_Create)(form*) = NULL;
	void(*Event_On_Unload)(form*) = NULL;
	void(*Event_Load_Complete)(form*) = NULL;
	void(*Event_Window_Resize)(form*) = NULL;
	void(*Event_On_Paint)(form*) = NULL;
};

class control :public window {				//继承类
private:

public:
	//属性
	std::string tag;
	//方法
	std::vector<window*>& getContainer() {
		return ((form*) parent())->tab;
	}
	control(LPTSTR clsname, window* parent, int x = 0, int y = 0, int w = 0, int h = 0);
	size_t operator()() {
		return this->create();
	}
};

class Button :public control {
public:
	Button(form* parent, int x, int y, int w, int h, const TCHAR* Name);
	void(*Event_On_Click)(Button*) = NULL;
};

class Label :public control {
public:

	Label(form* parent, int x, int y, int w, int h, const TCHAR* Name);
	size_t create() {
		control::create();
		setFont((TCHAR*)_T("宋体"), 14);
		return id;
	}
	void(*Event_On_Click)(Label*) = NULL;

	void setFont(LPTSTR fontName, int size);
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
public:
	Textbox(form* parent, int x, int y, int w, int h, const LPTSTR Name, bool Multiline = true);
	size_t create() {
		control::create();
		preProc = SetWindowLong(hWnd(), GWL_WNDPROC, (long)proc);
		return id;
	}
	WNDPROC proc = [](HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) -> LRESULT {
		Textbox * p = NULL;
		for (unsigned int i = 0; i < formSet.size(); i++) {
			auto a = find_if(formSet[i]->getContainer().begin(), formSet[i]->getContainer().end(), [hwnd](const void* x) -> bool {return ((control*)x)->type == 't' && ((control*)x)->hWnd() == hwnd; });
			if (a != formSet[i]->getContainer().end()) {
				p = (Textbox*)*a;
				break;
			}
		}
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
	void(*Event_Text_Change)(Textbox*) = NULL;
};

class ProgressBar :public control {
public:
	int step = 10;
	int range = 100;
	ProgressBar(form* parent, int x, int y, int w, int h, const LPTSTR Name);
	void stepIn() {
		SendMessage(hWnd(), PBM_STEPIT, 0, 0);
	}
	int setStep(int newStep = 0) {
		if (newStep) {
			SendMessage(hWnd(), PBM_SETSTEP, (WPARAM)newStep, 0);
			step = newStep;
		}
		return step;
	}
	int setRange(int newRange = 0) {
		if (newRange) {
			SendMessage(hWnd(), PBM_SETRANGE, 0, MAKELPARAM(0, newRange));
			range = newRange;
		}
		return range;
	}
	void setPos(int pos) {
		SendMessage(hWnd(), PBM_SETPOS, pos, 0);
	}
	void empty() {
		setPos(0);
	}
	void full() {
		setPos((int)SendMessage(hWnd(), PBM_GETPOS, 0, 0));
	}
	void setColor(ULONG color = CLR_DEFAULT) {
		//back to default if void
		SendMessage(hWnd(), PBM_SETBARCOLOR, 0, color);
	}
};

class Radio :public control {
private:
	bool getCheck() {
		return (int)SendMessage(hWnd(), BM_GETCHECK, 0, 0);
	}
	void setCheck(bool value) {
		SendMessage(hWnd(), BM_SETCHECK, (int)value, 0);
	}
public:
	//属性
	bool head = false;
	Property<Radio, bool, readWrite> Value;
	//方法
	Radio(form* parent, int x, int y, int w, int h, const LPTSTR Name, bool head = false);
	//事件
	void(*Event_On_Check)(Radio*) = NULL;
};

class Checkbox :public control {
private:
	bool getCheck() {
		return (int)SendMessage(hWnd(), BM_GETCHECK, 0, 0);
	}
	void setCheck(bool value) {
		SendMessage(hWnd(), BM_SETCHECK, (int)value, 0);
	}
public:
	Property<Checkbox, bool, readWrite> Value;
	Checkbox(form* parent, int x, int y, int w, int h, const TCHAR* Name);
	void(*Event_On_Check)(Checkbox*) = NULL;
};

class Timer : public control {
private:
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
	void(*Event_Timer)(form*) = NULL;

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

static void popError() {
	LPVOID lpMsgBuf;
	FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL, GetLastError(), MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR)&lpMsgBuf, 0, NULL);
	MessageBox(NULL, (LPCTSTR)lpMsgBuf, NULL, MB_OK);
	LocalFree(lpMsgBuf);
}