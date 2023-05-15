#pragma once
#include <vector>

namespace Net
{
	class Packet final
	{
	public:
		Packet(std::vector<uint8_t>&);
		~Packet();
		unsigned char* getData() const;
		size_t getSize() const;

	private:
		unsigned char* m_data;
		size_t m_buffer_size;
	};
}

