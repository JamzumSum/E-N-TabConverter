#pragma once
#pragma  warning(disable : 4302)
#pragma  warning(disable : 4311)
#pragma  warning(disable : 4312)
#include <windows.h>
#include <assert.h>
#include <vector>
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
	LPCSTR Name = NULL;
	HMENU Menu = NULL;
	HWND Hwnd = NULL;
	void setName(LPCSTR newName) {
		if (hWnd) SetWindowTextA(Hwnd, newName);
		Name = newName;
	}
	LPCSTR getName() {
		static char r[MAXSIZE];
		GetWindowTextA(Hwnd, r, MAXSIZE);
		Name = r;
		return Name;
	}
	void setMenu(int ID) {
		Menu = LoadMenu(hi, MAKEINTRESOURCE(ID));
	}
	Dgetter(Menu, HMENU)
		Dgetter(Hwnd, HWND)
		Dgesetter(Classname, LPCSTR)
		Dgesetter(Parent, void*)

public:
	int x = CW_USEDEFAULT;
	int y = CW_USEDEFAULT;
	int w = CW_USEDEFAULT;
	int h = CW_USEDEFAULT;
	char type = 0;
	size_t id = 0;

	window() noexcept {
		name.setContainer(this);
		hWnd.setContainer(this);
		classname.setContainer(this);
		parent.setContainer(this);
		menu.setContainer(this);
		name.setter(&window::setName);
		name.getter(&window::getName);
		hWnd.getter(&window::getHwnd);
		classname.setter(&window::setClassname);
		classname.getter(&window::getClassname);
		parent.setter(&window::setParent);
		parent.getter(&window::getParent);
		menu.setter(&window::setMenu);
	}
	Property<window, LPCSTR, readOnly, true> classname;
	Property<window, LPCSTR, readWrite, false> name;
	Property<window, HWND, readOnly, false> hWnd;
	Property<window, void*, readOnly, true> parent;
	Property<window, int, writeOnly, false> menu;

	long feature = 0;
	virtual std::vector<void*>& getContainer() = 0;
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
	virtual size_t create() {
		Hwnd = CreateWindowA(
			this->Classname, this->Name,
			this->feature,
			this->x, this->y, this->w, this->h,
			Parent ? ((window*)Parent)->Hwnd : NULL,
			Menu, hi, NULL
		);
		std::vector<void*> &a = getContainer();
		if (Hwnd) return id = a.size();
		else {
			a.pop_back();
			popError();
			return 0;
		}
	}
};

class form :public window {
private:
	//窝已经尽量私有了QAQ
	std::vector<LPCSTR> menuList;
	std::vector<void*> menuEventList;

	HMENU RBmenu = NULL;
	LPCSTR icon = IDI_APPLICATION;
	LPCSTR smallIcon = IDI_APPLICATION;

	bool createOver = false;
	HMENU CONTEXTMENU() { return this->RBmenu; }
	bool isCreated() { return this->createOver; };
public:
	//构造
	form() noexcept {}
	form(form* parent, const char* className, const char* title, int x = CW_USEDEFAULT, int y = CW_USEDEFAULT, int w = CW_USEDEFAULT, int h = CW_USEDEFAULT) {
		this->x = x; this->y = y; this->w = w; this->h = h;
		this->type = 'f';
		this->feature = WS_OVERLAPPEDWINDOW;
		this->name = title;
		this->classname = className;
		RButtonMenu.setContainer(this);
		MessageCreated.setContainer(this);
		formSet.push_back((void*)this);
		RButtonMenu.getter(&form::CONTEXTMENU);
		MessageCreated.getter(&form::isCreated);
		this->parent = parent;
	}
	~form() {
		if (parent) top();
		formSet.erase(find_if(formSet.begin(), formSet.end(), [this](void* x) {
			return (void*)this == x;
		}));
	}
	//属性
	Property<form, HMENU, readOnly, false> RButtonMenu;
	Property<form, bool, readOnly, false> MessageCreated;
	std::vector<void*> tab;

	int xbias = 0;
	int ybias = 0;
	int brush = 0;
	
	HDC hdc = NULL;
	LPCSTR bitmapName = NULL;
	//方法


