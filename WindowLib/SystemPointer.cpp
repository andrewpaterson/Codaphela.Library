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
#include "BaseLib/WindowsHeaders.h"
#include "SystemPointer.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CSystemPointer::Init(void)
{
	Update();
	mfSpeed = 1.5f;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CSystemPointer::Kill(void)
{
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CSystemPointer::Update(void)
{
	POINT	point;

	GetCursorPos(&point);
	if (!((point.x == (int)msPosition.x) && (point.y == (int)msPosition.y)))
	{
		msPosition.x = (float)point.x;
		msPosition.y = (float)point.y;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CSystemPointer::Set(int x, int y)
{
	SetCursorPos(x, y);
	msPosition.x = (float)x;
	msPosition.y = (float)y;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CSystemPointer::Move(int x, int y)
{
	msPosition.x += (float)x;
	msPosition.y += (float)y;
	SetCursorPos((int)msPosition.x, (int)msPosition.y);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CSystemPointer::Set(float x, float y)
{
	SetCursorPos((int)x, (int)y);
	msPosition.x = x;
	msPosition.y = y;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CSystemPointer::Move(float x, float y)
{
	msPosition.x += x;
	msPosition.y += y;
	SetCursorPos((int)msPosition.x, (int)msPosition.y);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CSystemPointer::GetX(void)
{
	return (int)msPosition.x;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CSystemPointer::GetY(void)
{
	return (int)msPosition.y;
}
