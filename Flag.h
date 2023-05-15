#pragma once
#include "SFML/Graphics.hpp"
#include "Serialization/Core.h"

class Flag
{
public:
	Flag(sf::Vector2f pos, sf::Vector2i hex_num, int32_t player_seq_num)
		:
		m_pos(pos),
		m_hex_num(hex_num),
		m_player_seq_num(player_seq_num)
	{}
	Flag()
		:
		m_pos(sf::Vector2f(0.0f, 0.0f)),
		m_hex_num(sf::Vector2i(0, 0)),
		m_player_seq_num(-1) {}
	sf::Vector2f m_pos;
	sf::Vector2i m_hex_num;
	int32_t m_player_seq_num;
	static constexpr float m_tack_off_radius = 70.0f;
	static uint8_t alpha;


	static size_t getSerializeSize()
	{
		constexpr size_t result = sizeof(m_pos.x) + sizeof(m_pos.y) + sizeof(m_hex_num.x) + sizeof(m_hex_num.y) + sizeof(m_player_seq_num);
		return result;
	}

	void serialize(std::vector<uint8_t>& buffer, int16_t& it)
	{
		Core::encode(buffer, it, m_pos.x);
		Core::encode(buffer, it, m_pos.y);
		Core::encode(buffer, it, m_hex_num.x);
		Core::encode(buffer, it, m_hex_num.y);
		Core::encode(buffer, it, m_player_seq_num);
	}

	void deserialize(std::vector<uint8_t>& buffer, int16_t& iterator)
	{
		m_pos.x = Core::decode<float>(buffer, iterator);
		m_pos.y = Core::decode<float>(buffer, iterator);
		m_hex_num.x = Core::decode<int>(buffer, iterator);
		m_hex_num.y = Core::decode<int>(buffer, iterator);
		m_player_seq_num = Core::decode<int32_t>(buffer, iterator);
	}

	static void updateFlagAlpha(float dt)
	{
		//printf("%f\n", dt);
		static uint8_t delta = 1;
		if (alpha >= UCHAR_MAX || alpha <= 0)
			delta *= -1;	
		alpha += delta;
		//printf("%f\n", alpha);
	}

};