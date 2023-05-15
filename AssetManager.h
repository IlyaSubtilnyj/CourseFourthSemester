#pragma once
#include "SFML/Audio.hpp"
#include "SFML/Graphics.hpp"
#include <assert.h>
#include <map>

//singleton
class AssetManager
{
private:
	std::map<std::string, sf::Texture> m_Textures;
	std::map<std::string, sf::SoundBuffer> m_SoundBuffers;
	std::map<std::string, sf::Font> m_Fonts;

	static AssetManager* sInstance;

public:
	AssetManager();
	static AssetManager* getInstance();
	static sf::Texture& GetTexture(std::string const& filename);
	static sf::SoundBuffer& GetSoundBuffer(std::string const& filename);
	static sf::Font& GetFont(std::string const& filename);

};

