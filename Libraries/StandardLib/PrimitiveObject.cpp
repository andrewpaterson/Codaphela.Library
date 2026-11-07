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
#include "BaseLib/Define.h"
#include "BaseLib/Logger.h"
#include "Object.h"
#include "PrimitiveObject.h"


//#define DEBUG_PRIMITIVE
//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void LogPrimitiveAccessDebug(CPrimitiveObject* pvThis, const char* szMethod)
{
#ifdef DEBUG_PRIMITIVE
#ifdef _DEBUG
	const char* szEmbeddingClass;
	char* szEmbeddingName;
	char* szEmbeddingIndex;
	char* szEmbeddingAddress;

	CObject* pcEmbedding;

	pcEmbedding = pvThis->Embedding();
	if (pcEmbedding != NULL)
	{
		szEmbeddingClass = pcEmbedding->ClassName();
		szEmbeddingIndex = IndexToString(pcEmbedding->GetIndex());
		szEmbeddingName = pcEmbedding->GetName();
		szEmbeddingAddress = PointerToString(pcEmbedding);

		gcLogger.Debug2(PointerToString(pvThis), " ", szMethod, " [Embedding ", szEmbeddingClass, ": ", szEmbeddingIndex, " ", szEmbeddingName, " (", szEmbeddingAddress, ")]", NULL);
	}
	else
	{
		gcLogger.Debug2(PointerToString(pvThis), "->", szMethod, " [Embedding NULL]", NULL);
	}


#endif // _DEBUG
#endif // DEBUG_PRIMITIVE
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void LogPrimitiveAssignDebug(CPrimitiveObject* pvThis, const char* szMethod)
{
#ifdef DEBUG_PRIMITIVE
#ifdef _DEBUG
	const char* szEmbeddingClass;
	char* szEmbeddingName;
	char* szEmbeddingIndex;
	char* szEmbeddingAddress;

	CObject* pcEmbedding;

	pcEmbedding = pvThis->Embedding();
	if (pcEmbedding != NULL)
	{
		szEmbeddingClass = pcEmbedding->ClassName();
		szEmbeddingIndex = IndexToString(pcEmbedding->GetIndex());
		szEmbeddingName = pcEmbedding->GetName();
		szEmbeddingAddress = PointerToString(pcEmbedding);

		gcLogger.Debug2(PointerToString(pvThis), " ", szMethod, " [Embedding ", szEmbeddingClass, ": ", szEmbeddingIndex, " ", szEmbeddingName, " (", szEmbeddingAddress, ")]", NULL);
	}
	else
	{
		gcLogger.Debug2(PointerToString(pvThis), "->", szMethod, " [Embedding NULL]", NULL);
	}


#endif // _DEBUG
#endif // DEBUG_PRIMITIVE
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CPrimitiveObject::CPrimitiveObject(EPrimitiveType eType)
{
	muiClassType = eType;
	muiNegativeEmbeddingOffset = 0;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CPrimitiveObject::SetEmbedding(CObject* pcEmbedding)
{
	muiNegativeEmbeddingOffset = (uint16)((uint8*)this - (uint8*)pcEmbedding);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CPrimitiveObject::SetDirty(void)
{
	CObject*	pcEmbedding;

	pcEmbedding = Embedding();
	if (pcEmbedding)
	{
		pcEmbedding->GetEmbeddingContainer()->SetDirty(true);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CObject* CPrimitiveObject::Embedding(void)
{
	CObject* pcEmbedding;

	if (muiNegativeEmbeddingOffset != 0)
	{
		pcEmbedding = (CObject*)RemapSinglePointer(this, -((ptrdiff_t)muiNegativeEmbeddingOffset));
		return pcEmbedding;
	}
	else
	{
		return NULL;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
EPrimitiveType CPrimitiveObject::GetClassType(void)
{
	return (EPrimitiveType)muiClassType;
}


