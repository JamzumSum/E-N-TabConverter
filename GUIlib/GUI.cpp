#pragma once
#include "stdafx.h"

using namespace std;

void form::run() {
	auto loop = [this]() {
		MSG msg;
		memset(&msg, 0, sizeof(msg));

		create();
		forAllControl([](control * me) {me->create(); });

		if (this->Event_Load_Complete) this->Event_Load_Complete();

		show();

		int loopret = 0;
		while ((loopret = GetMessage(&msg, this->Hwnd, 0, 0)) > 0) {
			if (loopret == -1) {
				popError();
				exit(1);
			}
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		fset.remove(id);
		window* w = fset.find(Classname);
		if (!w) UnregisterClass(Classname, hi);
	};
	
	msgLoop = thread(loop);
}

control* form::getControl(HWND controlHwnd) {
	window* r = tab.find(controlHwnd);
	return (control*)r;
}

static LRESULT CALLBACK WinProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam){

	form* t = NULL;
	switch (message) {
	case WM_CREATE:
		t = (form*)(((LPCREATESTRUCT)lParam)->lpCreateParams);
		fset.add(t);
		break;
	default: t = (form*)fset.find(hwnd);
	}
	
	if (t) return t->winproc(hwnd, message, wParam, lParam);
	else return DefWindowProc(hwnd, message, wParam, lParam);
}

LRESULT CALLBACK form::winproc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) {
	switch (message) {
	case WM_PAINT:
		if (bitmapName) {
			HBITMAP hbm;
			BITMAP bminfo;
			hbm = LoadBitmap(GetModuleHandle(NULL), bitmapName);
			GetObject(hbm, sizeof(bminfo), &bminfo);
			PAINTSTRUCT ps;
			hdc = BeginPaint(hwnd, &ps);
			HDC memdc = CreateCompatibleDC(hdc);
			SelectObject(memdc, hbm);
			BitBlt(hdc, 0, 0, bminfo.bmWidth, bminfo.bmHeight, memdc, 0, 0, SRCCOPY);
			DeleteDC(memdc);
			EndPaint(hwnd, &ps);
		}
		if (Event_On_Paint) Event_On_Paint();
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
			if (!TrackPopupMenu(RBmenu, TPM_LEFTALIGN | TPM_TOPALIGN | TPM_LEFTBUTTON, LOWORD(lParam), HIWORD(lParam), 0, (HWND)wParam, NULL)) {
				if (RBmenu) MessageBox(NULL, TEXT("弹出菜单失败"), NULL, MB_OK);
			}
			else return DefWindowProc(hwnd, message, wParam, lParam);
		break;
	}
	case WM_TIMER:
		((Timer*)tab[wParam - 1])->Event_Timer();
		break;
	case WM_SIZE:
	{
		w = LOWORD(lParam);
		h = HIWORD(lParam);
		if (Event_Window_Resize) Event_Window_Resize();
		break;
	}
	case WM_MOVE:
		x = LOWORD(lParam);
		y = HIWORD(lParam);
		break;
	case WM_COMMAND:
		if (lParam) {								//这是控件点击事件
			Clickable* p = dynamic_cast<Clickable*>(getControl((HWND)lParam));
			if(p->enable && p->Event_On_Click) p->Event_On_Click();
		}
		else {
			Event_Menu_Click(LOWORD(wParam));
		}
		break;
	case WM_CTLCOLORSTATIC:							//拦截WM_CTLCOLORSTATIC消息
		SetBkMode((HDC)wParam, TRANSPARENT);		//设置背景透明
		return (INT_PTR)GetStockObject(brush);		//返回父窗画刷
		break;
	case WM_CLOSE:
		if (Event_On_Unload) Event_On_Unload();
		DestroyWindow(hwnd);
		PostQuitMessage(0);
		break;
	case WM_CHAR:
		break;
	}
	return DefWindowProc(hwnd, message, wParam, lParam);
}

size_t window::create() {
	Hwnd = CreateWindow(
		this->Classname, this->Name,
		this->feature,
		this->x, this->y, this->w, this->h,
		Parent ? ((window*)Parent)->Hwnd : NULL,
		Menu, hi, this
	);
	if (Hwnd) return id;
	else {
		popError();
		exit(1);
	}
}

