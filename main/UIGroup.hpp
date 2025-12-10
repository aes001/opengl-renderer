#ifndef UI_GROUP_HPP
#define UI_GROUP_HPP

#include "UIObject.hpp"

class UIGroup
{
public:
	explicit UIGroup(std::vector<UIElement> UIelements);

	void checkMouseInterractions(Vec2f mousePos, int mouseStatus);

	const UIElement& getElement(int index) const;
	UIElement& getElement(int index);

	const UIElementGPU& getElementGPU(int index) const;
	UIElementGPU& getElementGPU(int index);

	const int& getElementCount() const;
	int& getElementCount();

private:
	std::vector<UIElement> uiElements;
	std::vector<UIElementGPU> uiElementGPUs;
	int elementCount;
};

#endif