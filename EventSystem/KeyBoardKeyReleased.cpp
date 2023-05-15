#include "KeyBoardKeyReleased.h"

KeyBoardKeyReleased::KeyBoardKeyReleased(int8_t key)
	:
	m_key(key)
{
	m_type = Event::EventType::KEYBOARDKEYRELEASED;
}

void KeyBoardKeyReleased::pack(std::vector<uint8_t>& const buffer, int16_t& iterator) const
{
	Core::encode(buffer, iterator, m_key);
}

size_t KeyBoardKeyReleased::getSerializeSize() const
{
	constexpr size_t size = sizeof(m_type) + sizeof(m_key);
	return size;
}

KeyBoardKeyReleased KeyBoardKeyReleased::create(int8_t key)
{
	KeyBoardKeyReleased* event_p = new KeyBoardKeyReleased(key);
	onCreate(dynamic_cast<Event*>(event_p));
	return *event_p;
}

Event* KeyBoardKeyReleased::create(std::vector<uint8_t>& buffer, int16_t& iterator)
{
	int8_t key = Core::decode<int8_t>(buffer, iterator);
	return new KeyBoardKeyReleased(key);
}