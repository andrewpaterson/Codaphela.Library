#include "SerialisedObject.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CSerialisedObject::IsNamed(void)
{
	BOOL bNamed;

	bNamed = *((int*)&mszType) == OBJECT_POINTER_NAMED;
	if (!bNamed)
	{
		return FALSE;
	}

	if (msName.miLength == 1)
	{
		return FALSE;
	}

	return TRUE;
}


////////////////////////////////////////////////////////////////////////////
////
////
////////////////////////////////////////////////////////////////////////////
BOOL CSerialisedObject::IsIndexed(void)
{
	BOOL bIndexed;
	BOOL bNamed;

	bIndexed = *((int*)&mszType) == OBJECT_POINTER_ID;
	if (bIndexed)
	{
		return TRUE;
	}

	bNamed = *((int*)&mszType) == OBJECT_POINTER_NAMED;
	if (bNamed && msName.miLength == 1)
	{
		return TRUE;
	}

	return FALSE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CSerialisedObject::IsVoid(void)
{
	return *((int*)&mszType) == OBJECT_POINTER_NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
char* CSerialisedObject::GetName(void)
{
	return msName.msz;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
OIndex CSerialisedObject::GetIndex(void)
{
	return moi;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CSerialisedObject::GetLength(void)
{
	return miLength;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CSerialisedObject::SetLength(int iLength)
{
	miLength = iLength;
}

