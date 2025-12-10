#ifndef UI_OBJECT_HPP
#define UI_OBJECT_HPP

// Includes
#include "glad/glad.h"
#include <GLFW/glfw3.h>
#include "../vmlib/vec2.hpp"
#include "../vmlib/vec3.hpp"
#include "../vmlib/vec4.hpp"

#include <vector>
#include <functional>

struct UIElementProperties
{
	Vec4f uiColour;
	Vec2f uiPosition;
	float uiWidth;
	float uiHeight;
	float uiBorderWidth;
};

class UIElement 
{
public:
	explicit UIElement(UIElementProperties properties);

	void checkUpdates(Vec2f mousePos, int mouseStatus);

	const std::vector<Vec2f>& Vertices() const;
	std::vector<Vec2f>& Vertices();

	const std::vector<uint8_t>& BorderFlags() const;
	std::vector<uint8_t>& BorderFlags();

	const Vec4f getColour() const;
	Vec4f getColour();

	void InsertOnClickCallback(std::function<void()> cb);


private:
	void CalculateVerticies(Vec2f position, float width, float height, float borderWidth);
	void CalculateBounds();
	void TriggerCallbacks();

private:
	std::vector<Vec2f> uiVertices;
	std::vector<uint8_t> uiBorderFlags;
	std::vector<std::function<void()>> uiOnClickCallbacks;
	Vec4f currentColour;
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

	void CreateBorderFlagsVBO(const UIElement& UI);

	void CreateVAO();

	void ReleaseBuffers();

private:
	GLuint uiVboPositions;
	GLuint uiVboBorderFlags;

	GLuint uiVao;

};

#endif