window::window(char type, const TCHAR* classname, window* p, int x, int y, int w, int h)
	: Parent(p), Classname((LPTSTR)classname), type(type),
		Text(Hwnd){
	this->x = x < 0 ? Parent->w + x : x;
	this->y = y < 0 ? Parent->h + y : y;
	this->w = w < 0 ? Parent->w + w : w;
	this->h = h < 0 ? Parent->h + h : h;
}

form::~form() {
	if (Parent) ((form*)Parent)->top();
	if (hdc) {
		//TODO
		assert(NULL);
	}
}

form::form(form* parent, const TCHAR* clsName, const TCHAR* title, int x, int y, int w, int h) 
	: window('f', (LPTSTR)clsName, parent, x, y, w, h) {
	this->feature = WS_OVERLAPPEDWINDOW;
	this->name = (TCHAR*) title;
}

void form::forAllControl(std::function<void(control*)> todo) {
	for (window* i : (vector<window*>)tab) todo((control*)i);
}

bool form::regist() {
	WNDCLASSEX wndclass;
	wndclass.cbSize = sizeof(wndclass);
	wndclass.style = CS_HREDRAW | CS_VREDRAW;
	wndclass.lpfnWndProc = WinProc;
	wndclass.cbClsExtra = 0;
	wndclass.cbWndExtra = 0;
	wndclass.hbrBackground = (HBRUSH)GetStockObject(brush);
	wndclass.hCursor = LoadCursor(NULL, IDC_ARROW);
	//注意：此处下两句，MSDN中记载，如果是标准图标，第一个参数应该是NULL而不是hInstance. 此处尚未做处理，但程序运行通过，暂且放下
	wndclass.hIcon = LoadIcon(hi, this->icon);
	wndclass.hIconSm = LoadIcon(hi, this->smallIcon);
	wndclass.hInstance = hi;
	wndclass.lpszClassName = Classname;
	wndclass.lpszMenuName = NULL;
	if (RegisterClassEx(&wndclass)) return true;
	else {
		DWORD x = GetLastError();
		if (x == 1410) return true;
		popError();
		return false;
	}
}

size_t form::create() {
	if (!regist()) return 0;
	window::create();
	if (x < 0) {
		RECT r;
		GetWindowRect(Hwnd, &r);
		x = r.left;
		y = r.top;
		w = r.right - x;
		h = r.bottom - y;
	}
	created = true;
	if (this->Event_On_Create) this->Event_On_Create();
	return id;
}

void form::paintLine(int x1, int y1, int x2, int y2) {
	//if x1 = x2 = y1 = y2 = 0 then end paint.
	PAINTSTRUCT ps;
	hdc = BeginPaint(Hwnd, &ps);
	MoveToEx(hdc, x1, y1, NULL);
	LineTo(hdc, x2, y2);
	EndPaint(Hwnd, &ps);
	hdc = NULL;
}

void form::paintLine(int x1, int y1, int x2, int y2, RECT* rect) {
	//if x1 = x2 = y1 = y2 = 0 then end paint.
	static PAINTSTRUCT ps;
	if (!hdc) {
		hdc = BeginPaint(Hwnd, &ps);
	}
	if (x1 || x2 || y1 || y2) {
		MoveToEx(hdc, x1, y1, NULL);
		LineTo(hdc, x2, y2);
	}
	else {
		EndPaint(Hwnd, &ps);
		hdc = NULL;
		UpdateWindow(Hwnd);
	}
}

window* windowSet::find(HWND hWnd) {
	std::vector<window*>::iterator r = find_if(pool.begin(), pool.end(), [hWnd](window* x) -> bool {return x && hWnd == x->hWnd(); });
	return r == pool.end() ? NULL : *r;
}

window* windowSet::find(LPTSTR clsname) {
	static std::vector<window*>::iterator r;
	if (clsname) r = pool.begin();
	else if (r == pool.end()) return NULL;
	else r++;

	r = find_if(r, pool.end(), [clsname](window* x) -> bool {
		return x && _tcscmp(x->classname(), clsname) == 0;
	});
	return r == pool.end() ? NULL : *r;
}

void windowSet::add(window* which) {
	std::vector<window*>::iterator r = std::find(pool.begin(), pool.end(), nullptr);
	if (r == pool.end()) {
		pool.push_back(which);
		which->id = pool.size();
	}
	else {
		which->id = r - pool.begin() + 1;
		pool[which->id - 1] = which;
	}
	this->__size++;
}