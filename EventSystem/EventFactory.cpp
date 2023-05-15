#include "EventFactory.h"

//EventFactory

EventFactory::EventFactory()
	:
	m_serialization_size(0)
{
	Event::onCreate += METHOD_HANDLER(*this, EventFactory::registerEvent);
}

void EventFactory::serialize(std::vector<uint8_t>& buffer)
{
	buffer.resize(m_serialization_size);
	int16_t iterator = 0;
	for (Event* event : m_events)
	{
		Core::encode(buffer, iterator, (int8_t)event->getType());
		event->pack(buffer, iterator);
	}
}

void EventFactory::registerEvent(Event* event)
{
	m_events.push_back(event);
	m_serialization_size += event->getSerializeSize();
}

std::unique_ptr<Net::Packet> EventFactory::PackEventsToPacket()
{
	std::vector<uint8_t> buffer;
	serialize(buffer);
	EventListClear();
	return std::make_unique<Net::Packet>(buffer);
}

std::vector<Event*>* EventFactory::EventListCreate(const unsigned char* const buf, size_t buffer_size)
{
	std::vector<uint8_t> buffer(buf, buf + buffer_size);
	int16_t iterator = 0;
	m_events.clear();
	while (buffer.size() > iterator)
	{
		int8_t e_type = Core::decode<int8_t>(buffer, iterator);
		auto* event = creators[(int8_t)e_type - 1](buffer, iterator);
		m_events.push_back(event);
	}
	return &m_events;
}

void EventFactory::EventListClear(std::vector<Event*>* vec)
{
	if (vec == nullptr) return;
	for (Event* e : *vec)
	{
		delete e;
	}
	vec->clear();
	m_serialization_size = 0;
}

std::shared_ptr<EventFactory> EventFactory::getInstance()
{
	static std::shared_ptr<EventFactory> ef_instance{ new EventFactory() };
	return ef_instance;
}

void EventFactory::EventListClear()
{
	for (Event* e : m_events)
	{
		delete e;
	}
	m_events.clear();
	m_serialization_size = 0;
}

EventFactory::PFN_EventCreator EventFactory::creators[Event::EventTypeCount] = { MouseButtonPressed::create, MouseButtonReleased::create,
																				KeyBoardKeyPressed::create, KeyBoardKeyReleased::create, WindowClosed::create };