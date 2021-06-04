#include "BaseLib/Define.h"
#include "BaseLib/Logger.h"
#include "Object.h"
#include "PrimitiveObject.h"


//#define DEBUG_PRIMITIVE
//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void LogPrimitiveAccessDebug(CPrimitiveObject* pvThis, char* szMethod)
{
#ifdef DEBUG_PRIMITIVE
#ifdef DEBUG
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


#endif // DEBUG
#endif // DEBUG_POINTER
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void LogPrimitiveAssignDebug(CPrimitiveObject* pvThis, char* szMethod)
{
#ifdef DEBUG_PRIMITIVE
#ifdef DEBUG
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


#endif // DEBUG
#endif // DEBUG_POINTER
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
	muiNegativeEmbeddingOffset = ((uint8*)this - (uint8*)pcEmbedding);
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
		pcEmbedding->GetEmbeddingContainer()->SetDirty(TRUE);
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


