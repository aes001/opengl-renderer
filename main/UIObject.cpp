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