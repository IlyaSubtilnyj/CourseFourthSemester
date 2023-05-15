#pragma once
#include "Event.h"

class MouseButtonPressed : public Event
{
public:

	MouseButtonPressed() = delete;

	virtual void pack(std::vector<uint8_t>& const, int16_t&) const override;
	virtual size_t getSerializeSize() const override;

	static MouseButtonPressed create(float, float, int8_t);
	static Event* create(std::vector<uint8_t>&, int16_t&);

private:

	MouseButtonPressed(float, float, int8_t);
	float m_coo_x;
	float m_coo_y;
	int8_t m_button;
};

