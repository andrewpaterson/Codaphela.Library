/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2009 Andrew Paterson

This file is part of The Codaphela Project: Codaphela StandardLib

Codaphela StandardLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela StandardLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela StandardLib.  If not, see <http://www.gnu.org/licenses/>.

** ------------------------------------------------------------------------ **/
#ifndef __STANDARD_HEADER_H__
#define __STANDARD_HEADER_H__
#include "BaseLib/ChunkFile.h"
#include "Unknown.h"


class CStandardTrackerObject : public CUnknown
{
BASE_FUNCTIONS(CStandardTrackerObject);
protected:
	int				miUniqueID;
	CChars			mszName;

public:
	void			Init(void);
	void			Init(int iUniqueID, char* szName);  
	int				GetID(void);
	char*			GetName(void);
	BOOL			Load(CFileReader* pcFile);
	virtual BOOL	LoadSpecific(CFileReader* pcSrc, int iChunkNum) =0;
	BOOL			BeginLoadStandardTrackerObject(CFileReader* pcFile, int iChunkNum);
	BOOL			BeginSaveStandardTrackerObject(CFileWriter* pcFile);
	BOOL			EndLoadStandardTrackerObject(CFileReader* pcFile);
	BOOL			EndSaveStandardTrackerObject(CFileWriter* pcFile);
	void			SetName(char* szName);
	void			SetID(int iID);
	BOOL			HasName(void);
};


#endif // __STANDARD_HEADER_H__

