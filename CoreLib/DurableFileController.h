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
#ifndef __FILE_DISTRIBUTOR_H__
#define __FILE_DISTRIBUTOR_H__
#include "BaseLib/FileBasic.h"
#include "BaseLib/FileUtil.h"
#include "DurableSet.h"


class CDurableFileController
{
private:
	CDurableSet		mcDurableSet;
	CChars			mszDirectory;
	CChars			mszRewriteDirectory;
	BOOL			mbDurable;

public:
	BOOL			Init(char* szDirectory, char* szRewriteDirectory);
	void			Kill(void);

	BOOL			Begin(void);
	BOOL			End(void);
	BOOL			Recover(void);

	void			AddFile(CDurableFile* pcFile);
	int				NumFiles(void);
	CDurableFile*	GetFile(int iIndex);

	BOOL			MakeDir(char* szPathName);
	BOOL			RemoveDir(char* szPathName);

	BOOL			IsBegun(void);
	char*			GetDirectory(void);
	char*			GetRewriteDirectory(void);
	BOOL			IsDurable(void);
	CDurableSet*	GetDurableSet(void);
};


#endif // __FILE_DISTRIBUTOR_H__

