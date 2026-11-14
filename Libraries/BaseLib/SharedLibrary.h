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
#ifndef __SHARED_LIBRARY_H__
#define __SHARED_LIBRARY_H__
#include "PrimitiveTypes.h"


//A library of code loaded while the program runs: a .dll on Windows, a .so on Linux.  The 65C64 has none, so Open always
//fails there.  Open takes the name the system looks for, as "vulkan-1.dll" or "libvulkan.so.1", and is false if it isn't found.
//GetFunction is NULL for a name the library hasn't got, or when it isn't open.  Kill closes it.
class CSharedLibrary
{
protected:
	void*	mpvHandle;

public:
	void	Init(void);
	void	Kill(void);

	bool	Open(const char* szName);
	void	Close(void);
	bool	IsOpen(void);
	void*	GetFunction(const char* szName);
};


#endif // __SHARED_LIBRARY_H__

