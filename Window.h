#pragma once
#include <iostream>
#include <string>
#include <functional>
#include "SFML/Graphics.hpp"
#include "AssetManager.h"
#include "FlowControl.h"
#include "Net.h"
#include "Animator.h"
#include "GameCollision.h"
#include "GameMenu.h"
#include "EventSystem/EventFactory.h"

namespace Core
{

	class WindowState;

	class Window final
	{
	private:
		std::string m_name;
		unsigned int m_height;
		unsigned int m_width;
		unsigned int m_bpp;

		sf::RenderWindow* window = nullptr;
		static Window* s_instance;

		std::function<void(Event&)> fnEventHandlerCB;
		std::function<void(sf::Time)> fnUpdateCB;
		//void(*fnUpdateCB)();
		std::function<void()> fnReleaseCB;
	public:
		Window(Window& other) = delete;
		void operator=(const Window&) = delete;

		bool running; //need to thimk about it

	protected:
		Window(std::string const& name, unsigned int height, unsigned int width, unsigned int bpp);
		void init(std::string const& name, unsigned int height, unsigned int width, unsigned int bpp);
	private:
		~Window() = default;
	private:
		__forceinline static void MouseButtonPressedHandler(sf::Vector2f coo, sf::Mouse::Button button);
		__forceinline static void MouseButtonReleasedHandler(sf::Vector2f coo, sf::Mouse::Button button);
		__forceinline static void KeyBoardKeyPressedHandler(sf::Keyboard::Key key);
		__forceinline static void KeyBoardKeyReleasedHandler(sf::Keyboard::Key key);
		__forceinline static void WindowClosedHandler();
	private:

		WindowState* currentState;

	public:
		void setfnUpdate(const std::function<void(sf::Time)>& fn) { fnUpdateCB = fn; } //need to think
		void setfnEventHandler(const std::function<void(Event&)>& fn) { fnEventHandlerCB = fn; }//need to think
		void setfnRealease(const std::function<void()>& fn) { fnReleaseCB = fn; }//need to think

		inline WindowState* getCurrentState() const { return currentState; }
		void toggle(); //need to think
		void setState(WindowState& state); //need to think

	public:
		void run(void);
		inline static Window& getInstance(std::string const& name, unsigned int height, unsigned int width, unsigned int bpp);	
		__forceinline sf::RenderWindow* getRenderWindow() { return window; };
	public:
		sf::Time DT;
	protected:
		__forceinline bool isOpen() const { return running; }
		void pollEvents() const;
		void update(sf::Time) const;
		void release() const;
		//так же функция отрисовки
	};

	Window& Window::getInstance(std::string const& name, unsigned int height, unsigned int width, unsigned int bpp) {
		if (s_instance == nullptr)
		{
			s_instance = new Window(name, height, width, bpp);
		}
		return *s_instance;
	}

	

	class WindowState
	{
	private:
		static WindowState* transitionState;
	protected:

		static //common variables
			GameCollision* gc;

		//Net
		static const int ServerPort = 30000;
		static const int ProtocolId = 0x11223344;
		static constexpr float DeltaTime = 1.0f / 30.0f;
		static constexpr float SendRate = 1.0f / 30.0f;
		static constexpr float TimeOut = 10.0f;

		static flow_control::FlowControl flowControl;
		static net::ReliableConnection connection;
		static net::Address server_address;

		static bool connected;
		static float sendAccumulator;
		static float statsAccumulator;
		static float netTimeAccumulator;
		static sf::Text connection_performance;
		static sf::Font con_perm_font;
		static bool tv;

		static AssetManager manager;
		static WindowState& getTransitionState() { return *transitionState; }
		static void setTransitionState(WindowState& to) { 
			transitionState = &to;
		}
		static WindowState& wWindowMain;
		static WindowState& wWindowPregame;
		static WindowState& wWindowMenu;
	public:
		virtual void enter(Window* window) = 0;
		virtual void toggle(Window* window) = 0;
		virtual void exit(Window* window) = 0;
		virtual ~WindowState() {}
	};

