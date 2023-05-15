#include "GameCollision.h"

void GameCollision::Camera::inCoordinates(sf::Vector2i bc, float dt)
{
	if (miniMapActivated)
	{
		miniMapAccumulator += dt;
		if (miniMapAccumulator >= miniMapSkillTime)
		{
			miniMapActivated = false;
			miniMapAccumulator = 0.0f;
			onKD = true;
			miniMapKDAccumulator = 0.0f;
		}
	}
	else if (onKD)
	{
		miniMapKDAccumulator += dt;
		if (miniMapKDAccumulator >= miniMapKDTime)
		{
			onKD = false;
			miniMapKDAccumulator = 0.0f;
		}
	}

	constexpr float smoothness = 2.0f;
	sf::Vector2f result;
	sf::Vector2f oper_center;

	if (!((bc.y > topCenter.y) && (bc.y < bottomCenter.y) && (bc.x > leftCenter.x) && (bc.x < rightCenter.x)))
	{
		if (bc.y < topCenter.y)
		{
			if (bc.x < topright.x)
			{
				if (bc.x < topleft.x)
				{
					oper_center = topleft;
				}
				else
				{
					oper_center = topCenter;
				}
			}
			else
			{
				oper_center = topright;
			}
		}
		else if (bc.y > bottomCenter.y)
		{
			if (bc.x < bottomright.x)
			{
				if (bc.x < bottomleft.x)
				{
					oper_center = bottomleft;
				}
				else
				{
					oper_center = bottomCenter;
				}
			}
			else
			{
				oper_center = bottomright;
			}
		}
		else
		{
			if (bc.x < leftCenter.x)
			{
				oper_center = leftCenter;
			}
			else
			{
				oper_center = rightCenter;
			}
		}

		sf::Vector2f bc_float = sf::Vector2f(bc.x, bc.y);
		result = Lerp(oper_center, bc_float, smoothness * dt);
		result -= oper_center;
		operationalView.move(result);
		miniMap.move(result);
	}
}


//GameCollision::GameCollision(sf::RenderWindow* rw)
//{
//
//	m_game_state = GameState::IDLE;
//
//	map.init(250, sf::Vector2i(5, 0)); //650
//	auto y = map.hex2pixel(sf::Vector2i(2, 2));
//	auto x = map.hex2pixel(map._get_hor_hex_map_center());
//	x *= 2;
//	rt.create(x.x, x.y);
//
//	this->Window = rw;
//	CreateColliders(sf::Vector2i(0, 0), x);
//
//	for (auto& c : m_Colliders)
//		rt.draw(*c);
//	map._draw_hor_hex_grid(rt, sf::Color::White);
//
//	rt.display();
//}

GameCollision::~GameCollision()
{
	for (auto& c : m_Colliders)
		delete c;
	delete[] m_players;
}

void GameCollision::HandleEvents(::Event&)
{
}

#define client
#define server

