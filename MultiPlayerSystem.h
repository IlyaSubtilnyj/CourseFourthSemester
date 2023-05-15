#pragma once
#include "Player.h"

class MultiPlayerSystem
{
private:
	static const int MULTIPLAYER_NUM = 256;
	Player _players[MultiPlayerSystem::MULTIPLAYER_NUM];
};

