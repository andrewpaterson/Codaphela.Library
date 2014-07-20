/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2009 Andrew Paterson

This file is part of The Codaphela Project: Codaphela CoreLib

Codaphela CoreLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela CoreLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela CoreLib.  If not, see <http://www.gnu.org/licenses/>.

Microsoft Windows is Copyright Microsoft Corporation

** ------------------------------------------------------------------------ **/
#ifndef __PROPERTIES_FILE_H__
#define __PROPERTIES_FILE_H__
#include "BaseLib/TextFile.h"
#include "BaseLib/Chars.h"
#include "BaseLib/MapStringString.h"


class CPropertiesFile
{
public:
	CMapStringString	mcProperties;
	CChars				mszName;

	void 	Init(char* szFileName);
	void 	Kill(void);
	char*	Get(char* szProperty);
	void	Set(char* szProperty, char* szValue);
	BOOL	Read(void);
	BOOL	Write(void);
	int		NumProperties(void);
};


#endif // __PROPERTIES_FILE_H__