void GameCollision::Update(float dt) 
{
	//server UpdateBox(dt);
	//server CheckCollision();

	Flag::updateFlagAlpha(dt);

	client
	{
		sf::Vector2f mousePos = Collision::Instance.GetCenter(m_players[m_player_seq_number].m_box);
		sf::Vector2i pix = Window->mapCoordsToPixel(mousePos);
		camera.inCoordinates(pix, dt);
		Points.clear();
		MiniMapPoints.clear();

		int16_t edges_count = 0;

		for (auto* circle : m_CircleColliders)
		{
			auto edge = GetCurCircleEdge(circle);
			m_Edges.emplace_back(edge);
			edges_count++;
		}

		int16_t edges_smoke_count = 0;
		for (size_t i = 0; i < m_players_count; i++)
		{
			auto& player = m_players[i];
			if (player.uc == Unit::LIGHT)
			{
				auto unit = static_cast<Light*>(player.m_unit);
				int16_t edges_c = 0;
				auto edges = unit->getEdges(edges_c);
				if (edges_c != 0)
				{
					edges_smoke_count += edges_c;
					for (size_t i = 0; i < edges_c; i++)
					{
						m_Edges.push_back(edges[i]);
						m_MiniMapEdges.push_back(edges[i]);
					}
					delete[] edges;
				}
			}
		}

		for (auto& e0 : m_Edges) {
			// Because we have two points in every edge we need to iterate through it like this
			for (uint32_t i = 0; i < 2; i++) {
				// Calculating vector between mouse and point of our edge
				sf::Vector2f rd((!i ? e0.Start : e0.End) - mousePos);

				float baseAng = atan2f(rd.y, rd.x);

				float ang = 0.0f;

				// For casting aditional rays
				for (uint32_t j = 0; j < 3; j++) {
					if (j == 0)	ang = baseAng - 0.0001f;
					if (j == 1)	ang = baseAng;
					if (j == 2)	ang = baseAng + 0.0001f;

					rd.x = 100.0f * cosf(ang);
					rd.y = 100.0f * sinf(ang);

					sf::Vector2f minP;
					float		 minT1 = 9999.0f;
					float		 minAng = 0.0f;
					bool		 valid = false;

					for (auto& e1 : m_Edges) {

						sf::Vector2f sd = e1.End - e1.Start;

						float t2 = (rd.x * (e1.Start.y - mousePos.y) + (rd.y * (mousePos.x - e1.Start.x))) / (sd.x * rd.y - sd.y * rd.x);

						float t1 = (e1.Start.x + sd.x * t2 - mousePos.x) / rd.x;

						if (t1 > 0 && t2 >= 0 && t2 <= 1.0f) {
							if (t1 < minT1) {
								// If vectors cross we add thier intersection point to our vector 

								minT1 = t1;
								minP = mousePos + rd * t1;
								minAng = atan2f(minP.y - mousePos.y, minP.x - mousePos.x);

								valid = true;
							}
						}
					}

					if (valid)
						Points.emplace_back(sf::Vector2f(minP.x, minP.y), minAng);
				}
			}
		}
		// We need to sort our points based on thier angle to draw them correctly
		std::sort(Points.begin(), Points.end(), [&](const Point& a, const Point& b) { return a.Angle < b.Angle; });

		for (size_t i = 0; i < edges_count + edges_smoke_count; i++)
		{
			m_Edges.pop_back();
		}


		for (auto& e0 : m_MiniMapEdges) {
			// Because we have two points in every edge we need to iterate through it like this
			for (uint32_t i = 0; i < 2; i++) {
				// Calculating vector between mouse and point of our edge
				sf::Vector2f rd((!i ? e0.Start : e0.End) - mousePos);

				float baseAng = atan2f(rd.y, rd.x);

				float ang = 0.0f;

				// For casting aditional rays
				for (uint32_t j = 0; j < 3; j++) {
					if (j == 0)	ang = baseAng - 0.0001f;
					if (j == 1)	ang = baseAng;
					if (j == 2)	ang = baseAng + 0.0001f;

					rd.x = 100.0f * cosf(ang);
					rd.y = 100.0f * sinf(ang);

					sf::Vector2f minP;
					float		 minT1 = 9999.0f;
					float		 minAng = 0.0f;
					bool		 valid = false;

					for (auto& e1 : m_MiniMapEdges) {

						sf::Vector2f sd = e1.End - e1.Start;

						float t2 = (rd.x * (e1.Start.y - mousePos.y) + (rd.y * (mousePos.x - e1.Start.x))) / (sd.x * rd.y - sd.y * rd.x);

						float t1 = (e1.Start.x + sd.x * t2 - mousePos.x) / rd.x;

						if (t1 > 0 && t2 >= 0 && t2 <= 1.0f) {
							if (t1 < minT1) {
								// If vectors cross we add thier intersection point to our vector 

								minT1 = t1;
								minP = mousePos + rd * t1;
								minAng = atan2f(minP.y - mousePos.y, minP.x - mousePos.x);

								valid = true;
							}
						}
					}

					if (valid)
						MiniMapPoints.emplace_back(sf::Vector2f(minP.x, minP.y), minAng);
				}
			}
		}
		// We need to sort our points based on thier angle to draw them correctly
		std::sort(MiniMapPoints.begin(), MiniMapPoints.end(), [&](const Point& a, const Point& b) { return a.Angle < b.Angle; });


		for (size_t i = 0; i < edges_smoke_count; i++)
		{
			m_MiniMapEdges.pop_back();
		}

	}
}

