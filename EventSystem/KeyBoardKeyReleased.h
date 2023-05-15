#pragma once
#include "Event.h"

class KeyBoardKeyReleased : public Event
{
public:
	KeyBoardKeyReleased() = delete;

	virtual void pack(std::vector<uint8_t>& const, int16_t&) const override;
	virtual size_t getSerializeSize() const override;

	static KeyBoardKeyReleased create(int8_t key);
	static Event* create(std::vector<uint8_t>&, int16_t&);

private:
	KeyBoardKeyReleased(int8_t);
	int8_t m_key;
};
