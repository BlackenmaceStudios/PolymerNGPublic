// InputSystem.cpp
//

#include "../Xbox/Xboxutilpch.h"

#include "InputSystem_private.h"
#include "../../include/build.h"

#include <SDL.h>
#include "sdlkeytranslation.h"

XBuildInputSystemPrivate xBuildInputSystemPrivate;
XBuildInputSystem *xBuildInputSystem = &xBuildInputSystemPrivate;

extern float globalWindowWidth;
extern float globalWindowHeight;

void XBuildInputSystemPrivate::Init()
{
	//m_keyboard = std::make_unique<DirectX::Keyboard>();
	//m_mouse = std::make_unique<DirectX::Mouse>();
	gamePad = std::make_unique<DirectX::GamePad>();

	//m_mouse->SetWindow(window);
	//m_keyboard->SetWindow(window);

	for (int i = 0; i < XB_NumButtons; i++)
	{
		controllerButtonForcedUp[i] = false;
	}

	SDL_JoystickEventState(SDL_ENABLE);
	//SDL_SetRelativeMouseMode(SDL_TRUE);

	buildkeytranslationtable();

	memset(&keyboardInputLocalState[0], 0, sizeof(bool) * 256);

	if (gamePad == nullptr)
		return;

	// Find a xbox controller that works.
	for (int i = 0; i < 4; i++)
	{
		if (gamePad->GetCapabilities(i).connected)
		{
			currentPlayerId = i;
		}
	}


}

bool XBuildInputSystemPrivate::KB_KeyPressed(unsigned char c)
{
	return false; // m_keyboard->GetState().IsKeyDown((DirectX::Keyboard::Keys)c);
}

extern "C" void readmousexy(int32_t *x, int32_t *y)
{

	static int32_t lastMouseX = 0;
	static int32_t lastMouseY = 0;
	static int32_t currentMouseX = 0;
	static int32_t currentMouseY = 0;

	// Get the mouse position.
	SDL_GetMouseState(&currentMouseX, &currentMouseY);
	if (currentMouseX - lastMouseX != 0 || currentMouseY - lastMouseY != 0)
	{
		//int32_t mousePositionX = currentMouseX - lastMouseX;
		//int32_t mousePositionY = currentMouseY - lastMouseY;
		//
		//float movementDirX = 0;
		//float movementDirY = 0;
		//
		//movementDirX = (((currentMouseX - 0) / (globalWindowWidth - 0)) - 0.5f) * 2.0f;
		//movementDirY = (((currentMouseY - 0) / (globalWindowHeight - 0)) - 0.5f) * 2.0f;
		//
		//
		//lastMouseX = currentMouseX;
		//lastMouseY = currentMouseY;
		//
		//*x = movementDirX * 50;
		//*y = (movementDirY * 50);
		//return;
	}

	if (xBuildInputSystemPrivate.gamePad == nullptr)
	{
		*x = 0;
		*y = 0;
		return;
	}

	DirectX::GamePad::State controllerState = xBuildInputSystemPrivate.gamePad->GetState(xBuildInputSystemPrivate.GetCurrentPlayerId(), DirectX::GamePad::DEAD_ZONE_CIRCULAR);

	if (controllerState.connected)
	{
		*x = controllerState.thumbSticks.rightX * 100;
		*y = (-controllerState.thumbSticks.rightY * 100) * 2.0f;
		return;
	}
	else
	{
		*x = 0;
		*y = 0;
	}
#if 0
	int32_t mousePositionX = xBuildInputSystemPrivate.m_mouse->GetState().x - lastMouseX;
	int32_t mousePositionY = xBuildInputSystemPrivate.m_mouse->GetState().y - lastMouseY;

	lastMouseX = xBuildInputSystemPrivate.m_mouse->GetState().x;
	lastMouseY = xBuildInputSystemPrivate.m_mouse->GetState().y;

	int32_t xwidth = max(scale(240 << 16, xdim, ydim), 320 << 16);

	*x = mousePositionX; //scale(mousePositionX, xwidth, xdim) - ((xwidth >> 1) - (320 << 15));
	*y = mousePositionY; // scale(mousePositionY, 200 << 16, ydim);
#endif
}

extern "C" void readmousebstatus(int32_t *b)
{
	if (SDL_GetMouseState(NULL, NULL) & SDL_BUTTON(SDL_BUTTON_LEFT))
	{
		b[0] = 1;
		return;
	}

	if (xBuildInputSystemPrivate.gamePad == nullptr)
		return;

	DirectX::GamePad::State controllerState = xBuildInputSystemPrivate.gamePad->GetState(xBuildInputSystemPrivate.GetCurrentPlayerId(), DirectX::GamePad::DEAD_ZONE_CIRCULAR);

	if (controllerState.connected)
	{
		if (controllerState.triggers.right)
		{
			b[0] = 1;
		}
		else
		{
			b[0] = 0;
		}
		return;
	}

	b[0] = 0;
}