void GameCollision::RenderMiniMap()
{

	if (m_players[m_player_seq_number].uc != Unit::LIGHT)
	{
		castTexture.clear(sf::Color::Black);
		std::vector<sf::Vertex> Vertices;
		Vertices.emplace_back(Collision::Instance.GetCenter(m_players[m_player_seq_number].m_box));

		for (uint32_t i = 0; i < MiniMapPoints.size(); i++)
			Vertices.emplace_back(MiniMapPoints[i].Pos);

		if (!MiniMapPoints.empty())
			Vertices.emplace_back(MiniMapPoints[0].Pos);

		castTexture.draw(Vertices.data(), Vertices.size(), sf::TriangleFan);
	}
	else castTexture.clear(sf::Color::White);
	castTexture.display();


	map._draw_hor_hex_filled(m_players[m_player_seq_number].hex_number, *Window, sf::Color(63, 112, 77, 30));

	const float _flag_radius = 10;
	sf::CircleShape flag;
	flag.setRadius(_flag_radius*2);
	flag.setOutlineThickness(5);
	for (size_t i = 0; i < m_flags.size(); i++)
	{
		flag.setPosition(m_flags[i].m_pos);
		if (m_flags[i].m_player_seq_num == m_player_seq_number)
		{
			flag.setFillColor(sf::Color::Green);
			flag.setOutlineColor(sf::Color(255, 0, 0, Flag::alpha));
			map._draw_hor_hex_filled(m_flags[i].m_hex_num, *Window, sf::Color(63, 112, 77));
			Window->draw(flag);
		}
		else if (camera.miniMapActivated)
		{
			map._draw_hor_hex_filled(m_flags[i].m_hex_num, *Window, sf::Color(161, 40, 48, static_cast<uint8_t>(255 - camera.miniMapAccumulator * 255 / camera.miniMapSkillTime)));
		}
	}


	sf::CircleShape unit;
	unit.setRadius(50.0f);
	unit.setOutlineThickness(8);
	if (camera.miniMapActivated)
	{
		unit.setOutlineColor(sf::Color(0, 0, 0, static_cast<uint8_t>(255 - camera.miniMapAccumulator * 255 / camera.miniMapSkillTime)));
		unit.setFillColor(sf::Color(255, 0, 0, static_cast<uint8_t>(255 - camera.miniMapAccumulator * 255 / camera.miniMapSkillTime)));
		for (size_t i = 0; i < m_players_count; i++)
		{
			if (m_player_seq_number != i)
			{
				unit.setPosition(m_players[i].m_cs.getPosition());
				Window->draw(unit);
			}	
		}
	}	
	unit.setOutlineColor(sf::Color::Black);
	unit.setPosition(m_players[m_player_seq_number].m_cs.getPosition());
	unit.setFillColor(sf::Color(0, 255, 0));
	Window->draw(unit);
	

	Window->draw(sf::Sprite(castTexture.getTexture()), sf::BlendMultiply);
	
	//Window->draw(sf::Sprite(rt.getTexture()));

}

