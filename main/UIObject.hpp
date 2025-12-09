#ifndef UI_OBJECT_HPP
#define UI_OBJECT_HPP

#include "../vmlib/vec2.hpp"
#include "../vmlib/vec3.hpp"

#include <vector>

struct UIElementProperties
{
	Vec3f uiColour;
	Vec2f uiPosition;
	float uiWidth;
	float uiHeight;
};

class UIElement 
{
public:
	explicit UIElement(UIElementProperties properties);

	const std::vector<Vec2f>& Vertices() const;
	std::vector<Vec2f>& Vertices();

	const std::vector<Vec3f>& VertexColours() const;
	std::vector<Vec3f>& VertexColours();

private:
	std::vector<Vec2f> CalculateVerticies(Vec2f position, float width, float height);


private:
	std::vector<Vec2f> uiVertices;
	std::vector<Vec3f> uiVertexColours;
};

#endif