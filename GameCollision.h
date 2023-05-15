#pragma once
#include "SFML/Graphics.hpp"
#include <vector>
#include "Collision/Collider.hpp"
#include "Collision/Utilities.hpp"
#include "Collision/Collision.hpp"
#include "EventSystem/Event.h"
#include <string>
#include "../Networking/CourseServer/Units/Units.h"
#include "Flag.h"
#include "AssetManager.h"

class Transform2D
{
public:
	Transform2D(sf::Vector2i x, sf::Vector2i y)
		:
		x(x),
		y(y) {}
	Transform2D()
	{}
	sf::Vector2i x;
	sf::Vector2i y;
};

class Map
{
public:
	Map(int32_t tile_size, sf::Vector2i hex_map_size)
		:
		m_tile_size(tile_size),
		m_short((int32_t)(tile_size* sqrt(3) / 2)),
		m_long((int32_t)(tile_size / 2)),
		grid_basis(Transform2D(sf::Vector2i(m_short, 0), sf::Vector2i(0, m_long))),
		cw(m_short),
		ch(m_long),
		hex_basis(Transform2D(grid_basis.x * 2, grid_basis.x + grid_basis.y * 3)),
		m_hex_map_size(hex_map_size),
		m_diagonal(m_hex_map_size.x * 2 - 1),
		m_grid_map_size(sf::Vector2i(m_diagonal * 2, m_diagonal * 3 + 1)),
		m_offset(sf::Vector2i(-(m_short * 4), -(m_long * 4))) {}

	Map()
	{
		grid_basis = Transform2D();
		hex_basis = Transform2D();
	}

	void init(int32_t tile_size, sf::Vector2i hex_map_size)
	{
		m_tile_size = tile_size;
		m_short = (int32_t)(tile_size * sqrt(3) / 2);
		m_long = (int32_t)(tile_size / 2);
		grid_basis = Transform2D(sf::Vector2i(m_short, 0), sf::Vector2i(0, m_long));
		cw = m_short;
		ch = m_long;
		hex_basis = Transform2D(grid_basis.x * 2, grid_basis.x + grid_basis.y * 3);
		m_hex_map_size = hex_map_size;
		m_diagonal = m_hex_map_size.x * 2 - 1;
		m_grid_map_size = sf::Vector2i(m_diagonal * 2, m_diagonal * 3 + 1),
		m_offset = sf::Vector2i(-(m_short * 4), -(m_long * 4));
	}

	int32_t m_tile_size;
	int32_t m_short;
	int32_t m_long;
	Transform2D grid_basis;
	Transform2D hex_basis;
	int32_t cw; // length of horizontal basis
	int32_t ch; // length of vertical basis
	sf::Vector2i m_hex_map_size;
	int m_diagonal;
	sf::Vector2i m_grid_map_size;
	sf::Vector2i m_offset;

	sf::Vector2i hex2pixel(sf::Vector2i hex) { return hex.x * hex_basis.x + hex.y * hex_basis.y - m_offset; };
	sf::Vector2i* _get_hor_hex_vertices(sf::Vector2i hex)
	{
		auto pixel = hex2pixel(hex);
		sf::Vector2i* result = new sf::Vector2i[6]{
			pixel + grid_basis.x - grid_basis.y,
			pixel + grid_basis.x + grid_basis.y,
			pixel + 2 * grid_basis.y,
			pixel - grid_basis.x + grid_basis.y,
			pixel - grid_basis.x - grid_basis.y,
			pixel - 2 * grid_basis.y
		};
		return result;
	}

	sf::Vector2i round_hex(sf::Vector2f hex)
	{
		auto rx = round(hex.x);
		auto ry = round(hex.y);
		auto rz = round(-hex.x - hex.y);

		auto x_diff = abs(hex.x - rx);
		auto y_diff = abs(hex.y - ry);
		auto z_diff = abs(-hex.x - hex.y - rz);

		if ((x_diff > y_diff) && (x_diff > z_diff))
		{
			rx = -ry - rz;
		}
		else if (y_diff > z_diff)
		{
			ry = -rx - rz;
		}
		return sf::Vector2i(rx, ry);
	}

	sf::Vector2i pixel2hex(sf::Vector2i pixel)
	{
		float x = (float)(pixel.x + m_offset.x) / (2 * cw) - (float)(pixel.y + m_offset.y) / (6 * ch);
		float y = (float)(pixel.y + m_offset.y) / (3 * ch);
		return round_hex(sf::Vector2f(x, y));
	}

	void draw_line(sf::RenderTexture& tx, sf::Vector2i start, sf::Vector2i end, sf::Color c)
	{
		sf::Vertex line[]
		{
			sf::Vertex(sf::Vector2f(start.x, start.y), c),
			sf::Vertex(sf::Vector2f(end.x, end.y), c)
		};
		tx.draw(line, 2, sf::LineStrip);
	}

