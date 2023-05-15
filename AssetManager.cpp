#include "AssetManager.h"

AssetManager* AssetManager::sInstance = nullptr;

AssetManager::AssetManager()
{
	assert(sInstance == nullptr);
	sInstance = this;
}

AssetManager* AssetManager::getInstance()
{
	return sInstance;
}

sf::Texture& AssetManager::GetTexture(std::string const& filename)
{
	auto& textMap = sInstance->m_Textures;
	auto pairFound = textMap.find(filename);
	if (pairFound != textMap.end())
	{
		return pairFound->second;
	}
	else
	{
		auto& texture = textMap[filename];
		texture.loadFromFile(filename);
		return texture;
	}
}

sf::SoundBuffer& AssetManager::GetSoundBuffer(std::string const& filename)
{
	auto& textMap = sInstance->m_SoundBuffers;
	auto pairFound = textMap.find(filename);
	if (pairFound != textMap.end())
	{
		return pairFound->second;
	}
	else
	{
		auto& texture = textMap[filename];
		texture.loadFromFile(filename);
		return texture;
	}
}

sf::Font& AssetManager::GetFont(std::string const& filename)
{
	auto& textMap = sInstance->m_Fonts;
	auto pairFound = textMap.find(filename);
	if (pairFound != textMap.end())
	{
		return pairFound->second;
	}
	else
	{
		auto& texture = textMap[filename];
		texture.loadFromFile(filename);
		return texture;
	}
}