	class WindowMenu : WindowState
	{
	private:

		sf::Sprite backSprite;
		Animator backAnim = Animator(backSprite);
		sf::RectangleShape backk;
		sf::Texture backkTexture;

		WindowMenu()
		{
			backk.setPosition(0, 0);
			backk.setSize(sf::Vector2f(1920, 1080));
			backk.setFillColor(sf::Color::White);
			auto backSpriteSize = sf::Vector2i(500, 288);
			auto& idleForward = backAnim.creteAnimation("idleForward", "images/menu_back.png", sf::seconds(16), true);
			idleForward.AddFrames(sf::Vector2i(0, 0), backSpriteSize, 16, 8);
			backSprite.setPosition(100, (1080-288)/2);
		}
		WindowMenu(const WindowMenu& other);
		WindowMenu& operator=(const WindowMenu& other);
	public:
		void enter(Window* window) override 
		{
			window->setfnEventHandler([this, window](Event& e) { //dispatcher
						if (e.getType() == Event::EventType::WINDOWCLOSED)
						{
							window->getRenderWindow()->close();
							window->running = false;
						}
						if (e.getType() == Event::EventType::MOUSEBUTTONPRESSED) 
						{
							setTransitionState(wWindowPregame);
							window->toggle();
						}
						});
			window->setfnUpdate([this](sf::Time time) {

				backAnim.Update(time);

				});
			window->setfnRealease([this, window]() {
				window->getRenderWindow()->clear();
			window->getRenderWindow()->draw(backk);
				window->getRenderWindow()->draw(backSprite);
				window->getRenderWindow()->display();
				});
		}
		void toggle(Window* window) override;
		void exit(Window* window) override 
		{
			std::cout << "Exit WindowMenu\n";
		}
		static WindowState& getInstance();
	};

	class WindowPregame : public WindowState
	{
	private:

		game::GameMenu* menu;

		sf::RectangleShape backk;

