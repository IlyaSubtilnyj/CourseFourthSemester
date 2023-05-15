#pragma once
#include "Window.h"

namespace Core
{

	sf::Font WindowState::con_perm_font;

	AssetManager WindowState::manager;
	WindowState* WindowState::transitionState = &WindowMenu::getInstance();
	WindowState& WindowState::wWindowMain = WindowMain::getInstance();
	WindowState& WindowState::wWindowPregame = WindowPregame::getInstance();
	WindowState& WindowState::wWindowMenu = WindowMenu::getInstance();
	GameCollision* WindowState::gc = nullptr;

	Window* Window::s_instance = nullptr;

	Window::Window(std::string const& name, unsigned int height, unsigned int width, unsigned int bpp)
	{
		init(name, height, width, bpp);
	}

	void Window::init(std::string const& name, unsigned int height, unsigned int width, unsigned int bpp)
	{
		this->m_name = name;
		this->m_height = height;
		this->m_width = width;
		this->m_bpp = bpp;

		this->currentState = &WindowMenu::getInstance();
		this->currentState->enter(this);

		this->running = true;
		sf::VideoMode v_mode(height, width, bpp);
		this->window = new sf::RenderWindow(v_mode, name, sf::Style::Fullscreen);
		this->window->setFramerateLimit(144);
	}


	//main function
	void Window::run(void)
	{
		if (!net::InitializeSockets())
		{
			printf("failed to initialize sockets\n");
			return;
		}
		sf::Clock clock;
		while (isOpen())
		{
			DT = clock.restart();
			pollEvents();
			update(DT);
			release();
		}
		net::ShutdownSockets();
	}

	//handlers
	void Window::pollEvents() const
	{
		sf::Event event;
		while (window->pollEvent(event))
		{
			if (event.type == sf::Event::MouseButtonPressed)
			{
				sf::Vector2i cooi = sf::Mouse::getPosition(*window);
				sf::Vector2f coof = window->mapPixelToCoords(cooi);
				sf::Mouse::Button button = event.mouseButton.button;
				MouseButtonPressedHandler(coof, button);
			}
			else if (event.type == sf::Event::MouseButtonReleased)
			{
				sf::Vector2i cooi = sf::Mouse::getPosition(*window);
				sf::Vector2f coof = window->mapPixelToCoords(cooi);
				sf::Mouse::Button button = event.mouseButton.button;
				MouseButtonReleasedHandler(coof, button);
			}
			else if (event.type == sf::Event::KeyPressed)
			{
				sf::Keyboard::Key key = event.key.code;
				KeyBoardKeyPressedHandler(key);
			}
			else if (event.type == sf::Event::KeyReleased)
			{
				sf::Keyboard::Key key = event.key.code;
				KeyBoardKeyReleasedHandler(key);
			}
			else if (event.type == sf::Event::Closed)
			{
				WindowClosedHandler();
			}
		}
	}

	void Window::update(sf::Time time) const
	{
		this->fnUpdateCB(time);
	}

	void Window::release() const
	{
		this->fnReleaseCB();
	}

	void Window::MouseButtonPressedHandler(sf::Vector2f coo, sf::Mouse::Button button)
	{
		auto e = MouseButtonPressed::create(coo.x, coo.y, static_cast<int8_t>(button));
		s_instance->fnEventHandlerCB(e);
	}

	void Window::MouseButtonReleasedHandler(sf::Vector2f coo, sf::Mouse::Button button)
	{
		auto e = MouseButtonReleased::create(coo.x, coo.y, static_cast<int8_t>(button));
		s_instance->fnEventHandlerCB(e);
	}

	void Window::KeyBoardKeyPressedHandler(sf::Keyboard::Key key)
	{
		auto e = KeyBoardKeyPressed::create(static_cast<int8_t>(key));
		s_instance->fnEventHandlerCB(e);
	}

	void Window::KeyBoardKeyReleasedHandler(sf::Keyboard::Key key)
	{
		auto e = KeyBoardKeyReleased::create(static_cast<int8_t>(key));
		s_instance->fnEventHandlerCB(e);
	}

	void Window::WindowClosedHandler() 
	{
		auto e = WindowClosed::create();
		s_instance->fnEventHandlerCB(e);
	}

	//Window state
	WindowState& WindowMenu::getInstance()
	{
		static WindowMenu instance;
		return instance;
	}
	bool WindowState::connected = false;
	float WindowState::sendAccumulator = 0.0f;
	float WindowState::statsAccumulator = 0.0f;
	float WindowState::netTimeAccumulator = 0.0f;
	net::ReliableConnection WindowState::connection = net::ReliableConnection(WindowState::ProtocolId, WindowState::TimeOut);
	flow_control::FlowControl WindowState::flowControl;
	net::Address WindowState::server_address;
	sf::Text WindowState::connection_performance;
	bool WindowState::tv = false;

	WindowState& WindowPregame::getInstance()
	{
		static WindowPregame instance;
		return instance;
	}

	WindowState& WindowMain::getInstance()
	{
		static WindowMain instance;
		return instance;
	}

	void WindowMenu::toggle(Window* window)
	{
		window->setState(getTransitionState());
	}

	void WindowPregame::toggle(Window* window)
	{
		window->setState(getTransitionState());
	}

	void WindowMain::toggle(Window* window)
	{
		window->setState(getTransitionState());
	}

	void Window::setState(WindowState& newState)
	{
		currentState->exit(this);  // do stuff before we change state
		currentState = &newState;  // actually change states now
		currentState->enter(this);
	}

	void Window::toggle()
	{
		currentState->toggle(this);
	}

}

