/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2022 Andrew Paterson

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
#include "LogConfig.h"
#include "LogString.h"
#include "ArrayTemplate.h"


typedef CArrayTemplate<CAbstractFile*> CArrayFile;


class CAbstractFile;
class CLogger
{
protected:
	CArrayFile		mapcFiles;
	SLogConfig		msConfig;
	CAbstractFile*	mpcDebugOutputFile;

public:
	void		Init(void);
	void		Init(const char* szName);
	void		Init(CAbstractFile* pcFile, const char* szName);
	void		Kill(void);

	void		Add(const char* szText);
	void		Add(const char* szErrorLevel, const char* szText);
	BOOL		Error(const char* szText);
	BOOL		Error2(const char* szText, ...);
	BOOL		Warning(const char* szText);
	BOOL		Warning2(const char* szText, ...);
	void		Info(const char* szText);
	void		Info2(const char* szText, ...);
	void		Debug(const char* szText);
	void		Debug2(const char* szText, ...);

	void		Disable(void);
	void		Enable(void);
	void		SetBreakOnError(BOOL bBreakOnError);
	void		SetBreakOnWarning(BOOL bBreakOnWarning);
	SLogConfig	SetSilent(void);
	void		SetConfig(SLogConfig* psConfig);
	void		GetConfig(SLogConfig* psConfig);

	void		AddOutput(CAbstractFile* pcFile);
	void		RemoveOutput(CAbstractFile* pcFile);

	void		Break(void);
};


extern CLogger gcLogger;


#endif // !__LOGGER_H__