void XHandleControllerMovement(int32_t *dx, int32_t *dy)
{
	if (xBuildInputSystemPrivate.gamePad == nullptr)
	{
		*dx = 0;
		*dy = 0;
		return;
	}

	DirectX::GamePad::State controllerState = xBuildInputSystemPrivate.gamePad->GetState(xBuildInputSystemPrivate.GetCurrentPlayerId(), DirectX::GamePad::DEAD_ZONE_INDEPENDENT_AXES);

	if (controllerState.connected)
	{
		*dx = controllerState.thumbSticks.leftX * 100 ;
		*dy = (-controllerState.thumbSticks.leftY * 10000) * 2.0f;
		return;
	}
}

bool XBuildInputSystemPrivate::ControllerKeyDown(XControllerButton button)
{
	if (xBuildInputSystemPrivate.gamePad == nullptr)
		return false;

	DirectX::GamePad::State controllerState = xBuildInputSystemPrivate.gamePad->GetState(currentPlayerId, DirectX::GamePad::DEAD_ZONE_INDEPENDENT_AXES);

	if (controllerState.connected)
	{
		if (controllerButtonForcedUp[button])
		{
			return false;
		}

		switch (button)
		{
			case XB_Button_A:
				return controllerState.buttons.a;

			case XB_Button_B:
				return controllerState.buttons.b;
			case XB_Button_Y:
				return controllerState.buttons.y;
			case XB_Button_X:
				return controllerState.buttons.x;
			case XB_Button_DPAD_Up:
				return controllerState.dpad.up;
			case XB_Button_DPAD_Down:
				return controllerState.dpad.down;
			case XB_Right_Trigger:
				return controllerState.IsRightTriggerPressed();
			case XB_Button_DPAD_Left:
				return controllerState.dpad.left;
			case XB_Button_DPAD_Right:
				return controllerState.dpad.right;
			case XB_Shoulder_Left:
				return controllerState.IsLeftShoulderPressed();
			case XB_Shoulder_Right:
				return controllerState.IsRightShoulderPressed();
		}
	}
	return false;
}

void XBuildInputSystemPrivate::SetControllerButtonsUp()
{
	for (int i = 0; i < XB_NumButtons; i++)
	{
		controllerButtonForcedUp[i] = true;
	}
}

void XBuildInputSystemPrivate::Update()
{
	const uint8_t* keystate = SDL_GetKeyboardState(NULL);

	for (int i = 0; i < SDL_NUM_SCANCODES; i++)
	{
		if (keystate[keytranslation[i].sdlkey])
		{
			// The game can intentionally disable key down's, we need to handle this.
			if(keyboardInputLocalState[keytranslation[i].buildkey] == false && keystatus[keytranslation[i].buildkey] == false)
				keystatus[keytranslation[i].buildkey] = 1;

			keyboardInputLocalState[keytranslation[i].buildkey] = 1;
		}
		else
		{
			keystatus[keytranslation[i].buildkey] = 0;
			keyboardInputLocalState[keytranslation[i].buildkey] = 0;
		}
	}

	if (xBuildInputSystemPrivate.gamePad != nullptr)
	{
		try
		{
			DirectX::GamePad::State controllerState = xBuildInputSystemPrivate.gamePad->GetState(xBuildInputSystemPrivate.GetCurrentPlayerId(), DirectX::GamePad::DEAD_ZONE_INDEPENDENT_AXES);

			if (controllerState.connected)
			{
				for (int i = 0; i < XB_NumButtons; i++)
				{
					switch (i)
					{
					case XB_Button_A:
						if (controllerState.buttons.a == false)
							controllerButtonForcedUp[i] = false;
						break;
					case XB_Button_B:
						if (controllerState.buttons.b == false)
							controllerButtonForcedUp[i] = false;
						break;
					case XB_Button_Y:
						if (controllerState.buttons.y == false)
							controllerButtonForcedUp[i] = false;
						break;
					case XB_Button_X:
						if (controllerState.buttons.x == false)
							controllerButtonForcedUp[i] = false;
						break;
					case XB_Button_DPAD_Up:
						if (controllerState.dpad.up == false)
							controllerButtonForcedUp[i] = false;
						break;
					case XB_Button_DPAD_Down:
						if (controllerState.dpad.down == false)
							controllerButtonForcedUp[i] = false;
						break;
					case XB_Button_DPAD_Left:
						if (controllerState.dpad.left == false)
							controllerButtonForcedUp[i] = false;
						break;
					case XB_Button_DPAD_Right:
						if (controllerState.dpad.right == false)
							controllerButtonForcedUp[i] = false;
						break;
					case XB_Shoulder_Left:
						if (controllerState.IsLeftShoulderPressed() == false)
							controllerButtonForcedUp[i] = false;
						break;
					case XB_Shoulder_Right:
						if (controllerState.IsRightShoulderPressed() == false)
							controllerButtonForcedUp[i] = false;
						break;
					}

				}
			}
		}
		catch (...)
		{

		}
	}
}