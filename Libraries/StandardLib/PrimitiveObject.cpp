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
// With DEBUG_PRIMITIVE (and _DEBUG), logs szMethod on the primitive and
// the object it is embedded in.  Both LOG_PRIMITIVE_ACCESS_DEBUG and
// LOG_PRIMITIVE_ASSIGN_DEBUG come here.
//
//////////////////////////////////////////////////////////////////////////
void LogPrimitiveDebug(CPrimitiveObject* pcThis, const char* szMethod)
{
#ifdef DEBUG_PRIMITIVE
#ifdef _DEBUG
	CObject*	pcEmbedding;

	pcEmbedding = pcThis->Embedding();
	if (pcEmbedding != NULL)
	{
		gcLogger.Debug2(PointerToString(pcThis), " ", szMethod, " [Embedding ", pcEmbedding->ClassName(), ": ", IndexToString(pcEmbedding->GetIndex()), " ", pcEmbedding->GetName(), " (", PointerToString(pcEmbedding), ")]", NULL);
	}
	else
	{
		gcLogger.Debug2(PointerToString(pcThis), " ", szMethod, " [Embedding NULL]", NULL);
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
	ptr		uiOffset;

	//0 is not embedded: a primitive can't be at its object's start (the object's own fields are there).
	uiOffset = (ptr)((uint8*)this - (uint8*)pcEmbedding);
	if (uiOffset > MAX_UINT16)
	{
		//The object is still being set up (its Class), so it isn't printed.
		gcLogger.Error2(__METHOD__, " A primitive [", SizeToString((size)uiOffset), "] bytes into its object can't find it (64KB at most).", NULL);
		muiNegativeEmbeddingOffset = 0;
		return;
	}
	muiNegativeEmbeddingOffset = (uint16)uiOffset;
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


