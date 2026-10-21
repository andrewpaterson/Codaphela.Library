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
#ifndef __DURABLE_SET_H__
#define __DURABLE_SET_H__
#include "ArrayTemplatePtr.h"
#include "Chars.h"
#include "ArrayChars.h"


class CDurableFile;
typedef CArrayTemplatePtr<CDurableFile>	CArrayDurableFilePtr;


class CDurableSet
{
private:
	CArrayDurableFilePtr	mapcFiles;
	CChars					mszMarkStart;
	CChars					mszMarkRewrite;
	bool					mbBegun;
	bool					mbSync;

public:
	void			Init(const char* szMarkStartFile, const char* szMarkRewriteFile);
	void			Kill(void);

	bool			HasBegun(void);
	void			SetSync(bool bSync);

	bool			Recover(void);
	bool			Begin(void);
	bool			End(void);
	void			Abort(void);
	bool			Check(bool bThorough, bool bLogError);

	bool			Add(CDurableFile* pcFile);
	size			Num(void);
	CDurableFile*	Get(size iIndex);

	bool			MarkStart(void);
	bool			MarkRewrite(void);
	bool			MarkFinish(void);

private:
	bool			Mark(const char* szMarkFile, CChars* pszContents);
	bool			ReadMarkedFiles(CArrayChars* paszFiles, bool* pbComplete);
	bool			CopyRecoveryFiles(bool bBackupToPrimary);
	bool			ContainsPair(CArrayChars* paszFiles, const char* szPrimary);
	void			GetMarks(bool* pbMarkStart, bool* pbMarkRewrite);
	bool			CopyRecoveredFile(const char* szSource, const char* szDest);
	bool			Unmark(const char* szMarkFile);
	bool			CheckFilesIdentical(bool bThorough, bool bLogError);
	bool			CheckWriteStatus(bool bMarkStart, bool bMarkRewrite, bool bLogError);
	bool			CheckWriteStatus(bool bLogError);
};


#endif // __DURABLE_SET_H__