void GameCollision::Render()
{
	castTexture.clear();

	std::vector<sf::Vertex> Vertices;
	Vertices.emplace_back(Collision::Instance.GetCenter(m_players[m_player_seq_number].m_box));

	for (uint32_t i = 0; i < Points.size(); i++)
		Vertices.emplace_back(Points[i].Pos);

	if (!Points.empty())
		Vertices.emplace_back(Points[0].Pos);

	castTexture.draw(Vertices.data(), Vertices.size(), sf::TriangleFan);
	castTexture.display();

	// previously draw hero here

	const float _flag_radius = 10;

	sf::CircleShape flag_radius;
	flag_radius.setRadius(Flag::m_tack_off_radius);
	flag_radius.setFillColor(sf::Color(0, 0, 0, 0));
	flag_radius.setOrigin(sf::Vector2f(Flag::m_tack_off_radius - _flag_radius, Flag::m_tack_off_radius - _flag_radius));
	flag_radius.setOutlineThickness(2);
	flag_radius.setOutlineColor(sf::Color(150, 75, 0, 55));

	sf::CircleShape flag;
	flag.setRadius(_flag_radius);
	flag.setOutlineThickness(5);
	for (size_t i = 0; i < m_flags.size(); i++)
	{
		flag.setPosition(m_flags[i].m_pos);
		flag_radius.setPosition(m_flags[i].m_pos);
		if (m_flags[i].m_player_seq_num == m_player_seq_number)
		{
			flag.setFillColor(sf::Color::Green);
			flag.setOutlineColor(sf::Color(255, 0, 0, Flag::alpha));
		}
		else
		{
			flag.setFillColor(sf::Color::Red);
			flag.setOutlineColor(sf::Color(0, 255, 0, Flag::alpha));
		}
		Window->draw(flag);
		Window->draw(flag_radius);
	}

	sf::CircleShape attack_cs;
	attack_cs.setFillColor(sf::Color(0, 0, 0, 0));
	attack_cs.setOutlineColor(sf::Color::Red);
	attack_cs.setOutlineThickness(2);
	for (size_t i = 0; i < m_players_count; i++)
	{
		//Window->draw(m_players[i].m_box);
		Window->draw(m_players[i].m_cs);
		attack_cs.setRadius(m_players[i].m_unit->attack_radius);
		attack_cs.setPosition(m_players[i].m_box.GetPosition() - sf::Vector2f(attack_cs.getRadius(), attack_cs.getRadius()));
		Window->draw(attack_cs);
	}

	sf::RectangleShape border;
	border.setSize(sf::Vector2f(100, 10));
	border.setFillColor(sf::Color(0, 0, 0, 0));
	border.setPosition(0, 0);
	border.setOutlineThickness(2);
	border.setOutlineColor(sf::Color::White);

	sf::RectangleShape inside;
	inside.setFillColor(sf::Color::Green);
	inside.setPosition(0, 0);

	for (size_t i = 0; i < m_players_count; i++)
	{
		if (m_players[i].m_flag > 0.0001)
		{
			border.setPosition(m_players[i].m_box.GetPosition() - sf::Vector2f(50.0f, 65.0f));
			inside.setPosition(m_players[i].m_box.GetPosition() - sf::Vector2f(50.0f, 65.0f));
			inside.setSize(sf::Vector2f(100 * m_players[i].m_flag, 10));
			inside.setFillColor(sf::Color::Green);
			Window->draw(border);
			Window->draw(inside);
		}
		if (m_players[i].m_deflag > 0.0001)
		{
			border.setPosition(m_players[i].m_box.GetPosition() - sf::Vector2f(50.0f, 65.0f));
			inside.setPosition(m_players[i].m_box.GetPosition() - sf::Vector2f(50.0f, 65.0f));
			inside.setSize(sf::Vector2f(100 * m_players[i].m_deflag, 10));
			inside.setFillColor(sf::Color::Red);
			Window->draw(border);
			Window->draw(inside);
		}
			
	}


	Window->draw(sf::Sprite(castTexture.getTexture()), sf::BlendMultiply);
	Window->draw(sf::Sprite(rt.getTexture()));

	map._draw_hor_hex(m_players[m_player_seq_number].hex_number, *Window, sf::Color::Red);

}

Edge GameCollision::GetCurCircleEdge(CircleCollider* circle)
{
	Edge result;

	float BC = circle->GetRadius();
	sf::Vector2f B = circle->GetPosition();
	sf::Vector2f A = Collision::Instance.GetCenter(m_players[m_player_seq_number].m_box);
	float AB = sqrtf(powf((B.x - A.x), 2.0f) + powf((B.y - A.y), 2.0f));
	float CA = sqrtf(powf(AB, 2.0f) - powf(BC, 2.0f));
	float cosB = BC / AB;
	float sinB = CA / AB;

	sf::Vector2f v1;
	v1.x = (A.x - B.x) / AB;
	v1.y = (A.y - B.y) / AB;

	sf::Vector2f v2_1, v2_2;
	//1
	v2_1.x = v1.x * cosB - v1.y * sinB;
	v2_1.y = v1.x * sinB + v1.y * cosB;
	//2
	sinB *= -1;
	v2_2.x = (v1.x * cosB - v1.y * sinB);
	v2_2.y = (v1.x * sinB + v1.y * cosB);

	sf::Vector2f BC1_vec;
	BC1_vec.x = v2_1.x * BC;
	BC1_vec.y = v2_1.y * BC;
	sf::Vector2f BC2_vec;
	BC2_vec.x = v2_2.x * BC;
	BC2_vec.y = v2_2.y * BC;

	sf::Vector2f start = BC1_vec + B;
	sf::Vector2f end = BC2_vec + B;
	result = Edge(start, end);

	return result;
}