	void setIcon(LPCSTR smallIconName, LPCSTR iconName = NULL) {
		if (iconName) this->icon = iconName;
		smallIcon = smallIconName;
	}
	void pushMenu(void(*Event_Menu_Click)(), char* menu) {
		if (!this->RBmenu) RBmenu = CreatePopupMenu();
		if (AppendMenu(this->RBmenu, MF_STRING, ID_MENU + this->menuList.size(), menu)) {
			LPCSTR A = MAKEINTRESOURCE(ID_MENU + this->menuList.size());
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

	bool regist() {
		WNDCLASSEXA wndclass;
		wndclass.cbSize = sizeof(wndclass);
		wndclass.style = CS_HREDRAW | CS_VREDRAW;
		wndclass.lpfnWndProc = WinProc;
		wndclass.cbClsExtra = 0;
		wndclass.cbWndExtra = 0;
		wndclass.hbrBackground = (HBRUSH)GetStockObject(brush);
		wndclass.hCursor = LoadCursorA(NULL, IDC_ARROW);
		//注意：此处下两句，MSDN中记载，如果是标准图标，第一个参数应该是NULL而不是hInstance. 此处尚未做处理，但程序运行通过，暂且放下
		wndclass.hIcon = LoadIconA(hi, this->icon);
		wndclass.hIconSm = LoadIconA(hi, this->smallIcon);
		wndclass.hInstance = hi;
		wndclass.lpszClassName = classname;
		wndclass.lpszMenuName = NULL;
		if (RegisterClassExA(&wndclass)) return true;
		else {
			DWORD x = GetLastError();
			if (x == 1410) return true;
			popError();
			return false;
		}
	}

	std::vector<void*>& getContainer() {
		return formSet;
	}
	size_t create() {
		if (!regist()) return 0;
		window::create();
		if (x < 0) {
			RECT r;
			GetWindowRect(hWnd, &r);
			x = r.left;
			y = r.top;
			w = r.right - x;
			h = r.bottom - y;
		}
		if (this->Event_On_Create) this->Event_On_Create(this);
		return id;
	}

	unsigned long long run();
	void close() {
		PostMessage(hWnd, WM_CLOSE, 0, 0);
	}
	void top() {
		SwitchToThisWindow(hWnd, true);
	}
	void cls(RECT* area = NULL) {
		InvalidateRect(hWnd, area, true);
	}
	void paintLine(int x1, int y1, int x2, int y2) {
		//if x1 = x2 = y1 = y2 = 0 then end paint.
		PAINTSTRUCT ps;
		hdc = BeginPaint(hWnd, &ps);
		MoveToEx(hdc, x1, y1, NULL);
		LineTo(hdc, x2, y2);
		EndPaint(hWnd, &ps);
		hdc = NULL;
	}
	void paintLine(int x1, int y1, int x2, int y2, RECT* rect) {
		//if x1 = x2 = y1 = y2 = 0 then end paint.
		static PAINTSTRUCT ps;
		if (!hdc) {
			hdc = BeginPaint(hWnd, &ps);
		}
		if (x1 || x2 || y1 || y2) {
			MoveToEx(hdc, x1, y1, NULL);
			LineTo(hdc, x2, y2);
		}
		else {
			EndPaint(hWnd, &ps);
			hdc = NULL;
			UpdateWindow(hWnd);
		}
	}
	void* getControl(HWND controlHwnd);
	//事件
	void(*Event_On_Create)(form*) = NULL;
	void(*Event_On_Unload)(form*) = NULL;
	void(*Event_Load_Complete)(form*) = NULL;
	void(*Event_Window_Resize)(form*) = NULL;
	void(*Event_On_Paint)(form*) = NULL;
	//运算符重载
	unsigned long long operator()() {
		return run();
	}
};

class control :public window {				//继承类
private:

public:
	//属性
	std::string tag;
	//方法
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
	button(form* parent, int x, int y, int w, int h, const char* Name) {
		this->x = x;
		this->y = y;
		this->w = w;
		this->h = h;
		this->type = 'b';
		this->parent = parent;
		this->name = Name;
		this->classname = "BUTTON";
		this->feature |= BS_DEFPUSHBUTTON;
		push();
	}
	void(*Event_On_Click)(button*) = NULL;
};



class Label :public control {
public:

	Label() noexcept {}
	Label(form* parent, int x, int y, int w, int h, const char* Name) {
		this->x = x;
		this->y = y;
		this->w = w;
		this->h = h;
		this->type = 'l';
		this->parent = parent;
		this->name = Name;
		this->classname = "STATIC";
		this->feature |= SS_NOTIFY | BS_FLAT;
		push();
	}
	size_t create() {
		control::create();
		setFont("宋体", 14);
		return id;
	}
	void(*Event_On_Click)(Label*) = NULL;

	void setFont(LPCSTR fontName, int size) {
		HFONT hFont = CreateFont(size, 0, 0, 0, FW_THIN, false, false, false,
			CHINESEBIG5_CHARSET, OUT_CHARACTER_PRECIS,
			CLIP_CHARACTER_PRECIS, DEFAULT_QUALITY,
			FF_MODERN, fontName);
		SendMessage(this->hWnd, WM_SETFONT, (WPARAM)hFont, TRUE);//发送设置字体消息
	}
};



class Picture :public control {
	//.bmp only
public:
	LPCSTR path = NULL;
	Picture() noexcept {}
	Picture(form* parent, int x, int y, int w, int h, const char* Name, char* picPath) {
		this->x = x;
		this->y = y;
		this->w = w;
		this->h = h;
		this->type = 'p';
		this->parent = parent;
		this->name = Name;				//x
		this->path = picPath;
		this->classname = "STATIC";
		this->feature |= SS_NOTIFY | SS_BITMAP;
		push();
	}
	void(*Event_On_Click)(Picture*) = NULL;
};

class Textbox :public control {
private:
	long preProc = NULL;
	bool multiline = true;
public:
	Textbox() noexcept {}
	Textbox(form* parent, int x, int y, int w, int h, const char* Name, bool Multiline = true) {
		this->x = x;
		this->y = y;
		this->w = w;
		this->h = h;
		this->type = 't';
		this->parent = parent;
		this->name = Name;
		this->multiline = Multiline;
		this->classname = "Edit";
		this->feature |= WS_BORDER | WS_GROUP | WS_TABSTOP | ES_WANTRETURN;
		if (Multiline) this->feature |= ES_MULTILINE;
		push();
	}
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
	ProgressBar(form* parent, int x, int y, int w, int h, const char* Name) {
		this->x = x;
		this->y = y;
		this->w = w;
		this->h = h;
		this->type = 'P';
		this->parent = parent;
		this->name = Name;				//x
		this->classname = "msctls_progress32";
		this->feature |= PBS_SMOOTH;
		push();
	}
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
	//属性
	bool head = false;
	Property<radio, bool, readWrite, false> Value;
	//方法
	radio() noexcept {}
	radio(form* parent, int x, int y, int w, int h, const char* Name, bool head = false) {
		this->x = x;
		this->y = y;
		this->w = w;
		this->h = h;
		this->head = head;
		this->type = 'r';
		this->parent = parent;
		this->name = Name;				//x
		this->classname = "BUTTON";
		this->feature |= BS_AUTORADIOBUTTON;
		if (head) this->feature |= WS_GROUP;

		Value.setContainer(this);
		Value.setter(&radio::setCheck);
		Value.getter(&radio::getCheck);
		push();
	}
	//事件
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
	Checkbox(form* parent, int x, int y, int w, int h, const char* Name) {
		this->x = x;
		this->y = y;
		this->w = w;
		this->h = h;
		this->type = 'c';
		this->parent = parent;
		this->name = Name;				//x
		this->classname = "BUTTON";
		this->feature |= BS_AUTOCHECKBOX;
		Value.setContainer(this);
		Value.setter(&Checkbox::setCheck);
		Value.getter(&Checkbox::getCheck);
		push();
	}
	void(*Event_On_Check)(Checkbox*) = NULL;
};

class timer : public control {
private:
	bool value = false;
	void setTimer(bool value) {
		if (value ^ this->value) this->value = value;
		else return;
		if (value) {
			if (Event_Timer) {
				assert(Event_Timer);
				void* t = parent;
				if (!SetTimer(((form*)t)->hWnd, id, Interval, NULL)) {
					std::string des = "Fail to set timer";
					MessageBox(NULL, des.c_str(), (LPCSTR)(ULONG_PTR)GetLastError(), MB_OK);
					this->value = false;
				}
			}
		}
		else {
			void* t = parent;
			KillTimer(((form*)t)->hWnd, id);
		}
	}
	bool getTimer() {
		return this->value;
	}
	void setInterval(UINT value) {
		if (value != Interval) {
			bool r = enabled;
			enabled = false;
			Interval = value;
			setTimer(r);
		}
	}
	UINT getInterval() {
		return Interval;
	}
public:
	unsigned int Interval = 0;
	void(*Event_Timer)(form*) = NULL;

	timer(form* parent, UINT interval, void(*Event)(form*), bool enabled = false) {
		this->Event_Timer = Event;
		this->Interval = interval;
		this->parent = parent;
		this->enabled.setContainer(this);
		this->interval.setContainer(this);
		this->enabled.setter(&timer::setTimer);
		this->enabled.getter(&timer::getTimer);
		this->interval.setter(&timer::setInterval);
		this->interval.getter(&timer::getInterval);
		if (enabled) this->enabled = true;

		push();
	}
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

static LRESULT CALLBACK WinProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	form* t = (form*)getForm(hwnd);
	if (!t) return DefWindowProc(hwnd, message, wParam, lParam);
	switch (message)
	{
	case WM_CREATE:
		//创建事件的思路还没有头绪。。暂时丢到CreateWindow后面去
		break;
	case WM_PAINT:
		if (t->bitmapName) {
			HBITMAP hbm;
			BITMAP bminfo;
			hbm = LoadBitmap(GetModuleHandle(NULL), t->bitmapName);
			GetObject(hbm, sizeof(bminfo), &bminfo);
			PAINTSTRUCT ps;
			t->hdc = BeginPaint(hwnd, &ps);
			HDC memdc = CreateCompatibleDC(t->hdc);
			SelectObject(memdc, hbm);
			BitBlt(t->hdc, 0, 0, bminfo.bmWidth, bminfo.bmHeight, memdc, 0, 0, SRCCOPY);
			DeleteDC(memdc);
			EndPaint(hwnd, &ps);
		}
		if (t->Event_On_Paint) t->Event_On_Paint(t);
		break;
	case WM_CONTEXTMENU:
	{
		RECT rect;
		POINT pt;
		pt.x = LOWORD(lParam);
		pt.y = HIWORD(lParam);
		GetClientRect((HWND)wParam, &rect);
		//把屏幕坐标转为客户区坐标  
		ScreenToClient((HWND)wParam, &pt);
		if (PtInRect(&rect, pt))
			if (!TrackPopupMenu(t->RButtonMenu, TPM_LEFTALIGN | TPM_TOPALIGN | TPM_LEFTBUTTON, LOWORD(lParam), HIWORD(lParam), 0, (HWND)wParam, NULL)) {
				if (t->RButtonMenu) MessageBoxA(NULL, TEXT("弹出菜单失败"), NULL, MB_OK);
			}
			else return DefWindowProc(hwnd, message, wParam, lParam);
		break;
	}
	case WM_TIMER:
		((timer*)t->tab[wParam - 1])->Event_Timer(t);
		break;
	case WM_SIZE:
	{
		t->w = LOWORD(lParam);
		t->h = HIWORD(lParam);
		if (t->MessageCreated) if (t->Event_Window_Resize) t->Event_Window_Resize(t);
		break;
	}
	case WM_MOVE:
		t->x = LOWORD(lParam);
		t->y = HIWORD(lParam);
		break;
	case WM_COMMAND:
		if (lParam)
		{
			//这是控件点击事件
			void* p = t->getControl((HWND)lParam);
			if (p) switch (((control*)p)->type) {
			case 'b':
				if (((button*)p)->Event_On_Click) ((button*)p)->Event_On_Click((button*)p);
				break;
			case 'l':
				if (((Label*)p)->Event_On_Click) ((Label*)p)->Event_On_Click((Label*)p);
				break;
			case 'p':
				if (((Picture*)p)->Event_On_Click) ((Picture*)p)->Event_On_Click((Picture*)p);
				break;
			case 'r':
				if (((radio*)p)->Event_On_Check) ((radio*)p)->Event_On_Check((radio*)p);
				break;
			case 'c':
				if (((Checkbox*)p)->Event_On_Check) ((Checkbox*)p)->Event_On_Check((Checkbox*)p);
				break;
			}
		}
		else {
			t->Event_Menu_Click(LOWORD(wParam));
		}
		break;
	case WM_CTLCOLORSTATIC://拦截WM_CTLCOLORSTATIC消息
		SetBkMode((HDC)wParam, TRANSPARENT);//设置背景透明
		if (t) return (INT_PTR)GetStockObject(t->brush);//返回父窗画刷
		break;
	case WM_CLOSE:
		if (t->Event_On_Unload) t->Event_On_Unload(t);
		DestroyWindow(hwnd);
		PostQuitMessage(0);
		break;
	case WM_CHAR:
		break;
	default:
		return DefWindowProc(hwnd, message, wParam, lParam);
	}
	return DefWindowProc(hwnd, message, wParam, lParam);
}

inline void* form::getControl(HWND controlHwnd) {
	std::vector<void*>::iterator r = find_if(tab.begin(), tab.end(), [controlHwnd](const void* x) -> bool { return ((control*)x)->hWnd == controlHwnd; });
	return r == tab.end() ? nullptr : *r;
}

inline unsigned long long form::run() {				//在此处主程序挂起
	for (void* p : tab) {
		((control*)p)->create();
		((control*)p)->show();
	}
	MSG msg;
	ShowWindow(hWnd, SW_SHOW);
	UpdateWindow(hWnd);
	ZeroMemory(&msg, sizeof(msg));
	this->createOver = true;						//消息队列建立		//我想了两天终于下决心把这个SB东西和下边那句话调了过来		//上帝保佑我别再出bug了		//再写bug我学分就都没了
	if (this->Event_Load_Complete) this->Event_Load_Complete(this);
	while (GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	UnregisterClassA(classname, hi);
	return msg.wParam;
}

static void popError() {
	LPVOID lpMsgBuf;
	FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL, GetLastError(), MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR)&lpMsgBuf, 0, NULL);
	MessageBox(NULL, (LPCTSTR)lpMsgBuf, NULL, MB_OK);
	LocalFree(lpMsgBuf);
}