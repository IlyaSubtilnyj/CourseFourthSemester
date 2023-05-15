#pragma once
#include "SFML/Graphics.hpp"
#include "Animator.h"

class Player
{
private:

	struct Borders
	{
		float x;
		float y;
		float maxx;
		float maxy;
	};

	int _multi_seq_num;

	//sf::RenderWindow& window;
	sf::Sprite SpritePlayer;
	Animator AnimPlayer = Animator(SpritePlayer);

	float stepx = 0.0f;
	float stepy = 0.0f;
	Borders borders{ 0 };
	sf::Vector2f pos;
	sf::Time timeUpdate;
	bool dead = false;

public:

	Player(sf::RenderWindow& window, sf::Vector2f mypos = sf::Vector2f(90, 365), std::string const& texture = "image/SPRITESHEET.png",
		float time = 0.5f, sf::Vector2i spriteSize = sf::Vector2i(100, 100), int frame = 6, int stepy = 100)
		:
		_multi_seq_num(0),
		//window(window),
		pos(mypos)
	{
		SpritePlayer.setPosition(pos.x, pos.y);
		auto& idleForward = AnimPlayer.creteAnimation("idleForward", texture, sf::seconds(time), true);
		idleForward.AddFrames(sf::Vector2i(0, 0), spriteSize, frame, 1);
		auto& idleBack = AnimPlayer.creteAnimation("idleBack", texture, sf::seconds(time), true);
		idleForward.AddFrames(sf::Vector2i(0, stepy), spriteSize, frame, 1);

		borders.maxx = static_cast<float>(window.getSize().x);
		borders.maxy = static_cast<float>(window.getSize().y);
	}

	void setStey(float y);
	void setStepx(float x);

	void move(char direction, float step = 1.0f);

	void update(sf::Time const& deltaTime);

	void seetBordersPlayer(float x, float y, float maxx, float maxy);

	void setDead(bool playerdead)
	{
		dead = playerdead;
	}

	bool getDead()
	{
		return dead;
	}

	sf::Sprite& getPlayer()
	{
		return SpritePlayer;
	}
};

