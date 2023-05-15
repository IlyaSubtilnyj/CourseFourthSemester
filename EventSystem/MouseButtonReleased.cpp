#include "MouseButtonReleased.h"

MouseButtonReleased::MouseButtonReleased(float coo_x, float coo_y, int8_t button)
	:
	m_coo_x(coo_x),
	m_coo_y(coo_y),
	m_button(button)
{
	m_type = Event::EventType::MOYSEBUTTONRELEASED;
}

void MouseButtonReleased::pack(std::vector<uint8_t>& const buffer, int16_t& iterator) const
{
	Core::encode(buffer, iterator, m_coo_x);
	Core::encode(buffer, iterator, m_coo_y);
	Core::encode(buffer, iterator, m_button);
}

size_t MouseButtonReleased::getSerializeSize() const
{
	constexpr size_t size = sizeof(m_type) + sizeof(m_coo_x) + sizeof(m_coo_y) + sizeof(m_button);
	return size;
}

MouseButtonReleased MouseButtonReleased::create(float coo_x, float coo_y, int8_t button)
{
	MouseButtonReleased* event_p = new MouseButtonReleased(coo_x, coo_y, button);
	onCreate(dynamic_cast<Event*>(event_p));
	return *event_p;
}

Event* MouseButtonReleased::create(std::vector<uint8_t>& buffer, int16_t& iterator)
{
	float coox = Core::decode<float>(buffer, iterator);;
	float cooy = Core::decode<float>(buffer, iterator);;
	int8_t button = Core::decode<int8_t>(buffer, iterator);
	return new MouseButtonReleased(coox, cooy, button);
}