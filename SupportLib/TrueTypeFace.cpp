/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2026 Andrew Paterson

This file is part of The Codaphela Project: Codaphela SupportLib

Codaphela SupportLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela SupportLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela SupportLib.  If not, see <http://www.gnu.org/licenses/>.

** ------------------------------------------------------------------------ **/
#include "StandardLib/ClassDefines.h"
#include "TrueTypeFace.h"


//////////////////////////////////////////////////////////////////////////
//
// Not loaded until Load is called.
//
//////////////////////////////////////////////////////////////////////////
void CTrueTypeFace::Init(void)
{
	PreInit();

	mszFilename.Init();
	mcFont.Init();
	mbLoaded = false;

	PostInit();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTrueTypeFace::Class(void)
{
	U_String(mszFilename);
	U_Data(CTrueTypeFont, mcFont);
	U_Bool(mbLoaded);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTrueTypeFace::Free(void)
{
	mcFont.Kill();
	mszFilename.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CTrueTypeFace::Save(CObjectWriter* pcFile)
{
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CTrueTypeFace::Load(CObjectReader* pcFile)
{
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
// Returns false if the file can't be read or isn't a TrueType font.
//
//////////////////////////////////////////////////////////////////////////
bool CTrueTypeFace::Load(const char* szFilename)
{
	mszFilename.Kill();
	mszFilename.Init(szFilename);
	mcFont.Kill();
	mcFont.Init();
	mbLoaded = mcFont.Load((char*)szFilename);
	return mbLoaded;
}


bool CTrueTypeFace::IsLoaded(void) { return mbLoaded; }
CTrueTypeFont* CTrueTypeFace::GetFont(void) { return &mcFont; }
char* CTrueTypeFace::GetFilename(void) { return mszFilename.Text(); }

