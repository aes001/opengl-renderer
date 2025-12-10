#include "UIObject.hpp"
#include <iostream>

UIElement::UIElement(UIElementProperties properties) 
{
	CalculateVerticies(properties.uiPosition, properties.uiWidth, properties.uiHeight, properties.uiBorderWidth);
	currentColour = properties.uiColour;
	elementProperties = properties;
	CalculateBounds();
}

void UIElement::CalculateVerticies(Vec2f position, float width, float height, float borderWidth)
{
	std::vector<Vec2f> R;
	std::vector<uint8_t> B;

	Vec2f UL = position + Vec2f{0.f, height};
	Vec2f UR = position + Vec2f{ width, height };
	Vec2f LR = position + Vec2f{ width, 0.f };

	if (borderWidth == 0.f) {
		//triangle 1
		R.push_back(UL);
		R.push_back({ position });
		R.push_back(UR);
		//triangle 2
		R.push_back(UR);
		R.push_back({ position });
		R.push_back(LR);
	}
	else 
	{
		float b = borderWidth;
		Vec2f p = position;

		R.insert(R.end(), { UR, UL, UR - Vec2f{0, b} }); // top border
		R.insert(R.end(), { UR + Vec2f{0, -b}, UL, UL + Vec2f{0, - b}});

		R.insert(R.end(), { UL + Vec2f{0, -b}, p + Vec2f{0, b}, UL + Vec2f{b, -b} }); // left border
		R.insert(R.end(), { UL + Vec2f{b, -b}, p + Vec2f{0, b}, p + Vec2f{b, b} });

		R.insert(R.end(), { LR + Vec2f{0, b}, p + Vec2f{0, b}, LR }); //lower border
		R.insert(R.end(), { LR, p + Vec2f{0, b}, p });

		R.insert(R.end(), { UR + Vec2f{-b, -b}, LR + Vec2f{-b, b}, UR + Vec2f{0, -b} }); //right border
		R.insert(R.end(), { UR + Vec2f{0, -b}, LR + Vec2f{-b, b}, LR + Vec2f{0, b},  });

		for (int i = 0; i < R.size(); i++)
		{
			B.push_back(1);
		}

		R.insert(R.end(), { UL + Vec2f(b, -b), p + Vec2f{b, b}, LR + Vec2f{-b, b} }); //inner fill
		R.insert(R.end(), { UR + Vec2f(-b, -b), UL + Vec2f{b, -b}, LR + Vec2f{-b, b} });

		for (int i = 0; i < 6; i++)
		{
			B.push_back(0);
		}
	}


	uiVertices = R;
	uiBorderFlags = B;
}

void UIElement::CalculateBounds()
{
	LB = elementProperties.uiPosition.x;
	RB = elementProperties.uiPosition.x + elementProperties.uiWidth;
	BB = elementProperties.uiPosition.y;
	UB = elementProperties.uiPosition.y + elementProperties.uiHeight;

}

void UIElement::checkUpdates(Vec2f mousePos) 
{

	//check if within bounds of element
	if (LB <= mousePos.x && mousePos.x <= RB &&
		BB <= mousePos.y && mousePos.y <= UB)
	{
		currentColour = elementProperties.uiColour / 2.f;
	}
	else 
	{
		currentColour = elementProperties.uiColour;
	}

}


const std::vector<Vec2f>& UIElement::Vertices() const
{
	return uiVertices;
}


std::vector<Vec2f>& UIElement::Vertices()
{
	return uiVertices;
};

const std::vector<uint8_t>& UIElement::BorderFlags() const
{
	return uiBorderFlags;
}

std::vector<uint8_t>& UIElement::BorderFlags()
{
	return uiBorderFlags;
}

const Vec4f UIElement::getColour() const
{
	return currentColour;
}

Vec4f UIElement::getColour()
{
	return currentColour;
}



//GPU version of the class for loading an element into VBO's
UIElementGPU::UIElementGPU(const UIElement& UI) 
	: uiVboPositions(0)
	, uiVboBorderFlags(0)
	, uiVao(0)
{
	CreatePositionsVBO(UI);
	CreateBorderFlagsVBO(UI);
	CreateVAO();

	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

//destructor
UIElementGPU::~UIElementGPU()
{
	ReleaseBuffers();
}


UIElementGPU::UIElementGPU(UIElementGPU&& other) noexcept
	: uiVboPositions(std::exchange(other.uiVboPositions, 0))
	, uiVboBorderFlags(std::exchange(other.uiVboBorderFlags, 0))
	, uiVao(std::exchange(other.uiVao, 0))
{
}


UIElementGPU& UIElementGPU::operator=(UIElementGPU&& other) noexcept
{
	if (this != &other)
	{
		ReleaseBuffers();

		uiVboPositions = std::exchange(other.uiVboPositions, 0);
	}

	return *this;
}



GLuint UIElementGPU::BufferId(uiBufferType bufferType) const
{
	GLuint ret = 0;

	switch (bufferType)
	{
	case uiVboPositions_index:
		ret = uiVboPositions;
		break;
	}

	return ret;
}

GLuint UIElementGPU::ArrayId() const 
{
	return uiVao;
}

void UIElementGPU::CreatePositionsVBO(const UIElement& UI) 
{
	glGenBuffers(1, &uiVboPositions);
	glBindBuffer(GL_ARRAY_BUFFER, uiVboPositions);
	glBufferData(GL_ARRAY_BUFFER, UI.Vertices().size() * sizeof(Vec2f), UI.Vertices().data(), GL_STATIC_DRAW);
}

void UIElementGPU::CreateBorderFlagsVBO(const UIElement& UI)
{
	glGenBuffers(1, &uiVboBorderFlags);
	glBindBuffer(GL_ARRAY_BUFFER, uiVboBorderFlags);
	glBufferData(GL_ARRAY_BUFFER, UI.BorderFlags().size() * sizeof(uint8_t), UI.BorderFlags().data(), GL_STATIC_DRAW);
}


void UIElementGPU::CreateVAO() 
{
	glGenVertexArrays(1, &uiVao);
	glBindVertexArray(uiVao);
	glBindBuffer(GL_ARRAY_BUFFER, uiVboPositions);
	glVertexAttribPointer(
		0,
		2, GL_FLOAT, GL_FALSE,
		0,
		0
	);
	glEnableVertexAttribArray(0);

	glBindBuffer(GL_ARRAY_BUFFER, uiVboBorderFlags);
	glVertexAttribIPointer(
		1,
		1, GL_UNSIGNED_BYTE,
		0,
		0
	);
	glEnableVertexAttribArray(1);
}

void UIElementGPU::ReleaseBuffers() 
{
	glDeleteBuffers(1, &uiVboPositions);

	uiVboPositions = 0;
}