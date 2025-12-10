#include "UIGroup.hpp"

UIGroup::UIGroup(std::vector<UIElement> UIelements)
	: elementCount(0)
{
	for (auto const& element : UIelements)
	{
		uiElements.push_back(element);
		uiElementGPUs.push_back(UIElementGPU(element));
		elementCount += 1;
	}
}

void UIGroup::checkMouseInterractions(Vec2f mousePos, int mouseStatus)
{
	for (int i = 0; i < elementCount; i++)
	{
		uiElements[i].checkUpdates(mousePos, mouseStatus);
	}
}

const UIElement& UIGroup::getElement(int index) const
{
	return uiElements[index];
}

UIElement& UIGroup::getElement(int index)
{
	return uiElements[index];
}

const UIElementGPU& UIGroup::getElementGPU(int index) const
{
	return uiElementGPUs[index];
}

UIElementGPU& UIGroup::getElementGPU(int index)
{
	return uiElementGPUs[index];
}

const int& UIGroup::getElementCount() const
{
	return elementCount;
}

int& UIGroup::getElementCount()
{
	return elementCount;
}