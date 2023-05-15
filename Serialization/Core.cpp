#include "Core.h"

namespace Core
{

	namespace Util
	{

		bool isLittleEndian(uint8_t a)
		{
			// 0x00 0x00 0x00 0b0000 0101
			std::string result = std::bitset<8>(a).to_string();
			return (result.back() == '1') ? true : false;
		}

	}
}