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
public:
	CDurableSet		mcDurableSet;
	CChars			mszWorkingDirectory;
	BOOL			mbDurable;

	void			Init(char* szWorkingDirectory, BOOL bDurable);
	void			Kill(void);

	void			AddFile(CDurableFile* pcFile);

	BOOL			MakeDir(char* szPathName);
	BOOL			RemoveDir(char* szPathName);
};


#endif // __FILE_DISTRIBUTOR_H__

