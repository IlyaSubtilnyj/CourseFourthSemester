#include "Player.h"

void Player::move(char direction, float step)
{
	if (!dead)
	{
		switch (direction)
		{
		case 'u': if (SpritePlayer.getPosition().y > borders.y) stepy = -step; break;
		case 'd': if (SpritePlayer.getPosition().y < borders.maxy) stepy = step; break;
		case 'r': {if (SpritePlayer.getPosition().x < borders.maxx) stepx = step;
			if (AnimPlayer.GetCurrentAnimationName() != "idleForward") AnimPlayer.SwitchAnimation("idleForward");
			break;}
		case 'l': {if (SpritePlayer.getPosition().x > borders.x) stepx = -step;
			if (AnimPlayer.GetCurrentAnimationName() != "idleBack") AnimPlayer.SwitchAnimation("idleBack");
			break; }
		default:
			break;
		}
	}
}

void Player::update(sf::Time const& deltaTime)
{
	if (!dead) AnimPlayer.Update(deltaTime);

	timeUpdate += deltaTime;

	if (timeUpdate > sf::milliseconds(3))
	{
		timeUpdate = sf::milliseconds(0);

		if (!dead) {
			SpritePlayer.move(stepx, stepy);
			if (SpritePlayer.getPosition().x < borders.x) {
				SpritePlayer.setPosition(borders.x, SpritePlayer.getPosition().y);
			}
			if (SpritePlayer.getPosition().x > borders.maxx - SpritePlayer.getGlobalBounds().width) {
				SpritePlayer.setPosition(borders.maxx - SpritePlayer.getGlobalBounds().width, SpritePlayer.getPosition().y);
			}
			if (SpritePlayer.getPosition().y < borders.y) {
				SpritePlayer.setPosition(SpritePlayer.getPosition().x, borders.y);
			}
			if (SpritePlayer.getPosition().y > borders.maxy - SpritePlayer.getGlobalBounds().height) {
				SpritePlayer.setPosition(SpritePlayer.getPosition().x, borders.maxy - SpritePlayer.getGlobalBounds().height);
			}
		}
		else 
		{
			if (SpritePlayer.getRotation() == 0) {
				SpritePlayer.setPosition(SpritePlayer.getPosition().x + SpritePlayer.getGlobalBounds().height, SpritePlayer.getPosition().y);
				SpritePlayer.setRotation(90);
			}
			SpritePlayer.move(0,1);
			if (SpritePlayer.getPosition().y > borders.maxy)
			{
				SpritePlayer.setRotation(0);
				SpritePlayer.setPosition(pos.x, pos.y); 
				dead = false;
			}
		}
	}
}

void Player::setStepx(float x) 
{
	stepx = x;
}

void Player::setStey(float y)
{
	stepy = y;
}

void Player::seetBordersPlayer(float x, float y, float maxx, float maxy)
{
	borders.x = x;
	borders.y = y;
	borders.maxx = maxx;
	borders.maxy = maxy;
}