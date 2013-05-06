/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2009 Andrew Paterson

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
#ifndef __LOGGER_H__
#define __LOGGER_H__
#include "Define.h"


class CAbstractFile;
class CLogger
{
protected:
	CAbstractFile*	mpcFile;
	BOOL			mbEnabled;
	BOOL			mbFreeFile;
	BOOL			mbEngineOut;

public:
	void Init(void);
	void Init(char* szName);
	void Init(CAbstractFile* pcFile, char* szName);
	void Kill(void);
	void Add(char* szText);
	void Add(char* szErrorLevel, char* szText);
	void Error(char* szText);
	void Error2(char* szText, ...);
	void Warning(char* szText);
	void Info(char* szText);
	void Debug(char* szText);
	void Disable(void);
	void Enable(void);
	void SetEngineOutput(BOOL bEngineOut);
};


extern CLogger gcLogger;


#endif //__LOGGER_H__