		WindowPregame() 
		{
			con_perm_font = manager.GetFont("fonts/PFAgoraSlabPro Bold.ttf");
			backk.setPosition(0, 0);
			backk.setSize(sf::Vector2f(1920, 1080));
			backk.setFillColor(sf::Color::White);
			backk.setTexture(&manager.GetTexture("images/pregame_background.jpg"));
		}
		WindowPregame(const WindowMenu& other);
		WindowPregame& operator=(const WindowPregame& other);
	public:
		void enter(Window* window) override
		{
			connection_performance.setFont(con_perm_font);
			connection_performance.setCharacterSize(14);
			connection_performance.setColor(sf::Color::Red);
			connection_performance.setPosition(sf::Vector2f(0, 0));

			int a, b, c, d;
			if (sscanf("192.168.49.1", "%d.%d.%d.%d", &a, &b, &c, &d))
			{
				server_address = net::Address(a, b, c, d, ServerPort);
			}
			else assert(0);
			if (!connection.Start(NULL))
			{
				printf("could not start connection on port %s\n", "30030");
				assert(0);
			}
			connection.Connect(server_address);

			std::vector<sf::String> name_menu{ L"Старт", L"Настройки", L"О игре" };
			menu = new game::GameMenu(*window->getRenderWindow(), 950, 350, 100, 120, name_menu);
			menu->setColorTextMenu(sf::Color::Red, sf::Color::Black, sf::Color::Blue);
			menu->AlignMenu(2);

			gc = new GameCollision(window->getRenderWindow());
			gc->changeGameState(GameCollision::GameState::PREPARE);

			window->setfnEventHandler([this, window](Event& e) { //dispatcher
				if (e.getType() == Event::EventType::WINDOWCLOSED)
				{
					setTransitionState(wWindowMenu);
					window->toggle();
				}
				else if (e.getType() == Event::EventType::MOUSEBUTTONPRESSED)
				{
					
				}
				else if (e.getType() == Event::EventType::KEYBOARDKEYPRESSED)
				{
					auto ev = (KeyBoardKeyPressed*)&e;
					sf::Keyboard::Key key_code = static_cast<sf::Keyboard::Key>(ev->getKeyCode());
					if (key_code == sf::Keyboard::B)
						tv ^= 1;
					else if (key_code == sf::Keyboard::Escape)
					{
						gc->changeGameState(GameCollision::GameState::PREPARE);
					}

					if (key_code == sf::Keyboard::Up) menu->MoveUp();
					else if (key_code == sf::Keyboard::Down) menu->MoveDown();

					if (key_code == sf::Keyboard::Return)
					{
						gc->changeGameState(GameCollision::GameState::WILLING);
						gc->uc = static_cast<Unit::UnitClass>(menu->getSelectedMenuNumber() + 1);
					}
				}
			});

			window->setfnUpdate([this, window](sf::Time time) {

				if (connection.IsConnected())
				flowControl.Update(DeltaTime, connection.GetReliabilitySystem().GetRoundTripTime() * 1000.0f);

				const float sendRate = flowControl.GetSendRate();
				netTimeAccumulator += window->DT.asSeconds();

				if (!connected && connection.IsConnected())
				{
					printf("client connected to server\n");
					connected = true;
				}

				if ((!connected && connection.ConnectFailed()) || (connected && connection.IsDisconnected()))
				{
					connected = false;
					netTimeAccumulator = 0.0f;
					flowControl.Reset();
					connection.Stop();
					setTransitionState(wWindowMenu);
					window->toggle();
					return;
				}
				else if (netTimeAccumulator >= DeltaTime)
				{
					netTimeAccumulator -= DeltaTime;
					sendAccumulator += DeltaTime;

					while (sendAccumulator > 1.0f / sendRate)
					{
						auto stuff = gc->getStuff();
						//printf("send state %d\n", stuff[0]);
						unsigned char* packet = new unsigned char[stuff.size()];
						for (size_t elem = 0; elem < stuff.size(); elem++)
						{
							packet[elem] = stuff.at(elem);
						}
						connection.SendPacket(packet, stuff.size());
						sendAccumulator -= 1.0f / sendRate;
						delete[] packet;
					}

					while (true)
					{
						unsigned char packet[500];
						int bytes_read = connection.ReceivePacket(packet, sizeof(packet));
						if (bytes_read == 0)
							break;
						else
						{
							//printf("recceived state %d\n", packet[0]);
							std::vector<uint8_t> received_stuff(packet, packet + bytes_read);
							gc->changeGameState(received_stuff);
						}
					}

					connection.Update(DeltaTime);

					statsAccumulator += DeltaTime;

					while (statsAccumulator >= 0.25f && connection.IsConnected())
					{
						float rtt = connection.GetReliabilitySystem().GetRoundTripTime();

						unsigned int sent_packets = connection.GetReliabilitySystem().GetSentPackets();
						unsigned int acked_packets = connection.GetReliabilitySystem().GetAckedPackets();
						unsigned int lost_packets = connection.GetReliabilitySystem().GetLostPackets();

						float sent_bandwidth = connection.GetReliabilitySystem().GetSentBandwidth();
						float acked_bandwidth = connection.GetReliabilitySystem().GetAckedBandwidth();

						printf("rtt %.1fms, sent %d, acked %d, lost %d (%.1f%%), sent bandwidth = %.1fkbps, acked bandwidth = %.1fkbps\n",
							rtt * 1000.0f, sent_packets, acked_packets, lost_packets,
							sent_packets > 0.0f ? (float)lost_packets / (float)sent_packets * 100.0f : 0.0f,
							sent_bandwidth, acked_bandwidth);
						connection_performance.setString(" rtt " + std::to_string(rtt * 1000.0f) + "fms,\n sent " + std::to_string(sent_packets) +
							",\n acked " + std::to_string(acked_packets) + ",\n lost " + std::to_string(lost_packets) +
							" (" + std::to_string(sent_packets > 0.0f ? (float)lost_packets / (float)sent_packets * 100.0f : 0.0f) +
							"%),\n sent bandwidth = " + std::to_string(sent_bandwidth) + "fkbps,\n acked bandwidth = " + std::to_string(acked_bandwidth) + "fkbps");

						statsAccumulator -= 0.25f;
					}

					if (gc->m_game_state == GameCollision::GameState::PLAYING)
					{
						setTransitionState(wWindowMain);
						window->toggle();
					}
				}
			});
			window->setfnRealease([this, window]() {
				window->getRenderWindow()->clear();
				window->getRenderWindow()->draw(backk);
				if (tv) window->getRenderWindow()->draw(connection_performance);
				menu->draw();
				window->getRenderWindow()->display();
				});
		}
		void toggle(Window* window) override;
		void exit(Window* window) override
		{
			std::cout << "Exit WindowPregame\n";
		}
		static WindowState& getInstance();
	};

