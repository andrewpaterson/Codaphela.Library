/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2026 Andrew Paterson

This file is part of The Codaphela Project: Codaphela OldDragonLib

Codaphela OldDragonLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela OldDragonLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela OldDragonLib.  If not, see <http://www.gnu.org/licenses/>.

** ------------------------------------------------------------------------ **/
#include "BaseLib/Logger.h"
#include "OldDragonReader.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool ReadOldDragonFile(const char* szFilename, COldDragonScene* pcScene)
{
	COldDragonFile	cFile;
	bool			bResult;

	bResult = cFile.Init(szFilename);
	if (bResult)
	{
		pcScene->meVersion = cFile.GetVersion();
		pcScene->mbDamaged = cFile.IsDamaged();
		switch (cFile.GetVersion())
		{
		case ODV_IOX:
			bResult = ReadOldDragonIOX(&cFile, pcScene);
			break;
		case ODV_August2009:
		case ODV_September2009:
			bResult = ReadOldDragon2009(&cFile, pcScene);
			break;
		case ODV_Max:
			bResult = ReadOldDragonMax(&cFile, pcScene);
			break;
		default:
			bResult = false;
			break;
		}

		if (!bResult)
		{
			gcLogger.Error2(__METHOD__, " Could not read the contents of file [", szFilename, "] as version [", OldDragonVersionName(cFile.GetVersion()), "].", NULL);
		}
	}
	cFile.Kill();
	return bResult;
}

