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
#ifdef LINUX_GNU
#include <dlfcn.h>
#include "SharedLibrary.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CSharedLibrary::Init(void)
{
	mpvHandle = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CSharedLibrary::Kill(void)
{
	Close();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CSharedLibrary::IsOpen(void)
{
	return mpvHandle != NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// Closes what it had first.  What it needs is found now, not when it's
// first called.
//
//////////////////////////////////////////////////////////////////////////
bool CSharedLibrary::Open(const char* szName)
{
	Close();
	mpvHandle = dlopen(szName, RTLD_NOW | RTLD_LOCAL);
	return mpvHandle != NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CSharedLibrary::Close(void)
{
	if (mpvHandle)
	{
		dlclose(mpvHandle);
		mpvHandle = NULL;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void* CSharedLibrary::GetFunction(const char* szName)
{
	if (mpvHandle == NULL)
	{
		return NULL;
	}
	return dlsym(mpvHandle, szName);
}


#endif // LINUX_GNU

