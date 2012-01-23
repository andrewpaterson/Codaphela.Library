/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2012 Andrew Paterson

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
#ifndef __FILES_H__
#define __FILES_H__
#include "BaseLib/DiskFile.h"
#include "BaseLib/PackFile.h"
#include "BaseLib/FileSystem.h"
#include "BaseLib/MapStringInt.h"
#include "PackFileOffset.h"


typedef CArrayTemplate<CPackFileOffset>	CArrayPackFiles;


class CFiles
{
protected:
	CFileSystem			mcFileSystem;
	CArrayPackFiles		mcPackFilesArray;
	CChars				mszPackFilesExtension;

public:

	BOOL			Init(char* szDirectory, char* szPackFilesExtension);
	void			Kill(void);

	int				GetNumPackFiles(void);
	CAbstractFile*	GetFile(char* szFullName);
	void			GetFileNames(CMapStringInt* cFileNames);

protected:
	CDiskFile*		GetSystemFile(char* szFullName);
	CPackFile*		GetPackFile(char* szFullName);
	CPackFile*		GetPackFile(CPackFileOffset* pcPackFiles, char* szFullName);
};


int ComparePackFileOffset(const void* arg1, const void* arg2);


#endif // __FILES_H__