void GameCollision::CreateColliders(sf::Vector2i box_hex, sf::Vector2i border_size)
{

	auto center = sf::Vector2f(map.hex2pixel(map._get_hor_hex_map_center()));
	{
		m_players[m_player_seq_number].m_box.Create(50.0f);
		//m_players[m_player_seq_number].m_box.SetOrigin(sf::Vector2f(50.0f, 50.0f));
		auto x = Window->mapPixelToCoords(map.hex2pixel(box_hex));
		m_players[m_player_seq_number].m_box.SetPosition(sf::Vector2f(x.x, x.y));
	}

	/*{
		CircleCollider* c = new CircleCollider();

		c->Create(200.0f);

		c->SetPosition(sf::Vector2f(0, 0) - sf::Vector2f(map.m_offset.x, map.m_offset.y));

		m_Colliders.push_back(c);
		m_CircleColliders.push_back(c);
	}*/

	{
		CircleCollider* c = new CircleCollider();

		c->Create(50.0f);

		c->SetPosition(center + sf::Vector2f(920.0f, 150.0f));

		m_Colliders.push_back(c);
		m_CircleColliders.push_back(c);
	}

	{
		BoxCollider* c = new BoxCollider();

		c->Create(sf::Vector2f(200.0f, 200.0f));

		c->SetPosition(center + sf::Vector2f(650.0f, 330.0f));

		c->SetRotation(sf::degrees(45.0f));

		m_Colliders.push_back(c);
	}

	{
		BoxCollider* c = new BoxCollider();

		c->Create(sf::Vector2f(300.0f, 200.0f));

		c->SetPosition(center - sf::Vector2f(120.0f, 560.0f));

		c->SetRotation(sf::degrees(25.0f));

		m_Colliders.push_back(c);
	}

	{

		CustomCollider* c = new CustomCollider(3);

		c->GetVertices(0) = sf::Vector2f(0.0f, 0.0f);
		c->GetVertices(1) = sf::Vector2f(200.0f, 200.0f);
		c->GetVertices(2) = sf::Vector2f(200.0f, 0.0f);

		c->SetPosition(center + sf::Vector2f(1230.0f, 80.0f));

		c->SetRotation(sf::degrees(15.0f));

		m_Colliders.push_back(c);
	}

	{

		CustomCollider* c = new CustomCollider(6);

		c->GetVertices(0) = sf::Vector2f(100.0f, 0.0f);
		c->GetVertices(1) = sf::Vector2f(200.0f, 0.0f);
		c->GetVertices(2) = sf::Vector2f(260.0f, 60.0f);
		c->GetVertices(3) = sf::Vector2f(160.0f, 120.0f);
		c->GetVertices(4) = sf::Vector2f(80.0f, 120.0f);
		c->GetVertices(5) = sf::Vector2f(0.0f, 50.0f);

		c->SetPosition(center - sf::Vector2f(1340.0f, 450.0f));

		c->SetRotation(sf::degrees(15.0f));

		m_Colliders.push_back(c);
	}

	{

		CustomCollider* c = new CustomCollider(4);

		c->GetVertices(0) = sf::Vector2f(0.0f, 0.0f);
		c->GetVertices(1) = sf::Vector2f(200.0f, 200.0f);
		c->GetVertices(2) = sf::Vector2f(200.0f, 0.0f);
		c->GetVertices(3) = sf::Vector2f(0.0f, -200.0f);

		c->SetPosition(center + sf::Vector2f(950.0f, 600.0f));

		c->SetRotation(sf::degrees(75.0f));

		m_Colliders.push_back(c);
	}

	InitEdges(border_size);
}

void GameCollision::InitEdges(sf::Vector2i border_size) {

	BoxCollider* c = new BoxCollider();
	c->Create(sf::Vector2f(border_size.x, border_size.y));
	c->SetPosition(sf::Vector2f(0.0f, 0.0f));
	
	castTexture.create(border_size.x, border_size.y);
	castTexture.setSmooth(true);

	for (uint32_t i = 0; i < c->getFigureDimension(); i++)
	{
		m_Edges.push_back(c->getEdge(i));
		m_MiniMapEdges.push_back(c->getEdge(i));
	}
	delete c;

	for (auto p : m_Colliders)
		for (uint32_t i = 0; i < p->getFigureDimension(); i++)
			m_Edges.push_back(p->getEdge(i));
}

void GameCollision::CheckCollision()
{
	for (auto& c : m_Colliders) {
		sf::Vector2f MTV;
		Collision::Instance.SATCollision(m_players[m_player_seq_number].m_box, *c, MTV);

		m_players[m_player_seq_number].m_box.Move(MTV);
	}
}

void GameCollision::UpdateBox(float dt)
{
	constexpr float smoothness = 8.0f;

	if (sf::Mouse::isButtonPressed(sf::Mouse::Left))
		m_players[m_player_seq_number].m_box.Rotate(sf::degrees(100.0f * dt));

	sf::Vector2f start = m_players[m_player_seq_number].m_box.GetPosition();
	sf::Vector2f end = MousePos(*Window);
	sf::Vector2f pos = Lerp(start, end, smoothness * dt);

	auto a = map.pixel2hex(sf::Vector2i(pos.x, pos.y));
	if (map._is_in_hex_grid(a))
	{
		m_players[m_player_seq_number].m_box.SetPosition(pos);
	}

}
