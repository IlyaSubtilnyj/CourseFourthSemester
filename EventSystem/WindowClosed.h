#pragma once
#include "Event.h"

class WindowClosed : public Event
{
public:

	virtual void pack(std::vector<uint8_t>& const, int16_t&) const override;
	virtual size_t getSerializeSize() const override;

	static WindowClosed create();
	static Event* create(std::vector<uint8_t>&, int16_t&);

private:
	WindowClosed();
};

