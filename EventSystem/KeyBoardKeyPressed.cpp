#include "KeyBoardKeyPressed.h"

KeyBoardKeyPressed::KeyBoardKeyPressed(int8_t key)
	:
	m_key(key)
{
	m_type = Event::EventType::KEYBOARDKEYPRESSED;
}

void KeyBoardKeyPressed::pack(std::vector<uint8_t>& const buffer, int16_t& iterator) const
{
	Core::encode(buffer, iterator, m_key);
}

size_t KeyBoardKeyPressed::getSerializeSize() const
{
	constexpr size_t size = sizeof(m_type) + sizeof(m_key);
	return size;
}

KeyBoardKeyPressed KeyBoardKeyPressed::create(int8_t key)
{
	//subscription on event
	KeyBoardKeyPressed* event_p = new KeyBoardKeyPressed(key);
	onCreate(dynamic_cast<Event*>(event_p));
	return *event_p;
}

Event* KeyBoardKeyPressed::create(std::vector<uint8_t>& buffer, int16_t& iterator)
{
	int8_t key = Core::decode<int8_t>(buffer, iterator);
	return new KeyBoardKeyPressed(key);
}

int8_t KeyBoardKeyPressed::getKeyCode()
{
	return m_key;
}