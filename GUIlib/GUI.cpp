#pragma once
#include "stdafx.h"

std::vector<window*> window::formSet;

void form::run() {
	//在此处挂起
	for (void* p : tab) ((control*)p)->create();

	MSG msg;
	ShowWindow(hWnd(), SW_SHOW);
	UpdateWindow(hWnd());
	ZeroMemory(&msg, sizeof(msg));
	//this->createOver = true;						//消息队列建立		//我想了两天终于下决心把这个SB东西和下边那句话调了过来		//上帝保佑我别再出bug了		//再写bug我学分就都没了
	if (this->Event_Load_Complete) this->Event_Load_Complete(this);

	int loopret = 0;
	while ((loopret = GetMessage(&msg, this->hWnd(), 0, 0)) > 0)
	{
		if (loopret == -1) {
			popError();
			exit(1);
		}
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	std::vector<window*>::iterator r = find_if(formSet.begin(), formSet.end(), [this](window* x) -> bool {
		return x != (void*)this && (_tcscmp(x->classname(), this->classname()) == 0);
	});
	if(r != formSet.end()) UnregisterClass(classname(), hi);
}

control* form::getControl(HWND controlHwnd) {
	std::vector<window*>::iterator r = find_if(tab.begin(), tab.end(), [controlHwnd](window* x) -> bool { return x->hWnd() == controlHwnd; });
	return r == tab.end() ? nullptr : (control*)*r;
}

form* form::getForm(HWND hWnd) {
	std::vector<window*>::iterator r = find_if(window::formSet.begin(), window::formSet.end(), [hWnd](window* x) -> bool {
		return x->hWnd() == hWnd;
	});
	return r == window::formSet.end() ? nullptr : (form*)*r;
};

static LRESULT CALLBACK WinProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	form* t = form::getForm(hwnd);
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
				if (t->RButtonMenu) MessageBox(NULL, TEXT("弹出菜单失败"), NULL, MB_OK);
			}
			else return DefWindowProc(hwnd, message, wParam, lParam);
		break;
	}
	case WM_TIMER:
		((Timer*)t->tab[wParam - 1])->Event_Timer(t);
		break;
	case WM_SIZE:
	{
		t->w = LOWORD(lParam);
		t->h = HIWORD(lParam);
		if (t->Event_Window_Resize) t->Event_Window_Resize(t);
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
				if (((Button*)p)->Event_On_Click) ((Button*)p)->Event_On_Click((Button*)p);
				break;
			case 'l':
				if (((Label*)p)->Event_On_Click) ((Label*)p)->Event_On_Click((Label*)p);
				break;
			case 'p':
				if (((Picture*)p)->Event_On_Click) ((Picture*)p)->Event_On_Click((Picture*)p);
				break;
			case 'r':
				if (((Radio*)p)->Event_On_Check) ((Radio*)p)->Event_On_Check((Radio*)p);
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

size_t window::create() {
	Hwnd = CreateWindow(
		this->Classname, this->Name,
		this->feature,
		this->x, this->y, this->w, this->h,
		Parent ? ((window*)Parent)->Hwnd : NULL,
		Menu, hi, NULL
	);
	if (Hwnd) return id;
	else {
		popError();
		exit(1);
	}
}

window::window(LPTSTR classname, window* p, int x, int y, int w, int h)
	: x(x), y(y), w(w), h(h), Parent(p), Classname(classname)
{
	name.setContainer(this);
	menu.setContainer(this);
	name.setter(&window::setName);
	name.getter(&window::getName);
	menu.setter(&window::setMenu);
}

form::~form() {
	if (parent()) ((form*)parent())->top();
	auto todelete = find(formSet.begin(), formSet.end(), this);
	if (formSet.end() != todelete) {
		size_t pos = todelete - formSet.begin();
		formSet.erase(todelete);
		for (; pos < formSet.size(); pos++) {
			((form*)formSet[pos])->id = pos + 1;
		}
	}
}

form::form(form* parent, const TCHAR* clsName, const TCHAR* title, int x, int y, int w, int h) 
	: window((LPTSTR)clsName, parent, x, y, w, h) {
	this->type = 'f';
	this->feature = WS_OVERLAPPEDWINDOW;
	this->name = (TCHAR*) title;
	RButtonMenu.setContainer(this);
	RButtonMenu.getter(&form::CONTEXTMENU);
	push();
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
	wndclass.lpszClassName = classname();
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
		GetWindowRect(hWnd(), &r);
		x = r.left;
		y = r.top;
		w = r.right - x;
		h = r.bottom - y;
	}
	if (this->Event_On_Create) this->Event_On_Create(this);
	return id;
}

void form::paintLine(int x1, int y1, int x2, int y2) {
	//if x1 = x2 = y1 = y2 = 0 then end paint.
	PAINTSTRUCT ps;
	hdc = BeginPaint(hWnd(), &ps);
	MoveToEx(hdc, x1, y1, NULL);
	LineTo(hdc, x2, y2);
	EndPaint(hWnd(), &ps);
	hdc = NULL;
}

void form::paintLine(int x1, int y1, int x2, int y2, RECT* rect) {
	//if x1 = x2 = y1 = y2 = 0 then end paint.
	static PAINTSTRUCT ps;
	if (!hdc) {
		hdc = BeginPaint(hWnd(), &ps);
	}
	if (x1 || x2 || y1 || y2) {
		MoveToEx(hdc, x1, y1, NULL);
		LineTo(hdc, x2, y2);
	}
	else {
		EndPaint(hWnd(), &ps);
		hdc = NULL;
		UpdateWindow(hWnd());
	}
}