#pragma once
#include "stdafx.h"

button::button(form* parent, int x, int y, int w, int h, const TCHAR* Name) {
	this->x = x;
	this->y = y;
	this->w = w;
	this->h = h;
	this->type = 'b';
	this->parent = parent;
	this->name = (LPTSTR) Name;
	this->classname = (TCHAR*)_T("BUTTON");
	this->feature |= BS_DEFPUSHBUTTON;
	push();
}

Label::Label(form* parent, int x, int y, int w, int h, const TCHAR* Name) {
	this->x = x;
	this->y = y;
	this->w = w;
	this->h = h;
	this->type = 'l';
	this->parent = parent;
	this->name = (LPTSTR) Name;
	this->classname = (TCHAR*)_T("STATIC");
	this->feature |= SS_NOTIFY | BS_FLAT;
	push();
}

void Label::setFont(LPTSTR fontName, int size) {
	HFONT hFont = CreateFont(size, 0, 0, 0, FW_THIN, false, false, false,
		CHINESEBIG5_CHARSET, OUT_CHARACTER_PRECIS,
		CLIP_CHARACTER_PRECIS, DEFAULT_QUALITY,
		FF_MODERN, fontName);
	SendMessage(this->hWnd, WM_SETFONT, (WPARAM)hFont, TRUE);//发送设置字体消息
}

Picture::Picture(form* parent, int x, int y, int w, int h, const LPTSTR Name, const LPTSTR picPath) {
	this->x = x;
	this->y = y;
	this->w = w;
	this->h = h;
	this->type = 'p';
	this->parent = parent;
	this->name = (LPTSTR)Name;				//x
	this->path = (LPTSTR)picPath;
	this->classname = (TCHAR*)_T("STATIC");
	this->feature |= SS_NOTIFY | SS_BITMAP;
	push();
}

ProgressBar::ProgressBar(form* parent, int x, int y, int w, int h, const LPTSTR Name) {
	this->x = x;
	this->y = y;
	this->w = w;
	this->h = h;
	this->type = 'P';
	this->parent = parent;
	this->name = (LPTSTR)Name;				//x
	this->classname = (TCHAR*)_T("msctls_progress32");
	this->feature |= PBS_SMOOTH;
	push();
}

radio::radio(form* parent, int x, int y, int w, int h, const LPTSTR Name, bool head) {
	this->x = x;
	this->y = y;
	this->w = w;
	this->h = h;
	this->head = head;
	this->type = 'r';
	this->parent = parent;
	this->name = Name;				//x
	this->classname = (TCHAR*)_T("BUTTON");
	this->feature |= BS_AUTORADIOBUTTON;
	if (head) this->feature |= WS_GROUP;

	Value.setContainer(this);
	Value.setter(&radio::setCheck);
	Value.getter(&radio::getCheck);
	push();
}

Checkbox::Checkbox(form* parent, int x, int y, int w, int h, const TCHAR* Name) {
	this->x = x;
	this->y = y;
	this->w = w;
	this->h = h;
	this->type = 'c';
	this->parent = parent;
	this->name = (LPTSTR) Name;				//x
	this->classname = (TCHAR*)_T("BUTTON");
	this->feature |= BS_AUTOCHECKBOX;
	Value.setContainer(this);
	Value.setter(&Checkbox::setCheck);
	Value.getter(&Checkbox::getCheck);
	push();
}

void timer::setTimer(bool value) {
	if (value ^ this->value) this->value = value;
	else return;
	if (value) {
		if (Event_Timer) {
			assert(Event_Timer);
			void* t = parent;
			if (!SetTimer(((form*)t)->hWnd, id, Interval, NULL)) {
				MessageBox(NULL, _T("Fail to set timer"), (LPTSTR)(ULONG_PTR)GetLastError(), MB_OK);
				this->value = false;
			}
		}
	}
	else {
		void* t = parent;
		KillTimer(((form*)t)->hWnd, id);
	}
}

void timer::setInterval(UINT value) {
	if (value != Interval) {
		bool r = enabled;
		enabled = false;
		Interval = value;
		setTimer(r);
	}
}

timer::timer(form* parent, UINT interval, void(*Event)(form*), bool enabled = false) {
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

Textbox::Textbox(form* parent, int x, int y, int w, int h, const LPTSTR Name, bool Multiline) {
	this->x = x;
	this->y = y;
	this->w = w;
	this->h = h;
	this->type = 't';
	this->parent = parent;
	this->name = Name;
	this->multiline = Multiline;
	this->classname = (TCHAR*)_T("Edit");
	this->feature |= WS_BORDER | WS_GROUP | WS_TABSTOP | ES_WANTRETURN;
	if (Multiline) this->feature |= ES_MULTILINE;
	push();
}
