#pragma once
#include "stdafx.h"

control::control(char t, LPTSTR clsname, window* parent, int x, int y, int w, int h)
	: window(t, clsname, parent, x, y, w, h) {
	this->feature = this->feature | WS_CHILD | WS_VISIBLE;
	push();
}

Button::Button(form* parent, int x, int y, int w, int h, const TCHAR* Name) 
	: control('b', (TCHAR*)_T("BUTTON"), parent, x, y, w, h) {
	this->name = (LPTSTR) Name;
	this->feature |= BS_DEFPUSHBUTTON;
}

Label::Label(form* parent, int x, int y, int w, int h, const TCHAR* Name) 
	: control('l', (TCHAR*)_T("STATIC"), parent, x, y, w, h) {
	this->name = (LPTSTR) Name;
	this->feature |= SS_NOTIFY | BS_FLAT;
}

void Label::setFont(LPTSTR fontName, int size) {
	HFONT hFont = CreateFont(size, 0, 0, 0, FW_THIN, false, false, false,
		CHINESEBIG5_CHARSET, OUT_CHARACTER_PRECIS,
		CLIP_CHARACTER_PRECIS, DEFAULT_QUALITY,
		FF_MODERN, fontName);
	SendMessage(this->Hwnd, WM_SETFONT, (WPARAM)hFont, TRUE);//发送设置字体消息
}

Picture::Picture(form* parent, int x, int y, int w, int h, const LPTSTR Name, const LPTSTR picPath) 
	: control('p', (TCHAR*)_T("STATIC"), parent, x, y, w, h) {
	this->name = (LPTSTR)Name;				//x
	this->path = (LPTSTR)picPath;
	this->feature |= SS_NOTIFY | SS_BITMAP;
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
			if (!SetTimer(parent()->hWnd(), id, Interval, NULL)) {
				MessageBox(NULL, _T("Fail to set timer"), (LPTSTR)(ULONG_PTR)GetLastError(), MB_OK);
				this->value = false;
			}
		}
	}
	else {
		KillTimer(parent()->hWnd(), id);
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

Timer::Timer(form* parent, UINT interval, void(*Event)(form*), bool enabled = false) 
	: control('T', NULL, parent) {
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