	class WindowMain : WindowState
	{
	private:
		
		std::shared_ptr<EventFactory>eventFactory = EventFactory::getInstance();

		sf::View initialView;

		WindowMain()
		{
		}
		WindowMain(const WindowMain& other);
		WindowMain& operator=(const WindowMain& other);

	public:
		void enter(Window* window) override 
		{
			
			//View 
			initialView = window->getRenderWindow()->getView();
			gc->camera.operationalView = initialView;
			gc->camera.operationalView.setCenter(Collision::Instance.GetCenter(gc->m_players[gc->m_player_seq_number].m_box));
			gc->camera.miniMap.setViewport(sf::FloatRect(0.0f, 0.7f, 0.3f, 0.3f));
			gc->camera.miniMap.setSize(2720, 1530);
			gc->camera.miniMap.setCenter(Collision::Instance.GetCenter(gc->m_players[gc->m_player_seq_number].m_box));

			connection_performance.setFont(con_perm_font);
			connection_performance.setColor(sf::Color::Red);
			connection_performance.setCharacterSize(16);

			window->setfnEventHandler([this, window](Event& e) {
					gc->HandleEvents(e);

					if (e.getType() == Event::EventType::WINDOWCLOSED)
					{
						setTransitionState(wWindowMenu);
						window->toggle();
					}
					else if (e.getType() == Event::EventType::KEYBOARDKEYPRESSED)
					{
						auto ev = (KeyBoardKeyPressed*)&e;
						sf::Keyboard::Key key_code = static_cast<sf::Keyboard::Key>(ev->getKeyCode());
						if (key_code == sf::Keyboard::B)
							tv ^= 1;
						else if (key_code == sf::Keyboard::V)
						{
							if (!gc->camera.onKD) gc->camera.miniMapActivated = true;
						}
					}
					});

			window->setfnUpdate([this, window](sf::Time time) {

				if (connection.IsConnected())
				flowControl.Update(DeltaTime, connection.GetReliabilitySystem().GetRoundTripTime() * 1000.0f);

			const float sendRate = flowControl.GetSendRate();
			netTimeAccumulator += window->DT.asSeconds();

			if (!connected && connection.IsConnected())
			{
				printf("client connected to server\n");
				connected = true;
			}

			if ((!connected && connection.ConnectFailed()) || (connected && connection.IsDisconnected()))
			{
				connected = false;
				netTimeAccumulator = 0.0f;
				setTransitionState(wWindowMenu);
				window->toggle();
				return;
			}
			else if (netTimeAccumulator >= DeltaTime)
			{
				netTimeAccumulator -= DeltaTime;
				sendAccumulator += DeltaTime;

				while (sendAccumulator > 1.0f / sendRate)
				{
					auto packet = eventFactory->PackEventsToPacket();
					unsigned char* packet2 = new unsigned char[packet->getSize() + 1];
					packet2[0] = GameCollision::GameState::PLAYING;
					for (int i = 0; i < packet->getSize(); i++)
					{
						packet2[i + 1] = packet->getData()[i];
					}
					connection.SendPacket(packet2, static_cast<int>(packet->getSize() + 1));
					sendAccumulator -= 1.0f / sendRate;
					delete[] packet2;
				}

					while (true)
					{
						unsigned char packet[1024];
						int bytes_read = connection.ReceivePacket(packet, sizeof(packet));
						if (bytes_read == 0)
							break;
						else
						{
							std::vector<uint8_t> received_stuff(packet, packet + bytes_read);
							gc->deserialize(received_stuff);
						}
						
					}

					connection.Update(DeltaTime);

					statsAccumulator += DeltaTime;

					while (statsAccumulator >= 0.25f && connection.IsConnected())
					{
						float rtt = connection.GetReliabilitySystem().GetRoundTripTime();

						unsigned int sent_packets = connection.GetReliabilitySystem().GetSentPackets();
						unsigned int acked_packets = connection.GetReliabilitySystem().GetAckedPackets();
						unsigned int lost_packets = connection.GetReliabilitySystem().GetLostPackets();

						float sent_bandwidth = connection.GetReliabilitySystem().GetSentBandwidth();
						float acked_bandwidth = connection.GetReliabilitySystem().GetAckedBandwidth();

						/*printf("rtt %.1fms, sent %d, acked %d, lost %d (%.1f%%), sent bandwidth = %.1fkbps, acked bandwidth = %.1fkbps\n",
							rtt * 1000.0f, sent_packets, acked_packets, lost_packets,
							sent_packets > 0.0f ? (float)lost_packets / (float)sent_packets * 100.0f : 0.0f,
							sent_bandwidth, acked_bandwidth);*/
						connection_performance.setString(" rtt " + std::to_string(rtt * 1000.0f) + "fms,\n sent " + std::to_string(sent_packets) +
							",\n acked " + std::to_string(acked_packets) + ",\n lost " + std::to_string(lost_packets) +
							" (" + std::to_string(sent_packets > 0.0f ? (float)lost_packets / (float)sent_packets * 100.0f : 0.0f) +
							"%),\n sent bandwidth = " + std::to_string(sent_bandwidth) + "fkbps,\n acked bandwidth = " + std::to_string(acked_bandwidth) + "fkbps");

						statsAccumulator -= 0.25f;
					}
				}

				gc->Update(window->DT.asSeconds());
				connection_performance.setPosition(window->getRenderWindow()->mapPixelToCoords(sf::Vector2i(0, 0)));

				});
			window->setfnRealease([this, window]() {
					window->getRenderWindow()->setView(gc->camera.operationalView);
					window->getRenderWindow()->clear(sf::Color(13, 47, 47));
					gc->Render();
					if (tv) window->getRenderWindow()->draw(connection_performance);
					//window->getRenderWindow()->display();

					sf::RectangleShape rs;
					rs.setSize(sf::Vector2f(576, 324));
					rs.setPosition(window->getRenderWindow()->mapPixelToCoords(sf::Vector2i(0, 1080 - 324)));
					rs.setFillColor(sf::Color(13, 47, 47, 190));
					rs.setOutlineThickness(5.0f);
					rs.setOutlineColor(sf::Color(112, 128, 144, 100));
					window->getRenderWindow()->draw(rs);

					window->getRenderWindow()->setView(gc->camera.miniMap);
					//window->getRenderWindow()->clear(sf::Color(13, 47, 47));
					gc->RenderMiniMap();
					window->getRenderWindow()->display();

					window->getRenderWindow()->setView(gc->camera.operationalView);

				});

		}
		void toggle(Window* window) override;
		void exit(Window* window) override 
		{
			window->getRenderWindow()->setView(initialView);
			flowControl.Reset();
			connection.Stop();
			delete gc;
			std::cout << "Exit WindowMain\n";
		}
		static WindowState& getInstance();
	};

}

//static Window* s_instance;
//Window* Window::s_instance = nullptr;
//creating only one ( first ) time; it doesn't make sence either variable is local or global

