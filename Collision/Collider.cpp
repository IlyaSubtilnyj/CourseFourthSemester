#include "stpch.h"
#include "Collider.hpp"

sf::Color ColliderColor(255, 255, 255);

Collider::Collider(uint32_t count)
	: m_verticesCount(count)
	, m_vertices(nullptr)
	, m_position(0.0f, 0.0f)
	, m_origin(0.0f, 0.0f)
	, m_rotation(sf::degrees(0.0f))
	, m_edgesCount(count)
{
	if (!count)
		return;

	m_edges = new Edge[count];
	m_vertices = new sf::Vector2f[count]; //count + 1
}

Collider::~Collider() {
	delete[] m_vertices;
	delete[] m_edges;
}

void Collider::SetPosition(sf::Vector2f position) {
	sf::Vector2f diff = position - m_position - m_origin;

	for (uint32_t i = 0; i < m_verticesCount; i++) {
		auto& v = m_vertices[i];
		auto& e = m_edges[i];

		v += diff;
		e.Start += diff;
		e.End += diff;
	}

	m_position = position - m_origin;
}

void Collider::SetOrigin(sf::Vector2f origin) {
	m_origin = origin;

	SetPosition(m_position);
}

void Collider::SetRotation(sf::Angle angle) {
	sf::Angle diff = angle - m_rotation;

	for (uint32_t i = 0; i < m_verticesCount; i++) {
		auto& v = m_vertices[i];

		v = ::Rotate(v, m_position + m_origin, diff);
	}

	m_rotation = angle;

	uint32_t i;
	for (i = 0; i < m_edgesCount - 1; i++)
	{
		m_edges[i] = Edge(m_vertices[i], m_vertices[i + 1]);
	}
	m_edges[i] = Edge(m_vertices[i], m_vertices[0]);

}

void Collider::Move(sf::Vector2f offset) {
	SetPosition(m_position + offset + m_origin);
}

void Collider::Rotate(sf::Angle angle) {
	SetRotation(m_rotation + angle);
}

sf::Vector2f Collider::GetPosition() const {
	return m_position + m_origin;
}

sf::Vector2f Collider::GetOrigin() const {
	return m_origin;
}

sf::Angle Collider::GetRotation() const {
	return m_rotation;
}

uint32_t Collider::getFigureDimension()
{
	return m_verticesCount;
}

Edge Collider::getEdge(int32_t edge_seq_num)
{
	if (edge_seq_num >= m_edgesCount) assert(0);
	return m_edges[edge_seq_num];
}

BoxCollider::BoxCollider()
	: Collider(4)
{

}

BoxCollider::~BoxCollider() {

}

void BoxCollider::Create(sf::Vector2f size) {
	auto& v = m_vertices;
	auto& e = m_edges;

	sf::Vector2f pos = m_position - m_origin;

	v[0] = pos + sf::Vector2f(0.0f, 0.0f);
	v[1] = pos + sf::Vector2f(size.x, 0.0f);
	v[2] = pos + sf::Vector2f(size.x, size.y);
	v[3] = pos + sf::Vector2f(0.0f, size.y);

	e[0] = Edge(v[0], v[1]);
	e[1] = Edge(v[1], v[2]);
	e[2] = Edge(v[2], v[3]);
	e[3] = Edge(v[3], v[0]);
}

void BoxCollider::draw(sf::RenderTarget& target, const sf::RenderStates states) const {
	static sf::Vertex v[6];

	for (int i = 0; i < 4; i++)
		v[i] = sf::Vertex(m_vertices[i], ColliderColor);

	v[4] = sf::Vertex(m_vertices[0], ColliderColor);
	v[5] = sf::Vertex(m_vertices[2], ColliderColor);

	target.draw(v, 6, sf::LineStrip);
}

CircleCollider::CircleCollider()
	: Collider(1)
{

}

CircleCollider::~CircleCollider() {

}

void CircleCollider::Create(float radius) {
	m_radius = radius;
}

float CircleCollider::GetRadius() const {
	return m_radius;
}

void CircleCollider::draw(sf::RenderTarget& target, const sf::RenderStates states) const {
	constexpr uint32_t points = 32;

	if (m_radius <= 0.0f)
		return;

	sf::Angle angle = sf::radians(2 * PI / (float)points);

	sf::Vector2f p = m_position;
	p.y += m_radius;

	static sf::Vertex v[points + 1];

	for (uint32_t i = 0; i <= points; i++)
		v[i] = sf::Vertex(::Rotate(p, m_position, angle * (float)i), ColliderColor);

	target.draw(v, points + 1, sf::LineStrip);
	auto vertex_p = new sf::Vertex(m_position, ColliderColor);
	target.draw(vertex_p, 1, sf::Points);
	delete vertex_p;
}

CustomCollider::CustomCollider()
	: Collider(0)
{

}

CustomCollider::CustomCollider(uint32_t count)
	: Collider(count)
{

}

CustomCollider::~CustomCollider() {

}

sf::Vector2f* CustomCollider::GetVertices() {
	return m_vertices;
}

sf::Vector2f& CustomCollider::GetVertices(uint32_t index) {
	return m_vertices[index];
}

void CustomCollider::Create(uint32_t count) {
	delete[] m_vertices;

	m_vertices = new sf::Vector2f[count];

	m_verticesCount = count;
}

sf::Vector2f& CustomCollider::operator[](uint32_t index) {
	return m_vertices[index];
}

bool CustomCollider::GenerateEdges()
{
	bool result = true;

	uint32_t i;
	for (i = 0; i < m_edgesCount - 1; i++)
	{
		m_edges[i] = Edge(m_vertices[i], m_vertices[i + 1]);
	}
	m_edges[i] = Edge(m_vertices[i], m_vertices[0]);

	return result;
}

void CustomCollider::draw(sf::RenderTarget& target, const sf::RenderStates states) const {
	const uint32_t count = m_verticesCount + 1;

	sf::Vertex* v = new sf::Vertex[count];

	for (uint32_t i = 0; i < count - 1; i++)
		v[i] = sf::Vertex(m_vertices[i], ColliderColor);

	v[count - 1] = sf::Vertex(m_vertices[0], ColliderColor);

	target.draw(v, count, sf::LineStrip);

	delete[] v;
}