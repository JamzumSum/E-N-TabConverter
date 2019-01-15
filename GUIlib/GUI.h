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

static std::vector<void*> formSet;

enum permission {
	readWrite, readOnly, writeOnly
};

template <typename Container, typename Value, enum permission prop, bool permissive>
class Property {
private:
	Container * me = NULL;
	void (Container::*Set)(Value value) = NULL;
	Value(Container::*Get)() = NULL;
	bool flag = true;
public:
	Property() noexcept {}
	void setContainer(Container* x)
	{
		me = x;
	}
	void setter(void (Container::*pSet)(Value value))
	{
		if (permissive) {
			Set = pSet;
			return;
		}
		if ((prop == writeOnly) || (prop == readWrite)) Set = pSet;

	}
	void getter(Value(Container::*pGet)())
	{
		if ((prop == readWrite) || (prop == readOnly)) Get = pGet;
	}
	Value operator =(const Value& value)
	{
		assert(me);
		assert(Set);
		if (prop == writeOnly || prop == readWrite) {
			(me->*Set)(value);
			return value;
		}
		if (prop == readOnly && flag) {
			(me->*Set)(value);
			if (value) flag = false;
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
	LPTSTR Name = NULL;
	HMENU Menu = NULL;
	HWND Hwnd = NULL;
	void setName(LPTSTR newName) {
		if (hWnd) SetWindowText(Hwnd, newName);
		Name = newName;
	}
	LPTSTR getName() {
		GetWindowText(Hwnd, Name, MAXSIZE);
		return Name;
	}
	void setMenu(int ID) {
		Menu = LoadMenu(hi, MAKEINTRESOURCE(ID));
	}
	Dgetter(Menu, HMENU)
	Dgetter(Hwnd, HWND)
	Dgesetter(Classname, LPTSTR)
	Dgesetter(Parent, void*)

public:
	int x = CW_USEDEFAULT;
	int y = CW_USEDEFAULT;
	int w = CW_USEDEFAULT;
	int h = CW_USEDEFAULT;
	char type = 0;
	size_t id = 0;

	Property<window, LPTSTR, readOnly, true> classname;
	Property<window, LPTSTR, readWrite, false> name;
	Property<window, HWND, readOnly, false> hWnd;
	Property<window, void*, readOnly, true> parent;
	Property<window, int, writeOnly, false> menu;

	long feature = 0;

	virtual std::vector<void*>& getContainer() = 0;
	window() noexcept;
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
	virtual size_t create();
};

class form :public window {
private:
	//���Ѿ�����˽����QAQ
	std::vector<LPTSTR> menuList;
	std::vector<void*> menuEventList;

	HMENU RBmenu = NULL;
	LPTSTR icon = IDI_APPLICATION;
	LPTSTR smallIcon = IDI_APPLICATION;

	bool createOver = false;
	HMENU CONTEXTMENU() { return this->RBmenu; }
	bool isCreated() { return this->createOver; };
	bool regist();
public:
	//����
	form() noexcept {}
	form(form* parent, TCHAR* className, TCHAR* title, 
		int x = CW_USEDEFAULT, int y = CW_USEDEFAULT, int w = CW_USEDEFAULT, int h = CW_USEDEFAULT);
	~form();
	//����
	Property<form, HMENU, readOnly, false> RButtonMenu;
	Property<form, bool, readOnly, false> MessageCreated;
	std::vector<void*> tab;

	int xbias = 0;
	int ybias = 0;
	int brush = 0;
	
	HDC hdc = NULL;
	LPTSTR bitmapName = NULL;
	//����


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
		ShowWindow(hWnd, SW_SHOWMINNOACTIVE);
	}

	std::vector<void*>& getContainer() {
		return formSet;
	}

	size_t create();
	WPARAM run();

	void close() {
		PostMessage(hWnd, WM_CLOSE, 0, 0);
	}
	void top() {
		SwitchToThisWindow(hWnd, true);
	}
	void cls(RECT* area = NULL) {
		InvalidateRect(hWnd, area, true);
	}
	void paintLine(int x1, int y1, int x2, int y2);
	void paintLine(int x1, int y1, int x2, int y2, RECT* rect);
	void* getControl(HWND controlHwnd);
	//�¼�
	void(*Event_On_Create)(form*) = NULL;
	void(*Event_On_Unload)(form*) = NULL;
	void(*Event_Load_Complete)(form*) = NULL;
	void(*Event_Window_Resize)(form*) = NULL;
	void(*Event_On_Paint)(form*) = NULL;
};

class control :public window {				//�̳���
private:

public:
	//����
	std::string tag;
	//����
	std::vector<void*>& getContainer() {
		void* t = parent;
		return ((form*)t)->tab;
	}
	control() noexcept {
		this->feature = this->feature | WS_CHILD | WS_VISIBLE;
	}
	void push() {
		std::vector<void*>& a = getContainer();
		a.push_back(this);
		id = a.size();
	}
};

class button :public control {
public:
	button() noexcept {}
	button(form* parent, int x, int y, int w, int h, TCHAR* Name);
	void(*Event_On_Click)(button*) = NULL;
};



class Label :public control {
public:

