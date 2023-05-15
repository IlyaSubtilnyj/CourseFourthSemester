#pragma once
#include <stdint.h>
#include <vector>
#include "../EventSubscription/EventSubscription.h"
#include "../Serialization/Core.h"

class Event
{
public:
	virtual ~Event() {}

	enum class EventType : int8_t
	{
		UNKNOWN = 0,
		MOUSEBUTTONPRESSED,
		MOYSEBUTTONRELEASED,
		KEYBOARDKEYPRESSED,
		KEYBOARDKEYRELEASED,
		WINDOWCLOSED
	};
	static const int32_t EventTypeCount = 5;

	//event sunscription
	static TEvent<Event*> onCreate;
	virtual void pack(std::vector<uint8_t>& const buffer, int16_t& iterator) const = 0;
	virtual size_t getSerializeSize() const = 0;
	Event::EventType getType();
protected:
	Event::EventType m_type = Event::EventType::UNKNOWN;
};