	void _draw_hor_hex_filled(sf::Vector2i hex, sf::RenderWindow& tx, sf::Color c)
	{
		auto points_p = _get_hor_hex_vertices(hex);
		sf::ConvexShape cs;
		cs.setFillColor(c);
		cs.setPointCount(6);
		for (size_t i = 0; i < 6; i++)
		{
			cs.setPoint(i, sf::Vector2f((points_p + i)->x, (points_p + i)->y));
		}
		delete[] points_p;
		tx.draw(cs);
	}

	void _draw_hor_hex(sf::Vector2i hex, sf::RenderWindow& tx, sf::Color c)
	{
		auto points_p = _get_hor_hex_vertices(hex);
		sf::VertexArray va;
		for (size_t i = 0; i < 6; i++)
		{
			float x = (points_p + i)->x;
			float y = (points_p + i)->y;
			va.append(sf::Vertex(sf::Vector2f(x, y), c));
		}
		va.append(va[0]);
		delete[] points_p;

		tx.draw(&va[0], 7, sf::LinesStrip, sf::BlendMultiply);
	}

	void _draw_hor_hex_grid(sf::RenderTexture& tx, sf::Color color, float width = 1.0f, bool antialiasing = false)
	{
		int parity = int(m_hex_map_size.x) % 2;
		sf::Vector2i offset = grid_basis.x + grid_basis.y * 2 + grid_basis.x * (1 - parity) + m_offset;
		int start;
		for (int i = 0; i < m_grid_map_size.y / 2; i += 3)
		{
			start = m_hex_map_size.x - i / 3 - 1;
			for (int j = start; j < m_grid_map_size.x / 2; j++)
			{
				if (abs(i % 2 - j % 2) != parity)
				{
					draw_line(tx, grid_basis.x * j + grid_basis.y * (i + 1) - offset, grid_basis.x * (j + 1) + grid_basis.y * i - offset, color);
					draw_line(tx, grid_basis.x * (m_grid_map_size.x - j) + grid_basis.y * (i + 1) - offset, grid_basis.x * (m_grid_map_size.x - j - 1) + grid_basis.y * i - offset, color);
					draw_line(tx, grid_basis.x * j + grid_basis.y * (m_grid_map_size.y - i - 1) - offset, grid_basis.x * (j + 1) + grid_basis.y * (m_grid_map_size.y - i) - offset, color);
					draw_line(tx, grid_basis.x * (m_grid_map_size.x - j) + grid_basis.y * (m_grid_map_size.y - i - 1) - offset, grid_basis.x * (m_grid_map_size.x - j - 1)
						+ grid_basis.y * (m_grid_map_size.y - i) - offset, color);
				}
				else
				{
					draw_line(tx, grid_basis.x * (j)+grid_basis.y * (i)-offset, grid_basis.x * (j + 1) + grid_basis.y * (i + 1) - offset, color);
					draw_line(tx, grid_basis.x * (m_grid_map_size.x - j) + grid_basis.y * (i)-offset, grid_basis.x * (m_grid_map_size.x - j - 1) + grid_basis.y * (i + 1) - offset, color);
					draw_line(tx, grid_basis.x * (j)+grid_basis.y * (m_grid_map_size.y - i) - offset, grid_basis.x * (j + 1) + grid_basis.y * (m_grid_map_size.y - i - 1) - offset, color);
					draw_line(tx, grid_basis.x * (m_grid_map_size.x - j) + grid_basis.y * (m_grid_map_size.y - i) - offset, grid_basis.x * (m_grid_map_size.x - j - 1)
						+ grid_basis.y * (m_grid_map_size.y - i - 1) - offset, color);
				}
			}
		}
		for (int i = 1; i < m_grid_map_size.y; i += 3)
		{
			if (i <= m_grid_map_size.y / 2)
			{
				start = m_hex_map_size.x - 1 - i / 3;
			}
			else
			{
				start = (i - m_grid_map_size.y / 2) / 3;
			}
			for (int j = abs(parity - i % 2); j < m_grid_map_size.x + 1; j += 2)
			{
				if ((j >= start) && (j <= m_grid_map_size.x - start))
					draw_line(tx, grid_basis.x * j + grid_basis.y * i - offset, grid_basis.x * j + grid_basis.y * (i + 2) - offset, color);
			}
		}

	}

	int hex_distance(sf::Vector2i hex1, sf::Vector2i hex2)
	{
		auto dif = (hex2 - hex1);
		return (abs(dif.x) + abs(dif.y) + abs(-dif.x - dif.y)) / 2;
	}

	sf::Vector2i _get_hor_hex_map_center()
	{
		return sf::Vector2i((m_hex_map_size.x - 1) / 2, m_hex_map_size.x - 1);
	}

