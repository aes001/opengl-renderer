#include "UIObject.hpp"

UIElement::UIElement(UIElementProperties properties) 
{
	uiVertices = CalculateVerticies(properties.uiPosition, properties.uiWidth, properties.uiHeight);

	for (int i = 0; i < uiVertices.size(); i++) {
		uiVertexColours.push_back(properties.uiColour);
	}

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


const std::vector<Vec3f>& UIElement::VertexColours() const
{
	return uiVertexColours;
}


std::vector<Vec3f>& UIElement::VertexColours()
{
	return uiVertexColours;
};


//GPU version of the class for loading an element into VBO's
UIElementGPU::UIElementGPU(const UIElement& UI) 
	: uiVboPositions(0)
	, uiVboVertexColour(0)
{
	CreatePositionsVBO(UI);
	CreateVertexColourVBO(UI);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

//destructor
UIElementGPU::~UIElementGPU()
{
	ReleaseBuffers();
}


UIElementGPU::UIElementGPU(UIElementGPU&& other) noexcept
	: uiVboPositions(std::exchange(other.uiVboPositions, 0))
	, uiVboVertexColour(std::exchange(other.uiVboVertexColour, 0))
{
}


UIElementGPU& UIElementGPU::operator=(UIElementGPU&& other) noexcept
{
	if (this != &other)
	{
		ReleaseBuffers();

		uiVboPositions = std::exchange(other.uiVboPositions, 0);
		uiVboVertexColour = std::exchange(other.uiVboVertexColour, 0);
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
	case uiVboVertexColour_index:
		ret = uiVboVertexColour;
		break;
	};

	return ret;
}

void UIElementGPU::CreatePositionsVBO(const UIElement& UI) 
{
	glGenBuffers(1, &uiVboPositions);
	glBindBuffer(GL_ARRAY_BUFFER, uiVboPositions);
	glBufferData(GL_ARRAY_BUFFER, UI.Vertices().size() * sizeof(Vec2f), UI.Vertices().data(), GL_STATIC_DRAW);
}

void UIElementGPU::CreateVertexColourVBO(const UIElement& UI)
{
	glGenBuffers(1, &uiVboVertexColour);
	glBindBuffer(GL_ARRAY_BUFFER, uiVboVertexColour);
	glBufferData(GL_ARRAY_BUFFER, UI.VertexColours().size() * sizeof(Vec3f), UI.VertexColours().data(), GL_STATIC_DRAW);
}

void UIElementGPU::ReleaseBuffers() 
{
	glDeleteBuffers(1, &uiVboPositions);
	glDeleteBuffers(1, &uiVboVertexColour);

	uiVboPositions = 0;
	uiVboVertexColour = 0;
}