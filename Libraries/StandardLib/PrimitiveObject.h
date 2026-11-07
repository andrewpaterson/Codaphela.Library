/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2026 Andrew Paterson

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
#ifndef __PRIMITIVE_OBJECT_H__
#define __PRIMITIVE_OBJECT_H__
#include "BaseLib/PrimitiveTypes.h"
#include "BaseLib/DataTypes.h"


#define LOG_PRIMITIVE_ACCESS_DEBUG()	LogPrimitiveAccessDebug((CPrimitiveObject*)this, __ENGINE_PRETTY_FUNCTION__)
#define LOG_PRIMITIVE_ASSIGN_DEBUG()	LogPrimitiveAssignDebug((CPrimitiveObject*)this, __ENGINE_PRETTY_FUNCTION__)


class CObject;
class CPrimitiveObject
{
private:
	uint16	muiClassType;  //EPrimitiveType
	uint16	muiNegativeEmbeddingOffset;

public:
					CPrimitiveObject(EPrimitiveType eType);

	EPrimitiveType	GetClassType(void);

	void			SetEmbedding(CObject* pcEmbedding);
	void			SetDirty(void);

	CObject*		Embedding(void);
};


void LogPrimitiveAccessDebug(CPrimitiveObject* pvThis, const char* szMethod);
void LogPrimitiveAssignDebug(CPrimitiveObject* pvThis, const char* szMethod);


#endif // __PRIMITIVE_OBJECT_H__

