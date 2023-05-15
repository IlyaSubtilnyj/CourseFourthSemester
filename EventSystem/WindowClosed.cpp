#include "WindowClosed.h"

WindowClosed::WindowClosed()
{
	m_type = Event::EventType::WINDOWCLOSED;
}

void WindowClosed::pack(std::vector<uint8_t>& const buffer, int16_t& iterator) const {}

size_t WindowClosed::getSerializeSize() const
{
	constexpr size_t size = sizeof(m_type);
	return size;
}

WindowClosed WindowClosed::create()
{
	//subscription on event
	WindowClosed* event_p = new WindowClosed();
	onCreate(dynamic_cast<Event*>(event_p));
	return *event_p;
}

Event* WindowClosed::create(std::vector<uint8_t>& buffer, int16_t& iterator)
{
	return new WindowClosed();
}