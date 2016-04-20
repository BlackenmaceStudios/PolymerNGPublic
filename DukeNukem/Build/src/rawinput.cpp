#define WIN32_LEAN_AND_MEAN
#define _WIN32_WINNT 0x0501
#include <malloc.h>
#include <windows.h>
#include "rawinput.h"
#include "winlayer.h"
#include "scancodes.h"
#include "build.h"

//static BOOL rawinput_started = 0;
//static uint8_t KeyboardState[256] = {0}; // VKeys
//
//extern uint8_t moustat, mousegrab;
//extern void SetKey(int32_t key, int32_t state);
//
////#define MASK_DOWN (1<<(i<<1))
////#define MASK_UP (MASK_DOWN<<1)
//#ifndef GET_RAWINPUT_CODE_WPARAM
//#define GET_RAWINPUT_CODE_WPARAM(wParam)    ((wParam) & 0xff)
//#endif
//
//static inline void RI_ProcessMouse(const RAWMOUSE *rmouse)
//{
//    int32_t i, mask;
//    int8_t MWheel = 0;
//
//    if (!mousegrab || !appactive)
//        return;
//
//    mousex += rmouse->lLastX;
//    mousey += rmouse->lLastY;
//
//    if (rmouse->usFlags & MOUSE_MOVE_ABSOLUTE)
//    {
//        // untested... maybe devices like wacom tablets set this flag?
//        POINT pos = { xdim>>1, ydim>>1 };
//
//        ClientToScreen((HWND)win_gethwnd(), &pos);
//
//        mousex -= pos.x;
//        mousey -= pos.y;
//    }
//
//    for (i = 0, mask = (1<<0); mask <= (1<<8); i++, mask<<=2)
//    {
//        // usButtonFlags:
//        //  1<<0: left down   -> 1 / 1<<0
//        //  1<<2: middle down -> 2 / 1<<1
//        //  1<<4: right down  -> 3 / 1<<2
//        //  1<<6: x1 down     -> 4 / 1<<3
//        //                    ----------- mwheel here
//        //  1<<8: x2 down     -> 7 / 1<<6
//
//        if (mask == 1<<8)
//            i = 6;
//
//        if (rmouse->usButtonFlags & mask) // button down
//        {
//            if (mousepresscallback)
//                mousepresscallback(i+1, 1);
//            mouseb |= 1<<i;
//        }
//        else if (rmouse->usButtonFlags & (mask<<1)) // button up
//        {
//            if (mousepresscallback)
//                mousepresscallback(i+1, 0);
//            mouseb &= ~(1<<i);
//        }
//    }
//
//    MWheel = (rmouse->usButtonFlags & RI_MOUSE_WHEEL) ? rmouse->usButtonData : 0;
//
//    if (MWheel > 0)   	// wheel up
//    {
//        mouseb |= 16;
//        if (mousepresscallback) mousepresscallback(5, 1);
//    }
//    else if (MWheel < 0)  	// wheel down
//    {
//        mouseb |= 32;
//        if (mousepresscallback) mousepresscallback(6, 1);
//    }
//}
//
//static inline void RI_ProcessKeyboard(const RAWKEYBOARD *rkbd)
//{
//    uint8_t key = rkbd->MakeCode, VKey = rkbd->VKey;
//
//    // for some reason rkbd->MakeCode is wrong for these
//    // even though rkbd->VKey is right...
//
//    switch (VKey)
//    {
//    case VK_SHIFT:
//        if (rkbd->Flags & RI_KEY_E0) VKey = VK_RSHIFT, key = sc_RightShift; break;
//    case VK_CONTROL:
//        if (rkbd->Flags & RI_KEY_E0) VKey = VK_RCONTROL, key = sc_RightControl; break;
//    case VK_MENU:
//        if (rkbd->Flags & RI_KEY_E0) VKey = VK_RMENU, key = sc_RightAlt; break;
//    case VK_UP:
//    case VK_NUMPAD8:
//        if (rkbd->Flags & RI_KEY_E0) VKey = VK_UP, key = sc_UpArrow;
//        else VKey = VK_NUMPAD8, key = sc_kpad_8; break;
//    case VK_DOWN:
//    case VK_NUMPAD2:
//        if (rkbd->Flags & RI_KEY_E0) VKey = VK_DOWN, key = sc_DownArrow;
//        else VKey = VK_NUMPAD2, key = sc_kpad_2; break;
//    case VK_LEFT:
//    case VK_NUMPAD4:
//        if (rkbd->Flags & RI_KEY_E0) VKey = VK_LEFT, key = sc_LeftArrow;
//        else VKey = VK_NUMPAD4, key = sc_kpad_4; break;
//    case VK_RIGHT:
//    case VK_NUMPAD6:
//        if (rkbd->Flags & RI_KEY_E0) VKey = VK_RIGHT, key = sc_RightArrow;
//        else VKey = VK_NUMPAD6, key = sc_kpad_6; break;
//    case VK_DIVIDE:
////        if (rkbd->Flags & RI_KEY_E0) key = sc_Slash;
////        else
//        key = sc_kpad_Slash;
//        break;
//    case VK_INSERT:
//        key = sc_Insert; break;
//    case VK_HOME:
//        key = sc_Home; break;
//    case VK_DELETE:
//        key = sc_Delete; break;
//    case VK_END:
//        key = sc_End; break;
//    case VK_PRIOR:
//        key = sc_PgUp; break;
//    case VK_NEXT:
//        key = sc_PgDn; break;
//    case VK_RETURN:
//        if (rkbd->Flags & RI_KEY_E0) key = sc_kpad_Enter; break;
//    case VK_PAUSE:
//        KeyboardState[VKey] = 1 - (rkbd->Flags & RI_KEY_BREAK);
//        if (rkbd->Flags & RI_KEY_BREAK) return;
//
//        SetKey(sc_Pause, 1);
//
//        if (keypresscallback)
//            keypresscallback(sc_Pause, 1);
//    case 0xFF:
//        return;
//    }
//
//    KeyboardState[VKey] = 1 - (rkbd->Flags & RI_KEY_BREAK);
//
//    if (OSD_HandleScanCode(key, KeyboardState[VKey] > 0))
//    {
//        SetKey(key, KeyboardState[VKey] != 0);
//
//        if (keypresscallback)
//            keypresscallback(key, KeyboardState[VKey] != 0);
//    }
//
//    if (rkbd->Flags & RI_KEY_BREAK) return;
//    if (((keyasciififoend+1)&(KEYFIFOSIZ-1)) == keyasciififoplc) return;
//    if ((keyasciififoend - keyasciififoplc) > 0) return;
//
//    {
//        uint8_t buf[2];
//
//        if (ToAscii(VKey, key, &KeyboardState[0], (LPWORD)&buf[0], 0) != 1) return;
//        if ((OSD_OSDKey() < 128) && (Btolower(scantoasc[OSD_OSDKey()]) == Btolower(buf[0]))) return;
//        if (OSD_HandleChar(buf[0]) == 0) return;
//
//        keyasciififo[keyasciififoend] = buf[0];
//        keyasciififoend = ((keyasciififoend+1)&(KEYFIFOSIZ-1));
//    }
//}

// keyboard is always captured regardless of what we tell this function
BOOL RI_CaptureInput(BOOL grab, HWND target)
{
	return TRUE;
}

void RI_ProcessMessage(MSG *msg)
{
   
}

void RI_PollDevices(BOOL loop)
{
   
}

int32_t initmouse(void)
{
  
    return 0;
}

void uninitmouse(void)
{
}

void grabmouse(char a)
{

}

void AppGrabMouse(char a)
{
 //   UNREFERENCED_PARAMETER(a);
}
