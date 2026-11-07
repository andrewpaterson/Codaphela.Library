#ifndef __COMPONENT_INPUT_TEST_DEVICES_H__
#define __COMPONENT_INPUT_TEST_DEVICES_H__
#include "StandardLib/Objects.h"
#include "InputLib/Input.h"
#include "WindowLib/ComponentInput.h"


//////////////////////////////////////////////////////////////////////////
//
// Programmatic devices stand in for a keyboard and a mouse.  Their
// sources have the names a real keyboard and mouse have.
//
//////////////////////////////////////////////////////////////////////////
class CComponentInputTestDevices
{
public:
	CInput*					mpcInput;  //Held by the root from Init to Kill.
	CProgramDeviceDetail*	mpcKeyboard;
	CProgramDeviceDetail*	mpcMouse;
	CComponentInput*		mpcComponentInput;  //Updates the input through its source once it is set.

	void Init(void)
	{
		CProgramInput*	pcProgram;

		Ptr<CInput>		pInput;

		mpcComponentInput = NULL;
		pInput = OMalloc<CInput>((CNativeInput*)NULL);
		ORoot()->Add(pInput);
		mpcInput = &pInput;

		//The details are in an array that moves as it grows so they are only got once both are added.
		pcProgram = mpcInput->GetProgramInput();
		pcProgram->AddDetail("Programmatic: Keyboard", "Test Keyboard");
		pcProgram->AddDetail("Programmatic: Mouse", "Test Mouse");
		mpcKeyboard = pcProgram->macPDeviceDetails.Get(0);
		mpcMouse = pcProgram->macPDeviceDetails.Get(1);

		mpcKeyboard->AddSource("Left Shift");
		mpcKeyboard->AddSource("Right Shift");
		mpcKeyboard->AddSource("Left Control");
		mpcKeyboard->AddSource("Left Alt");
		mpcKeyboard->AddSource("Right Alt");
		mpcKeyboard->AddSource("N");
		mpcKeyboard->AddSource("B");
		mpcKeyboard->AddSource("O");
		mpcKeyboard->AddSource("Caps Lock");
		mpcKeyboard->AddSource("Num Lock");
		mpcKeyboard->AddSource("Insert");
		mpcKeyboard->AddSource("A");
		mpcKeyboard->AddSource("C");
		mpcKeyboard->AddSource("S");
		mpcKeyboard->AddSource("1");
		mpcKeyboard->AddSource("8");
		mpcKeyboard->AddSource("Space");
		mpcKeyboard->AddSource("Tab");
		mpcKeyboard->AddSource("Enter");
		mpcKeyboard->AddSource("End");
		mpcKeyboard->AddSource("Cursor Up");
		mpcKeyboard->AddSource("Cursor Left");
		mpcKeyboard->AddSource("Numpad 1");
		mpcKeyboard->AddSource("Numpad 5");
		mpcKeyboard->AddSource("Numpad 8");
		mpcKeyboard->AddSource("Numpad 0");
		mpcKeyboard->AddSource("F1");
		mpcKeyboard->AddSource("F6");
		mpcKeyboard->AddSource("F7");
		mpcKeyboard->AddSource("Backspace");

		mpcMouse->AddSource("Button 1");
		mpcMouse->AddSource("Button 2");
		mpcMouse->AddSource("Button 3");
		mpcMouse->AddSource("Wheel Forward");
		mpcMouse->AddSource("Wheel Backward");

		mpcInput->AddProgramInput();
	}

	void Kill(void)
	{
		ORoot()->Remove(mpcInput);
		mpcInput = NULL;
	}

	CInputDevice* GetKeyboard(void)
	{
		return mpcInput->GetDevices()->GetDevice(mpcKeyboard->miUniqueID);
	}

	CInputDevice* GetMouse(void)
	{
		return mpcInput->GetDevices()->GetDevice(mpcMouse->miUniqueID);
	}

	void Event(CProgramDeviceDetail* pcDetail, char* szSource, float fValue)
	{
		pcDetail->AddEvent(szSource, fValue);
		if (mpcComponentInput)
		{
			mpcComponentInput->Update();
		}
		else
		{
			mpcInput->Update();
		}
	}

	void Press(char* szKey) { Event(mpcKeyboard, szKey, 1.0f); }
	void Release(char* szKey) { Event(mpcKeyboard, szKey, 0.0f); }
	void Tap(char* szKey) { Press(szKey); Release(szKey); }
	void PressButton(char* szButton) { Event(mpcMouse, szButton, 1.0f); }
	void ReleaseButton(char* szButton) { Event(mpcMouse, szButton, 0.0f); }
};


#endif // __COMPONENT_INPUT_TEST_DEVICES_H__

