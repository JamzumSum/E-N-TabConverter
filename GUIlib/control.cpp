#pragma once
#include "stdafx.h"

control::control(char t, const TCHAR* clsname, window* parent, int x, int y, int w, int h)
	: window(t, clsname, parent, x, y, w, h) {
	this->feature = this->feature | WS_CHILD | WS_VISIBLE;
	push();
}

Button::Button(form* parent, int x, int y, int w, int h, const TCHAR* Name) 
	: control('b', (TCHAR*)_T("BUTTON"), parent, x, y, w, h){
	this->name = (LPTSTR) Name;
	this->feature |= BS_DEFPUSHBUTTON;
}

Label::Label(form* parent, int x, int y, int w, int h, const TCHAR* Name) 
	: control('l', (TCHAR*)_T("STATIC"), parent, x, y, w, h) {
	this->name = (LPTSTR) Name;
	this->feature |= SS_NOTIFY | SS_LEFT;
}

Picture::Picture(form* parent, int x, int y, int w, int h, const TCHAR* picPath) 
	: control('I', (TCHAR*)_T("STATIC"), parent, x, y, w, h) {
	this->name  = picPath;
	this->feature |= SS_NOTIFY | SS_BITMAP | SS_CENTERIMAGE;
}

ProgressBar::ProgressBar(form* parent, int x, int y, int w, int h, const LPTSTR Name) 
	: control('P', (TCHAR*)_T("msctls_progress32"), parent, x, y, w, h) {
	this->name = (LPTSTR)Name;
	this->feature |= PBS_SMOOTH;
}

Radio::Radio(form* parent, int x, int y, int w, int h, const LPTSTR Name, bool head) 
	: control('r', (TCHAR*)_T("BUTTON"), parent, x, y, w, h) {
	this->head = head;
	this->name = Name;
	this->feature |= BS_AUTORADIOBUTTON;
	if (head) this->feature |= WS_GROUP;

	Value.setContainer(this);
	Value.setter(&Radio::setCheck);
	Value.getter(&Radio::getCheck);
}

Checkbox::Checkbox(form* parent, int x, int y, int w, int h, const TCHAR* Name) 
	: control('c', (TCHAR*)_T("BUTTON"), parent, x, y, w, h) {
	this->name = (LPTSTR) Name;
	this->feature |= BS_AUTOCHECKBOX;
	Value.setContainer(this);
	Value.setter(&Checkbox::setCheck);
	Value.getter(&Checkbox::getCheck);
}

void Timer::setTimer(bool value) {
	if (value ^ this->value) this->value = value;
	else return;
	if (value) {
		if (Event_Timer) {
			assert(Event_Timer);
			if (!SetTimer(parent->hWnd(), id, Interval, NULL)) {
				MessageBox(NULL, _T("Fail to set timer"), (LPTSTR)(ULONG_PTR)GetLastError(), MB_OK);
				this->value = false;
			}
		}
	}
	else {
		KillTimer(parent->hWnd(), id);
	}
}

void Timer::setInterval(UINT value) {
	if (value != Interval) {
		bool r = enabled;
		enabled = false;
		Interval = value;
		setTimer(r);
	}
}

Timer::Timer(form* parent, UINT interval, vvEvent Event, bool enabled = false) {
	this->Event_Timer = Event;
	this->Interval = interval;
	this->enabled.setContainer(this);
	this->interval.setContainer(this);
	this->enabled.setter(&Timer::setTimer);
	this->enabled.getter(&Timer::getTimer);
	this->interval.setter(&Timer::setInterval);
	this->interval.getter(&Timer::getInterval);
	if (enabled) this->enabled = true;
}

Textbox::Textbox(form* parent, int x, int y, int w, int h, const LPTSTR Name, bool Multiline) 
	: control('t', (TCHAR*)_T("Edit"), parent, x, y, w, h) {
	this->name = Name;
	this->multiline = Multiline;
	this->feature |= WS_BORDER | WS_GROUP | WS_TABSTOP | ES_WANTRETURN;
	if (Multiline) this->feature |= ES_MULTILINE;
}

LRESULT Textbox::proc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) {
	Textbox* p = NULL;
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
		if (p->Event_Text_Change) p->Event_Text_Change();
		return r;
	}
	default:
		return CallWindowProc((WNDPROC)p->preProc, hwnd, message, wParam, lParam);
	}
};