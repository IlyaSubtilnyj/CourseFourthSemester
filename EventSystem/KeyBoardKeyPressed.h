#pragma once
#include "Event.h"

class KeyBoardKeyPressed : public Event
{
public:
	KeyBoardKeyPressed() = delete;

	virtual void pack(std::vector<uint8_t>& const, int16_t&) const override;
	virtual size_t getSerializeSize() const override;

	static KeyBoardKeyPressed create(int8_t);
	static Event* create(std::vector<uint8_t>&, int16_t&);

	int8_t getKeyCode();
private:
	KeyBoardKeyPressed(int8_t);
	int8_t m_key;
};