	bool _is_in_hex_grid(sf::Vector2i hex)
	{
		auto center = _get_hor_hex_map_center();
		auto dist = hex_distance(center, hex);
		return dist < m_hex_map_size.x;
	}
};

class GameCollision final
{
public:

	enum GameState
	{
		IDLE = 0,
		PREPARE,
		WILLING,
		RELEASING,
		LISTINING,
		LOADED,
		WAITING,
		PLAYING
	};

	GameState m_game_state;

	int32_t m_player_seq_number;

	Unit::UnitClass uc;

	class Player
	{
	public:
		enum PlayerDirection : uint8_t
		{
			NONE = 0,
			UP = 0b0001,
			DOWM = 0b0100,
			RIGHT = 0b0010,
			LEFT = 0b1000,
			UPRIGHT = 0b0011,
			UPLEFT = 0b1001,
			DOWNRIGHT = 0b0110,
			DOWNLEFT = 0b1100
		};

		PlayerDirection direction = PlayerDirection::NONE;
		int vert_speed = 10;
		int hor_speed = 10;

		float m_map_x = 0;
		float m_map_y = 0;

		float m_attack_radius;

		sf::Vector2i hex_number;

		float m_flag = 0.0f;
		float m_deflag = 0.0f;

		Unit::UnitClass uc;
		Unit* m_unit;
		CircleCollider m_box;

		sf::CircleShape m_cs;
		sf::Texture m_cs_t;

		//other stuff
		void serialize(std::vector<uint8_t>& buffer, int16_t& it)
		{
			Core::encode(buffer, it, m_box.GetPosition().x);
			Core::encode(buffer, it, m_box.GetPosition().y);
		}

		void deserialize(std::vector<uint8_t>& buffer, int16_t& iterator)
		{
			m_map_x = Core::decode<float>(buffer, iterator);
			m_map_y = Core::decode<float>(buffer, iterator);
			m_box.SetPosition(sf::Vector2f(m_map_x, m_map_y));
			m_cs.setPosition(m_box.GetPosition() - sf::Vector2f(50.0f, 50.0f));
			int hex_num_x = Core::decode<int>(buffer, iterator);
			int hex_num_y = Core::decode<int>(buffer, iterator);
			hex_number = sf::Vector2i(hex_num_x, hex_num_y);
			m_attack_radius = Core::decode<float>(buffer, iterator);
			m_flag = Core::decode<float>(buffer, iterator);
			m_deflag = Core::decode<float>(buffer, iterator);
			//let it be for now
			m_unit->deserialize(buffer, iterator);
			
		}
		static size_t getSerializingSize()
		{
			constexpr size_t size = sizeof(m_map_x) + sizeof(m_map_y);
			return size;
		}
	} ;

	int32_t m_players_count;
	Player* m_players;

	void deserialize(std::vector<uint8_t>& buffer)
	{
		int16_t iterator = 0;
		int32_t player_count = Core::decode<int32_t>(buffer, iterator);
		for (size_t i = 0; i < m_players_count; i++)
		{
			m_players[i].deserialize(buffer, iterator);
		}
		size_t flags_count = Core::decode<size_t>(buffer, iterator);
		m_flags.resize(flags_count);
		for (size_t i = 0; i < flags_count; i++)
		{
			m_flags[i].deserialize(buffer, iterator);
		}
	}

	struct Camera
	{
		sf::View operationalView;
		sf::View miniMap;
		float miniMapActivated = false;
		const float miniMapSkillTime = 2.5f;
		float miniMapAccumulator = 0.0f;
		bool onKD = false;
		const float miniMapKDTime = 10.0f;
		float miniMapKDAccumulator = 0.0f;

		sf::Vector2f center = sf::Vector2f(1920 / 2, 1080 / 2);
		sf::Vector2f topCenter = sf::Vector2f(1920 / 2, 300);
		sf::Vector2f bottomCenter = sf::Vector2f(1920 / 2, 780);
		sf::Vector2f rightCenter = sf::Vector2f(1320, 1080 / 2);
		sf::Vector2f leftCenter = sf::Vector2f(600, 1080 / 2);

		sf::Vector2f topleft = sf::Vector2f(600, 300);
		sf::Vector2f topright = sf::Vector2f(1320, 300);
		sf::Vector2f bottomleft = sf::Vector2f(600, 780);
		sf::Vector2f bottomright = sf::Vector2f(1320, 780);
		sf::View& getView() { return operationalView; }
		void inCoordinates(sf::Vector2i bc, float dt);
	} camera;

	//GameCollision(sf::RenderWindow*);
	GameCollision(sf::RenderWindow* rw)
		:
		m_game_state(GameState::IDLE),
		m_stuff(std::vector<uint8_t>(500)),
		Window(rw)
	{}

