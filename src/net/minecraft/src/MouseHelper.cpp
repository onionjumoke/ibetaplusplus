#include "MouseHelper.h"

#include "pc/lwjgl/Display.h"
#include "pc/lwjgl/Mouse.h"

MouseHelper::MouseHelper(void *component)
	: deltaX(0)
	, deltaY(0)
	, field_1117_c(component)
	, cursor(nullptr)
	, field_1115_e(10)
{
}

void MouseHelper::grabMouseCursor()
{
	lwjgl::Mouse::setGrabbed(true);
	deltaX = 0;
	deltaY = 0;
}

void MouseHelper::ungrabMouseCursor()
{
	(void)field_1117_c;
	lwjgl::Mouse::setCursorPosition(lwjgl::Display::getWidth() / 2, lwjgl::Display::getHeight() / 2);
	lwjgl::Mouse::setGrabbed(false);
}

void MouseHelper::mouseXYChange()
{
	deltaX = lwjgl::Mouse::getDX();
	deltaY = lwjgl::Mouse::getDY();
}
