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
#ifndef __DURABLE_SET_H__
#define __DURABLE_SET_H__
#include "BaseLib/ArrayTemplate.h"
#include "BaseLib/Chars.h"


class CDurableFile;
typedef CArrayTemplate<CDurableFile*>	CArrayDurableFilePtr;


class CDurableSet
{
private:
	CArrayDurableFilePtr	mapcFiles;
	CChars					mszMarkStart;
	CChars					mszMarkRewrite;
	BOOL					mbBegun;

public:
	void	Init(char* szMarkStartFile, char* szMarkRewriteFile);
	void	Kill(void);

	BOOL	Recover(void);
	BOOL	Begin(void);
	BOOL	End(void);
	BOOL	Add(CDurableFile* pcFile);

	void	MarkStart(void);
	void	MarkRewrite(void);
	void	MarkFinish(void);

	BOOL	HasBegun(void);
	BOOL	TestFilesIdentical(void);
	BOOL	CopyBackupToPrimary(void);
	BOOL	CopyPrimaryToBackup(void);
};


#endif // __DURABLE_SET_H__

