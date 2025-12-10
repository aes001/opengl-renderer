#include "UIObject.hpp"

UIElement::UIElement(UIElementProperties properties) 
{
	uiVertices = CalculateVerticies(properties.uiPosition, properties.uiWidth, properties.uiHeight);
	currentColour = properties.uiColour;
}

std::vector<Vec2f> UIElement::CalculateVerticies(Vec2f position, float width, float height)
{
	std::vector<Vec2f> R;

	Vec2f UL = position + Vec2f{0.f, height};
	Vec2f UR = position + Vec2f{ width, height };
	Vec2f LR = position + Vec2f{ width, 0.f };

	//triangle 1
	R.push_back(UL);
	R.push_back({ position });
	R.push_back( UR );
	//triangle 2
	R.push_back(UR);
	R.push_back({ position });
	R.push_back(LR);

	return R;
}


const std::vector<Vec2f>& UIElement::Vertices() const
{
	return uiVertices;
}


std::vector<Vec2f>& UIElement::Vertices()
{
	return uiVertices;
};

const Vec3f UIElement::getColour() const
{
	return currentColour;
}

Vec3f UIElement::getColour()
{
	return currentColour;
}



//GPU version of the class for loading an element into VBO's
UIElementGPU::UIElementGPU(const UIElement& UI) 
	: uiVboPositions(0)
	, uiVao(0)
{
	CreatePositionsVBO(UI);
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
}

void UIElementGPU::ReleaseBuffers() 
{
	glDeleteBuffers(1, &uiVboPositions);

	uiVboPositions = 0;
}