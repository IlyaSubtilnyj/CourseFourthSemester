#pragma once
#include <stdint.h>
#include <vector>
#include <memory>
#include "Events.h"
#include "../Packet.h"

class EventFactory
{
public:

	typedef Event* (*PFN_EventCreator)(std::vector<uint8_t>& vec, int16_t& iterator);
	static PFN_EventCreator creators[Event::EventTypeCount];

	virtual void serialize(std::vector<uint8_t>&);
	virtual void registerEvent(Event*);
	virtual std::unique_ptr<Net::Packet> PackEventsToPacket();
	virtual std::vector<Event*>* EventListCreate(const unsigned char* const, size_t);
	virtual void EventListClear(std::vector<Event*>*);

public:
	static std::shared_ptr<EventFactory> getInstance();
	EventFactory(EventFactory const&) = delete;
	EventFactory& operator=(EventFactory const&) = delete;

private:
	EventFactory();
	void EventListClear();
	int32_t m_serialization_size;
protected:
	std::vector<Event*> m_events;
};