	void changeGameState(std::vector<uint8_t>& stuff)
	{
		GameState stuff_state = static_cast<GameCollision::GameState>(stuff[0]);
		if (stuff_state > m_game_state)
		{
			m_game_state = stuff_state;
			m_stuff[0] = m_game_state;
		}
		else if (stuff_state == m_game_state)
			return;

		if (m_game_state == GameState::RELEASING)
		{
			user_init_data_to_server();
		}
		if (m_game_state == GameState::LOADED)
		{
			user_get_pregame_data_from_server(stuff);
		}
		if (m_game_state == GameState::PLAYING)
		{
			deserialize(stuff);
		}

	}

	void changeGameState(GameState gc)
	{
		m_game_state = gc;
		m_stuff[0] = gc;
	}

	std::vector<uint8_t>& getStuff()
	{
		return m_stuff;
	}

	GameState getGameState() const
	{
		return m_game_state;
	}

	~GameCollision();

	void HandleEvents(::Event&);
	void Update(float);
	void Render();
	void RenderMiniMap();

private:
	void CreateColliders(sf::Vector2i box_hex, sf::Vector2i border_size);

	void CheckCollision();
	void UpdateBox(float);

	Edge GetCurCircleEdge(CircleCollider*);
	void InitEdges(sf::Vector2i border_size);

	void user_init_data_to_server()
	{
		int16_t iterator{};
		Core::encode(m_stuff, iterator, static_cast<int8_t>(m_game_state));
		Core::encode<int8_t>(m_stuff, iterator, uc);
	}

	void user_get_pregame_data_from_server(std::vector<uint8_t>& stuff)
	{
		int16_t iterator{};
		Core::decode<int8_t>(stuff, iterator);
		m_player_seq_number = Core::decode<int32_t>(stuff, iterator);
		printf("pre stuff %d\n", m_player_seq_number);
		std::string str = std::to_string(m_player_seq_number);
		Window->setTitle(str);
		int _init_player_hex_x = Core::decode<int>(stuff, iterator);
		int _init_player_hex_y = Core::decode<int>(stuff, iterator);
		sf::Vector2i _init_player_hex(_init_player_hex_x, _init_player_hex_y);

		sf::Vector2i hex_map_size = sf::Vector2i(Core::decode<int>(stuff, iterator), 0);
		int32_t tile_size = Core::decode<int32_t>(stuff, iterator);
		m_players_count = Core::decode<int32_t>(stuff, iterator);
		m_players = new Player[m_players_count];

		for (size_t i = 0; i < m_players_count; i++)
		{
			auto& player = m_players[i];
			auto _unit_type = static_cast<Unit::UnitClass>(Core::decode<int8_t>(stuff, iterator));
			player.uc = _unit_type;
			AssetManager* manager = AssetManager::getInstance();
			player.m_cs.setRadius(50.0f);
			switch (_unit_type)
			{
			case Unit::LIGHT: 
				player.m_unit = new Light();
				player.m_cs.setTexture(&manager->GetTexture("images/blue_dragon.jpg"));
				break;
			case Unit::AV:
				player.m_unit = new AV();
				player.m_cs.setTexture(&manager->GetTexture("images/Spikey_Dragon_Stone.jpg"));
				break;
			case Unit::STRONG:
				player.m_unit = new Strong();
				player.m_cs.setTexture(&manager->GetTexture("images/Bone_Dragon_Stone.jpg"));
				break;
			default:
				break;
			}
		}

		init_game(tile_size, hex_map_size, _init_player_hex);
	}

	void init_game(int32_t tile_size, sf::Vector2i hex_map_size, sf::Vector2i _init_player_hex)
	{
		map.init(tile_size, hex_map_size); //650
		auto y = map.hex2pixel(_init_player_hex);
		auto x = map.hex2pixel(map._get_hor_hex_map_center());
		x *= 2;
		rt.create(x.x, x.y);

		CreateColliders(_init_player_hex, x);

		for (size_t i = 0; i < m_players_count; i++)
		{
			if (i != m_player_seq_number)
			{
				m_players[i].m_box.Create(50.0f);
				m_players[i].m_box.SetOrigin(sf::Vector2f(50.0f, 50.0f));
			}
		}

		for (auto& c : m_Colliders)
			rt.draw(*c);
		map._draw_hor_hex_grid(rt, sf::Color::White);

		rt.display();
	}

private:
	std::vector<uint8_t> m_stuff;

	sf::RenderWindow* Window;

	std::vector<Collider*> m_Colliders;
	std::vector<CircleCollider*> m_CircleColliders;

	std::vector<Edge> m_Edges;
	std::vector<Edge> m_MiniMapEdges;

	std::vector<Flag> m_flags;

	std::vector<Point> Points;
	std::vector<Point> MiniMapPoints;

	Map map;
	sf::RenderTexture rt;
	sf::RenderTexture castTexture;
};

