/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2026 Andrew Paterson

This file is part of The Codaphela Project: Codaphela WindowLib

Codaphela WindowLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela WindowLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela WindowLib.  If not, see <http://www.gnu.org/licenses/>.

** ------------------------------------------------------------------------ **/
#include "BaseLib/Logger.h"
#include "InputLib/InputDevices.h"
#include "X11Input.h"
#include <linux/input-event-codes.h>


//Windows raw input keyboard flags.
#define X11_RI_KEY_BREAK                0x0001
#define X11_RI_KEY_E0                   0x0002
#define X11_RI_KEY_E1                   0x0004

//Windows raw input mouse button flags.  Button 1 is left, 2 is right and 3 is middle.
#define X11_RI_MOUSE_BUTTON_1_DOWN      0x0001
#define X11_RI_MOUSE_BUTTON_1_UP        0x0002
#define X11_RI_MOUSE_BUTTON_2_DOWN      0x0004
#define X11_RI_MOUSE_BUTTON_2_UP        0x0008
#define X11_RI_MOUSE_BUTTON_3_DOWN      0x0010
#define X11_RI_MOUSE_BUTTON_3_UP        0x0020
#define X11_RI_MOUSE_BUTTON_4_DOWN      0x0040
#define X11_RI_MOUSE_BUTTON_4_UP        0x0080
#define X11_RI_MOUSE_BUTTON_5_DOWN      0x0100
#define X11_RI_MOUSE_BUTTON_5_UP        0x0200
#define X11_RI_MOUSE_WHEEL              0x0400
#define X11_WHEEL_DELTA                 120

//An X server using evdev or libinput gives each key the Linux input event code plus 8.
#define X11_KEYCODE_OFFSET              8


struct SX11KeyTranslation
{
    uint16  usEventCode;
    uint16  usVKey;
    uint16  usMakeCode;
    uint16  usFlags;
};


