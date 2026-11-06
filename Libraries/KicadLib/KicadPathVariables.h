/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2026 Andrew Paterson

This file is part of The Codaphela Project: Codaphela KicadLib

Codaphela KicadLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela KicadLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela KicadLib.  If not, see <http://www.gnu.org/licenses/>.

** ------------------------------------------------------------------------ **/
#ifndef __KICAD_PATH_VARIABLES_H__
#define __KICAD_PATH_VARIABLES_H__
#include "BaseLib/Chars.h"
#include "BaseLib/ArrayChars.h"


//KiCad's path variables, as "${_658_COMPUTER_}" in "${_658_COMPUTER_}/Kicad/Computer/...", as KiCad finds them: one
//set here first, then the environment's, then those KiCad keeps in its kicad_common.json (Preferences > Configure
//Paths), read by LoadKicadCommon.
class CKicadPathVariables
{
protected:
	CArrayChars		maszNames;
	CArrayChars		maszValues;
	CArrayChars		maszKicadNames;
	CArrayChars		maszKicadValues;

public:
	void		Init(void);
	void		Kill(void);

	void		Set(const char* szName, const char* szValue);
	bool		LoadKicadCommon(void);
	bool		LoadKicadCommon(const char* szFileName);
	bool		ReadKicadCommon(const char* szJSON);

	const char*	Get(const char* szName);
	bool		Resolve(const char* szPath, const char* szRelativeTo, CChars* pszDest);
};


#endif // __KICAD_PATH_VARIABLES_H__
