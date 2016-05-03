// InputSystem.h
//

#pragma once

//
// XControllerButton
//
enum XControllerButton
{
	XB_Button_A = 0,
	XB_Button_B,
	XB_Button_Y,
	XB_Button_X,
	XB_Button_DPAD_Up,
	XB_Button_DPAD_Down,
	XB_Button_DPAD_Left,
	XB_Button_DPAD_Right,
	XB_Shoulder_Left,
	XB_Shoulder_Right,
	XB_Right_Trigger,
	XB_NumButtons,
};

/*
=============================================================================

XBuildInputSystem

=============================================================================
*/
//
// XBuildInputSystem
//
class XBuildInputSystem
{
public:
	virtual bool ControllerKeyDown(XControllerButton button) = 0;
	virtual void SetControllerButtonsUp() = 0;
	virtual void Update() = 0;
};


extern XBuildInputSystem *xBuildInputSystem;
