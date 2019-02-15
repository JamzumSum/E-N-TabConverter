#pragma once
#include <functional>

typedef std::function<void(void)> vvEvent;

class Event {
public:
	bool enable = true;
};

class Clickable : public Event {
public:
	vvEvent Event_On_Click = NULL;
};