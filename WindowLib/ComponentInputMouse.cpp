/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2012 Andrew Paterson

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
#include "ComponentInputCommand.h"
#include "ComponentInput.h"
#include "ComponentInputMouse.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CComponentInputMouse::Init(CInputDevice* pcDevice, CComponentInput* pcComponentInput)
{
	CInputVirtualDevice*		pcVirtual;

	pcVirtual = pcDevice->CreateDefaultVirtualDeviceFromThis("Text Editor Input");
	InitDevice(pcVirtual, pcDevice, pcComponentInput);
	InitMouse(pcComponentInput->GetInput());
	pcVirtual->Enable();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CComponentInputMouse::InitMouse(CInput* pcInput)
{
	mButton1 =			AddButtonPressedAction( "Button 1", CIC_ActivateComponent, FALSE, FALSE, FALSE);
	mButton1Released =	AddButtonReleasedAction("Button 1", CIC_ExecuteComponent, FALSE, FALSE, FALSE);
	mButton1 =			AddButtonPressedAction( "Button 1", CIC_StartDragSelect, FALSE, FALSE, TRUE);
	mButton1Released =	AddButtonReleasedAction("Button 1", CIC_StopDragSelect, FALSE, FALSE, TRUE);
	mButton2 =			AddButtonPressedAction( "Button 2", CIC_ContextMenu, FALSE, FALSE, FALSE);
	mButton3 =			AddButtonPressedAction( "Button 3", CIC_StartDragScroll, FALSE, FALSE, FALSE);
	mButton3Released =	AddButtonReleasedAction("Button 3", CIC_StopDragScroll, FALSE, FALSE, FALSE);
	mWheelForward =		AddButtonPressedAction( "Wheel Forward", CIC_LineUp, FALSE, FALSE, FALSE);
	mWheelBackward =	AddButtonPressedAction( "Wheel Backward", CIC_LineDown, FALSE, FALSE, FALSE);

	mLeft =				AddRangeAction("Left");
	mRight =			AddRangeAction("Right");
	mUp =				AddRangeAction("Up");
	mDown =				AddRangeAction("Down");

	AddMotion(mLeft->GetSourceDesc(), -1.0f,  0.0f);
	AddMotion(mRight->GetSourceDesc(), 1.0f,  0.0f);
	AddMotion(mUp->GetSourceDesc(),    0.0f, -1.0f);
	AddMotion(mDown->GetSourceDesc(),  0.0f,  1.0f);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CComponentInputMouse::Kill(void)
{
	CComponentInputDevice::Kill();
}

