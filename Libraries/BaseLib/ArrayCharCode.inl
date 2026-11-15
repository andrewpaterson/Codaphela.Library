/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2026 Andrew Paterson

This file is part of The Codaphela Project: Codaphela BaseLib

Codaphela BaseLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela BaseLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela BaseLib.  If not, see <http://www.gnu.org/licenses/>.

Microsoft Windows is Copyright Microsoft Corporation

** ------------------------------------------------------------------------ **/
//CArrayCharBase's functions, for either array size.  ArrayChar.cpp includes it and instantiates them for both;
//ArrayChar.h declares them extern, so no other unit compiles them.

//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class PRIMITIVE>
void CArrayCharBase<PRIMITIVE>::AddList(char iStop, ...)
{
	va_list		vaMarker;
	char		iValue;

	va_start(vaMarker, iStop);
	iValue = (char)va_arg(vaMarker, int);  //char is promoted to int when passed through ...
	while (iValue != iStop)
	{
		this->Add((char)iValue);
		iValue = (char)va_arg(vaMarker, int);  //char is promoted to int when passed through ...
	}
	va_end(vaMarker);
}

