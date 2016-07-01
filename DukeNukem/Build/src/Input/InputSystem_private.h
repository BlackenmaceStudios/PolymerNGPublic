#pragma once

//#include "XKeyboard.h"
//#include "XMouse.h"
#include "XGamePad.h"

#include "InputSystem.h"


//
// XBuildInputSystem
//
class XBuildInputSystemPrivate : public XBuildInputSystem
{
public:
	XBuildInputSystemPrivate() { Init(); }

	void XBuildInputSystemPrivate::Init();

	virtual bool KB_KeyPressed(unsigned char c);
	virtual bool ControllerKeyDown(XControllerButton button);
	virtual void SetControllerButtonsUp();
	virtual void Update();

	int GetCurrentPlayerId() { return currentPlayerId; }
public:
	//std::unique_ptr<DirectX::Keyboard> m_keyboard;
	//std::unique_ptr<DirectX::Mouse> m_mouse;
	std::unique_ptr<DirectX::GamePad> gamePad;

	int currentPlayerId;

	bool controllerButtonForcedUp[XB_NumButtons];
	bool keyboardInputLocalState[512];
};

extern XBuildInputSystemPrivate xBuildInputSystemPrivate;