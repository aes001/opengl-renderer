#ifndef UI_OBJECT_HPP
#define UI_OBJECT_HPP

// Includes
#include "glad/glad.h"
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

	void checkUpdates(Vec2f mousePos);

	const std::vector<Vec2f>& Vertices() const;
	std::vector<Vec2f>& Vertices();

	const Vec3f getColour() const;
	Vec3f getColour();

private:
	std::vector<Vec2f> CalculateVerticies(Vec2f position, float width, float height);
	void CalculateBounds();

private:
	std::vector<Vec2f> uiVertices;
	Vec3f currentColour;
	UIElementProperties elementProperties;
	float LB, RB, BB, UB; //Left Bound, Right Bound, Bottom Bount, Upper Bound

};



enum uiBufferType : size_t
{
	uiVboPositions_index = 0,
	uiVboVertexColour_index,
};

class UIElementGPU 
{
public:
	UIElementGPU( const UIElement& UI );
	~UIElementGPU();

	UIElementGPU( const UIElementGPU& ) = delete;
	UIElementGPU& operator=( const UIElementGPU& ) noexcept;

	UIElementGPU(UIElementGPU&& other) noexcept;
	UIElementGPU& operator=(UIElementGPU&& other) noexcept;


	GLuint BufferId(uiBufferType bufferType) const;
	GLuint ArrayId() const;

private:
	void CreatePositionsVBO(const UIElement& UI);

	void CreateVAO();

	void ReleaseBuffers();

private:
	GLuint uiVboPositions;

	GLuint uiVao;

};

#endif