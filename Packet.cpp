#include "Packet.h"

namespace Net
{
	// NetBuffer
	Packet::Packet(std::vector<uint8_t>& buffer)
		:
		m_buffer_size(buffer.size())
	{
		m_data = new unsigned char[m_buffer_size];
		for (size_t i = 0; i < m_buffer_size; i++)
		{
			*(m_data + i) = buffer[i];
		}

	}

	Packet::~Packet()
	{
		delete[] m_data;
	}

	unsigned char* Packet::getData() const
	{
		return m_data;
	}

	size_t Packet::getSize() const
	{
		return m_buffer_size;
	}
}