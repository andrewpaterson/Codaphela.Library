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
#ifndef __UNKNOWN_H__
#define __UNKNOWN_H__
#include "BaseLib/ChunkFile.h"
#include "Base.h"


class CUnknowns;
class CUnknown : public CBase
{
protected:
	CUnknowns*	mpcUnknownsThisIn;

public:
						CUnknown(void);
			void		PreInit(CUnknowns* pcUnknowns);
	virtual void 		Kill(void);

	virtual BOOL		IsUnknown(void);
	virtual BOOL		Iterable(void);
			BOOL		SaveHeader(CFileWriter* pcFile);
	virtual BOOL		Save(CFileWriter* pcFile);
	virtual BOOL		Load(CFileReader* pcFile);
	virtual int			Compare(CUnknown* pcRight);
			CUnknowns*	GetUnknownsThisIn(void);
			void		SetNotInUnknowns(void);
};


#endif // __UNKNOWN_H__