//The Linux input event codes for the keys in WinInputLib's 'Standard 104 Button Keyboard' with what Windows raw input
//gives for them.  Raw input gives the numpad's keys as if Num Lock were off.  Event codes below KEY_KPENTER are the
//PC (set 1) scan codes.
static SX11KeyTranslation gasX11KeyTranslations[] =
{
    { KEY_ESC,          27,     0x01,   0 },
    { KEY_1,            '1',    0x02,   0 },
    { KEY_2,            '2',    0x03,   0 },
    { KEY_3,            '3',    0x04,   0 },
    { KEY_4,            '4',    0x05,   0 },
    { KEY_5,            '5',    0x06,   0 },
    { KEY_6,            '6',    0x07,   0 },
    { KEY_7,            '7',    0x08,   0 },
    { KEY_8,            '8',    0x09,   0 },
    { KEY_9,            '9',    0x0A,   0 },
    { KEY_0,            '0',    0x0B,   0 },
    { KEY_MINUS,        189,    0x0C,   0 },
    { KEY_EQUAL,        187,    0x0D,   0 },
    { KEY_BACKSPACE,    8,      0x0E,   0 },
    { KEY_TAB,          9,      0x0F,   0 },
    { KEY_Q,            'Q',    0x10,   0 },
    { KEY_W,            'W',    0x11,   0 },
    { KEY_E,            'E',    0x12,   0 },
    { KEY_R,            'R',    0x13,   0 },
    { KEY_T,            'T',    0x14,   0 },
    { KEY_Y,            'Y',    0x15,   0 },
    { KEY_U,            'U',    0x16,   0 },
    { KEY_I,            'I',    0x17,   0 },
    { KEY_O,            'O',    0x18,   0 },
    { KEY_P,            'P',    0x19,   0 },
    { KEY_LEFTBRACE,    219,    0x1A,   0 },
    { KEY_RIGHTBRACE,   221,    0x1B,   0 },
    { KEY_ENTER,        13,     0x1C,   0 },
    { KEY_LEFTCTRL,     17,     0x1D,   0 },
    { KEY_A,            'A',    0x1E,   0 },
    { KEY_S,            'S',    0x1F,   0 },
    { KEY_D,            'D',    0x20,   0 },
    { KEY_F,            'F',    0x21,   0 },
    { KEY_G,            'G',    0x22,   0 },
    { KEY_H,            'H',    0x23,   0 },
    { KEY_J,            'J',    0x24,   0 },
    { KEY_K,            'K',    0x25,   0 },
    { KEY_L,            'L',    0x26,   0 },
    { KEY_SEMICOLON,    186,    0x27,   0 },
    { KEY_APOSTROPHE,   222,    0x28,   0 },
    { KEY_GRAVE,        192,    0x29,   0 },
    { KEY_LEFTSHIFT,    16,     0x2A,   0 },
    { KEY_BACKSLASH,    220,    0x2B,   0 },
    { KEY_Z,            'Z',    0x2C,   0 },
    { KEY_X,            'X',    0x2D,   0 },
    { KEY_C,            'C',    0x2E,   0 },
    { KEY_V,            'V',    0x2F,   0 },
    { KEY_B,            'B',    0x30,   0 },
    { KEY_N,            'N',    0x31,   0 },
    { KEY_M,            'M',    0x32,   0 },
    { KEY_COMMA,        188,    0x33,   0 },
    { KEY_DOT,          190,    0x34,   0 },
    { KEY_SLASH,        191,    0x35,   0 },
    { KEY_RIGHTSHIFT,   16,     0x36,   0 },
    { KEY_KPASTERISK,   106,    0x37,   0 },
    { KEY_LEFTALT,      18,     0x38,   0 },
    { KEY_SPACE,        32,     0x39,   0 },
    { KEY_CAPSLOCK,     20,     0x3A,   0 },
    { KEY_F1,           112,    0x3B,   0 },
    { KEY_F2,           113,    0x3C,   0 },
    { KEY_F3,           114,    0x3D,   0 },
    { KEY_F4,           115,    0x3E,   0 },
    { KEY_F5,           116,    0x3F,   0 },
    { KEY_F6,           117,    0x40,   0 },
    { KEY_F7,           118,    0x41,   0 },
    { KEY_F8,           119,    0x42,   0 },
    { KEY_F9,           120,    0x43,   0 },
    { KEY_F10,          121,    0x44,   0 },
    { KEY_NUMLOCK,      144,    0x45,   0 },
    { KEY_SCROLLLOCK,   145,    0x46,   0 },
    { KEY_KP7,          36,     0x47,   0 },
    { KEY_KP8,          38,     0x48,   0 },
    { KEY_KP9,          33,     0x49,   0 },
    { KEY_KPMINUS,      109,    0x4A,   0 },
    { KEY_KP4,          37,     0x4B,   0 },
    { KEY_KP5,          12,     0x4C,   0 },
    { KEY_KP6,          39,     0x4D,   0 },
    { KEY_KPPLUS,       107,    0x4E,   0 },
    { KEY_KP1,          35,     0x4F,   0 },
    { KEY_KP2,          40,     0x50,   0 },
    { KEY_KP3,          34,     0x51,   0 },
    { KEY_KP0,          45,     0x52,   0 },
    { KEY_KPDOT,        46,     0x53,   0 },
    { KEY_102ND,        226,    0x56,   0 },
    { KEY_F11,          122,    0x57,   0 },
    { KEY_F12,          123,    0x58,   0 },
    { KEY_KPENTER,      13,     0x1C,   X11_RI_KEY_E0 },
    { KEY_RIGHTCTRL,    17,     0x1D,   X11_RI_KEY_E0 },
    { KEY_KPSLASH,      111,    0x35,   X11_RI_KEY_E0 },
    { KEY_SYSRQ,        255,    0x37,   X11_RI_KEY_E0 },
    { KEY_RIGHTALT,     18,     0x38,   X11_RI_KEY_E0 },
    { KEY_HOME,         36,     0x47,   X11_RI_KEY_E0 },
    { KEY_UP,           38,     0x48,   X11_RI_KEY_E0 },
    { KEY_PAGEUP,       33,     0x49,   X11_RI_KEY_E0 },
    { KEY_LEFT,         37,     0x4B,   X11_RI_KEY_E0 },
    { KEY_RIGHT,        39,     0x4D,   X11_RI_KEY_E0 },
    { KEY_END,          35,     0x4F,   X11_RI_KEY_E0 },
    { KEY_DOWN,         40,     0x50,   X11_RI_KEY_E0 },
    { KEY_PAGEDOWN,     34,     0x51,   X11_RI_KEY_E0 },
    { KEY_INSERT,       45,     0x52,   X11_RI_KEY_E0 },
    { KEY_DELETE,       46,     0x53,   X11_RI_KEY_E0 },
    { KEY_PAUSE,        19,     0x1D,   X11_RI_KEY_E1 },
    { KEY_LEFTMETA,     91,     0x5B,   X11_RI_KEY_E0 },
    { KEY_RIGHTMETA,    92,     0x5C,   X11_RI_KEY_E0 },
    { KEY_COMPOSE,      93,     0x5D,   X11_RI_KEY_E0 },
};


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CX11Input::Init(void)
{
    masEvents.Init();
    miKeyboard = -1;
    miMouse = -1;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CX11Input::Kill(void)
{
    masEvents.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// X merges every keyboard and every mouse into one of each.
//
//////////////////////////////////////////////////////////////////////////
void CX11Input::AddDevicesTo(CInputDevices* pcDevices)
{
    CInputDevice*       pcDevice;
    CInputDeviceDesc*   pcDesc;

    pcDesc = pcDevices->GetDescription("WinRAW Keyboard: Standard 104 Button Keyboard");
    if (pcDesc)
    {
        pcDevice = pcDevices->CreateDevice(pcDesc);
        pcDevice->GenerateRestState();
        miKeyboard = pcDevice->GetDeviceID();
    }
    else
    {
        gcLogger.Error2(__METHOD__, " The standard keyboard has not been described.  Has WinInputLib's XML been read?", NULL);
    }

    pcDesc = pcDevices->GetDescription("WinRAW Mouse: Standard 5 Button Mouse");
    if (pcDesc)
    {
        pcDevice = pcDevices->CreateDevice(pcDesc);
        pcDevice->GenerateRestState();
        miMouse = pcDevice->GetDeviceID();
    }
    else
    {
        gcLogger.Error2(__METHOD__, " The standard mouse has not been described.  Has WinInputLib's XML been read?", NULL);
    }
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CX11Input::Update(CInputDevices* pcDevices, uint32 uiSequence)
{
    size                    i;
    SX11InputEvent*         psEvent;
    CArrayX11InputEvent     asEvents;

    //The events are taken first so that ones added while they are processed, or processing them again from inside a
    //listener, as a modal dialog's loop does, see each event once.
    asEvents.Init();
    asEvents.Copy(&masEvents);
    masEvents.ReInit();

    for (i = 0; i < asEvents.NumElements(); i++)
    {
        psEvent = asEvents.Get(i);
        if (psEvent->bKeyboard)
        {
            if (miKeyboard != -1)
            {
                pcDevices->GetDevice(miKeyboard)->Process(&psEvent->uEvent.sKeyboard, uiSequence);
            }
        }
        else
        {
            if (miMouse != -1)
            {
                pcDevices->GetDevice(miMouse)->Process(&psEvent->uEvent.sMouse, uiSequence);
            }
        }
    }
    asEvents.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// uiKeyCode is the X key code.  Keys that the standard keyboard doesn't
// have are ignored.
//
//////////////////////////////////////////////////////////////////////////
void CX11Input::Key(unsigned int uiKeyCode, bool bPressed)
{
    size                    i;
    size                    uiNumTranslations;
    unsigned int            uiEventCode;
    SX11KeyTranslation*     psTranslation;
    SX11InputEvent*         psEvent;

    if (uiKeyCode < X11_KEYCODE_OFFSET)
    {
        return;
    }
    uiEventCode = uiKeyCode - X11_KEYCODE_OFFSET;

    uiNumTranslations = sizeof(gasX11KeyTranslations) / sizeof(SX11KeyTranslation);
    for (i = 0; i < uiNumTranslations; i++)
    {
        psTranslation = &gasX11KeyTranslations[i];
        if (psTranslation->usEventCode == uiEventCode)
        {
            psEvent = masEvents.Add();
            memset(psEvent, 0, sizeof(SX11InputEvent));
            psEvent->bKeyboard = true;
            psEvent->uEvent.sKeyboard.usVKey = psTranslation->usVKey;
            psEvent->uEvent.sKeyboard.usMakeCode = psTranslation->usMakeCode;
            psEvent->uEvent.sKeyboard.usFlags = psTranslation->usFlags;
            if (!bPressed)
            {
                psEvent->uEvent.sKeyboard.usFlags |= X11_RI_KEY_BREAK;
            }
            return;
        }
    }
}


//////////////////////////////////////////////////////////////////////////
//
// uiButton is the X button: 1 left, 2 middle, 3 right, 4 and 5 the
// wheel, 6 and 7 the horizontal wheel, 8 back and 9 forward.
//
//////////////////////////////////////////////////////////////////////////
void CX11Input::MouseButton(unsigned int uiButton, bool bPressed)
{
    SX11RawMouseEvent*  psMouse;
    uint16              usFlags;
    int16               sWheel;

    usFlags = 0;
    sWheel = 0;
    switch (uiButton)
    {
        case 1:
            usFlags = bPressed ? X11_RI_MOUSE_BUTTON_1_DOWN : X11_RI_MOUSE_BUTTON_1_UP;
            break;
        case 2:
            usFlags = bPressed ? X11_RI_MOUSE_BUTTON_3_DOWN : X11_RI_MOUSE_BUTTON_3_UP;
            break;
        case 3:
            usFlags = bPressed ? X11_RI_MOUSE_BUTTON_2_DOWN : X11_RI_MOUSE_BUTTON_2_UP;
            break;
        case 4:
        case 5:
            //X 'presses' and 'releases' a wheel button for each notch.  Windows gives each notch once.
            if (bPressed)
            {
                usFlags = X11_RI_MOUSE_WHEEL;
                sWheel = (uiButton == 4) ? X11_WHEEL_DELTA : -X11_WHEEL_DELTA;
            }
            break;
        case 8:
            usFlags = bPressed ? X11_RI_MOUSE_BUTTON_4_DOWN : X11_RI_MOUSE_BUTTON_4_UP;
            break;
        case 9:
            usFlags = bPressed ? X11_RI_MOUSE_BUTTON_5_DOWN : X11_RI_MOUSE_BUTTON_5_UP;
            break;
    }

    if (usFlags != 0)
    {
        psMouse = AddMouseEvent();
        psMouse->usButtonFlags = usFlags;
        psMouse->sButtonData = sWheel;
    }
}


//////////////////////////////////////////////////////////////////////////
//
// Raw input gives the mouse's movement, not where the pointer is.
//
//////////////////////////////////////////////////////////////////////////
void CX11Input::MouseMoved(int iDeltaX, int iDeltaY)
{
    SX11RawMouseEvent*  psMouse;

    psMouse = AddMouseEvent();
    psMouse->lLastX = iDeltaX;
    psMouse->lLastY = iDeltaY;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
SX11RawMouseEvent* CX11Input::AddMouseEvent(void)
{
    SX11InputEvent*     psEvent;

    psEvent = masEvents.Add();
    memset(psEvent, 0, sizeof(SX11InputEvent));
    psEvent->bKeyboard = false;
    return &psEvent->uEvent.sMouse;
}

