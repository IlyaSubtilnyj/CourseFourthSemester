#pragma once
#include <vector>
#include <string>
#include <list>
#include "SFML/Graphics.hpp"
#include "AssetManager.h"

class Animator
{
public:
	struct Animation
	{
		std::string m_name;
		std::string m_TextureName;
		std::vector<sf::IntRect> m_Frames;
		sf::Time m_Duration;
		bool m_Looping;

		Animation(std::string const& name, std::string const& textureName, sf::Time const& duration, bool looping)
			:
			m_name(name),
			m_TextureName(textureName),
			m_Duration(duration),
			m_Looping(looping) {}

		void AddFrames(sf::Vector2i const& startFrom, sf::Vector2i const& frameSize, unsigned int frames, unsigned int traccia) 
		{
			sf::Vector2i current = startFrom;
			for (unsigned int t = 0; t < traccia; t++)
			{
				for (unsigned int i = 0; i < frames; i++)
				{
					m_Frames.push_back(sf::IntRect(current.x, current.y, frameSize.x, frameSize.y));
					current.x += frameSize.x;
				}
				current.y += frameSize.y;
				current.x = startFrom.x;
			}
		}

	};

	explicit Animator(sf::Sprite& sprite);
	Animator::Animation& creteAnimation(std::string const& name, std::string const& textureName, sf::Time const& duration, bool loop = false);
	void Update(sf::Time const& dt);
	bool SwitchAnimation(std::string const& name); //seitch animations
	
	std::string GetCurrentAnimationName() const;
	void restart();
	bool getEndAnim() const { return endAnim; }

private:
	Animation* FindAnimation(std::string const& name);
	void SwitchAnimation(Animator::Animation* animation);

	sf::Sprite& m_Sprite;
	sf::Time m_CurrentTime;
	std::list<Animator::Animation> m_Animations;
	Animator::Animation* m_CurrentAnimation;
	bool endAnim = false;
};

