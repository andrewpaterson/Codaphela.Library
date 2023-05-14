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
#ifndef __FILE_DISTRIBUTOR_H__
#define __FILE_DISTRIBUTOR_H__
#include "FileBasic.h"
#include "FileUtil.h"
#include "MapStringInt.h"
#include "DurableFile.h"
#include "DurableSet.h"


#define DURABLE_FILE_MAGIC	'ELIF'


class CDurableFileController
{
private:
	CDurableSet		mcDurableSet;
	CChars			mszDirectory;
	CChars			mszRewriteDirectory;
	bool			mbDurable;
	CDurableFile	mcFileList;
	CMapStringInt	mcNameMap;

public:
	bool			Init(char* szDirectory);
	bool			Init(char* szDirectory, char* szRewriteDirectory, char* szFileListWrite = NULL, char* szFileListRewrite = NULL, char* szMarkWrite = NULL, char* szMarkRewrite = NULL);
	void			Kill(void);

	bool			Begin(void);
	bool			Begin(CDurableFile* pcFirst, ...);
	bool			End(void);
	bool			Recover(void);
	bool			Recover(CDurableFile* pcFirst, ...);
	bool			Check(bool bThorough, bool bLogError);

	bool			AddFile(CDurableFile* pcFile);
	int				NumFiles(void);
	CDurableFile*	GetFile(int iIndex);

	bool			IsBegun(void);
	char*			GetDirectory(void);
	char*			GetRewriteDirectory(void);
	bool			IsDurable(void);
	CDurableSet*	GetDurableSet(void);

	bool			ReadControlledFileList(CDurableFile* pcFile);
	bool			WriteControlledFileList(CDurableFile* pcFile);
};


#endif // !__FILE_DISTRIBUTOR_H__

