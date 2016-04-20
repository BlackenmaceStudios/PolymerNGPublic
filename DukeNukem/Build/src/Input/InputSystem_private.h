#pragma once

#include "XKeyboard.h"
#include "XMouse.h"
#include "XGamePad.h"

#include "InputSystem.h"


//
// XBuildInputSystem
//
class XBuildInputSystemPrivate : public XBuildInputSystem
{
public:
	void XBuildInputSystemPrivate::Init(ABI::Windows::UI::Core::ICoreWindow* window);

	virtual bool KB_KeyPressed(unsigned char c);
	virtual bool ControllerKeyDown(XControllerButton button);
	virtual void SetControllerButtonsUp();
	virtual void Update();
public:
	std::unique_ptr<DirectX::Keyboard> m_keyboard;
	std::unique_ptr<DirectX::Mouse> m_mouse;
	std::unique_ptr<DirectX::GamePad> gamePad;

	bool controllerButtonForcedUp[XB_NumButtons];
};

extern XBuildInputSystemPrivate xBuildInputSystemPrivate;