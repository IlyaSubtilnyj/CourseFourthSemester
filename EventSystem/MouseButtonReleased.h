#pragma once
#include "Event.h"

class MouseButtonReleased : public Event
{
public:

	MouseButtonReleased() = delete;

	virtual void pack(std::vector<uint8_t>& const, int16_t&) const override;
	virtual size_t getSerializeSize() const override;

	static MouseButtonReleased create(float, float, int8_t);
	static Event* create(std::vector<uint8_t>&, int16_t&);

private:

	MouseButtonReleased(float, float, int8_t);
	float m_coo_x;
	float m_coo_y;
	int8_t m_button;
};


