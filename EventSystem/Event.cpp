#include "Event.h"

Event::EventType Event::getType()
{
	return m_type;
}

const int32_t Event::EventTypeCount;
TEvent<Event*> Event::onCreate;