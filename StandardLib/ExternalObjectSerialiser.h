/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2012 Andrew Paterson

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
#ifndef __EXTERNAL_OBJECT_SERIALISER_H__
#define __EXTERNAL_OBJECT_SERIALISER_H__
#include "BaseObject.h"
#include "ExternalObjectWriter.h"
#include "DependentWriteObjects.h"


class CExternalObjectSerialiser
{
protected:
	CExternalObjectWriter*	mpcWriter;
	CDependentWriteObjects	mcDependentObjects;

public:
	void	Init(CExternalObjectWriter* pcWriter);
	void	Kill(void);

	BOOL	Write(CBaseObject* pcObject);
	void	AddDependent(CBaseObject* pcObject);

protected:
	BOOL	WriteUnwritten(CBaseObject* pcObject);
	void	MarkWritten(CBaseObject* pcObject);
};


#endif // !__EXTERNAL_OBJECT_SERIALISER_H__