	Label() noexcept {}
	Label(form* parent, int x, int y, int w, int h, LPTSTR Name);
	size_t create() {
		control::create();
		setFont((TCHAR*)_T("����"), 14);
		return id;
	}
	void(*Event_On_Click)(Label*) = NULL;

	void setFont(LPTSTR fontName, int size);
};



class Picture :public control {
	//.bmp only
public:
	LPTSTR path = NULL;
	Picture() noexcept {}
	Picture(form* parent, int x, int y, int w, int h, TCHAR* Name, TCHAR* picPath);
	void(*Event_On_Click)(Picture*) = NULL;
};

class Textbox :public control {
private:
	long preProc = NULL;
	bool multiline = true;
public:
	Textbox() noexcept {}
	Textbox(form* parent, int x, int y, int w, int h, TCHAR* Name, bool Multiline = true);
	size_t create() {
		control::create();
		preProc = SetWindowLong(hWnd, GWL_WNDPROC, (long)proc);
		return id;
	}
	WNDPROC proc = [](HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) -> LRESULT {
		Textbox * p = NULL;
		for (unsigned int i = 0; i < formSet.size(); i++) {
			auto a = find_if(((form*)formSet[i])->tab.begin(), ((form*)formSet[i])->tab.end(), [hwnd](const void* x) -> bool {return ((control*)x)->type == 't' && ((control*)x)->hWnd == hwnd; });
			if (a != ((form*)formSet[i])->tab.end()) {
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
	ProgressBar() noexcept {}
	ProgressBar(form* parent, int x, int y, int w, int h, TCHAR* Name);
	void stepIn() {
		SendMessage(hWnd, PBM_STEPIT, 0, 0);
	}
	int setStep(int newStep = 0) {
		if (newStep) {
			SendMessage(hWnd, PBM_SETSTEP, (WPARAM)newStep, 0);
			step = newStep;
		}
		return step;
	}
	int setRange(int newRange = 0) {
		if (newRange) {
			SendMessage(hWnd, PBM_SETRANGE, 0, MAKELPARAM(0, newRange));
			range = newRange;
		}
		return range;
	}
	void setPos(int pos) {
		SendMessage(hWnd, PBM_SETPOS, pos, 0);
	}
	void empty() {
		setPos(0);
	}
	void full() {
		setPos((int)SendMessage(hWnd, PBM_GETPOS, 0, 0));
	}
	void setColor(ULONG color = CLR_DEFAULT) {
		//back to default if void
		SendMessage(hWnd, PBM_SETBARCOLOR, 0, color);
	}
};

class radio :public control {
private:
	bool getCheck() {
		return (int)SendMessage(hWnd, BM_GETCHECK, 0, 0);
	}
	void setCheck(bool value) {
		SendMessage(hWnd, BM_SETCHECK, (int)value, 0);
	}
public:
	//����
	bool head = false;
	Property<radio, bool, readWrite, false> Value;
	//����
	radio() noexcept {}
	radio(form* parent, int x, int y, int w, int h, TCHAR* Name, bool head = false);
	//�¼�
	void(*Event_On_Check)(radio*) = NULL;
};

class Checkbox :public control {
private:
	bool getCheck() {
		return (int)SendMessage(hWnd, BM_GETCHECK, 0, 0);
	}
	void setCheck(bool value) {
		SendMessage(hWnd, BM_SETCHECK, (int)value, 0);
	}
public:
	Property<Checkbox, bool, readWrite, false> Value;
	Checkbox() noexcept {}
	Checkbox(form* parent, int x, int y, int w, int h, TCHAR* Name);
	void(*Event_On_Check)(Checkbox*) = NULL;
};

class timer : public control {
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

	timer(form* parent, UINT interval, void(*Event)(form*), bool enabled);
	~timer() {
		this->enabled = false;
	}
	size_t create() {
		return id;
	}
	void show() {
		return;
	}
	Property<timer, bool, readWrite, false> enabled;
	Property<timer, UINT, readWrite, false> interval;
};

static auto getForm = [](HWND hWnd) -> void* {
	std::vector<void*>::iterator r = find_if(formSet.begin(), formSet.end(), [hWnd](const void* x) -> bool {
		return ((form*)x)->hWnd == hWnd;
	});
	return r == formSet.end() ? nullptr : *r;
};

static void popError() {
	LPVOID lpMsgBuf;
	FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL, GetLastError(), MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR)&lpMsgBuf, 0, NULL);
	MessageBox(NULL, (LPCTSTR)lpMsgBuf, NULL, MB_OK);
	LocalFree(lpMsgBuf);
}