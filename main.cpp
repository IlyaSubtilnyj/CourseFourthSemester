#include "Window.h"
#include "SFML/Graphics.hpp"

int main(int argc, char** argv) {
	(void)argc; (void)argv;

	using namespace Core;

	std::vector<sf::VideoMode> video_modes = sf::VideoMode::getFullscreenModes();
	sf::VideoMode v_mode = video_modes[0];
	unsigned int v_mode_height = v_mode.height;
	unsigned int v_mode_width = v_mode.width;
	unsigned int v_mode_bpp = v_mode.bitsPerPixel;

	Window& wind = Window::getInstance("main", v_mode_height, v_mode_width, v_mode_bpp);
	wind.run();

	return 0;
}