#include "EmbeddedObject.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CEmbeddedObject::CEmbeddedObject()
{
	mpcEmbedded = NULL;
}


void CEmbeddedObject::AddFrom(CBaseObject* pcFrom)
{
}

BOOL CEmbeddedObject::IsHollow(void)
{
	return TRUE;
}

void CEmbeddedObject::RemoveFrom(CBaseObject* mpcEmbedding)
{
}

int CEmbeddedObject::NumFroms()
{
	return 0;
}

CBaseObject* CEmbeddedObject::GetFrom(int iFrom)
{
	return NULL;
}

int CEmbeddedObject::RemapTos(CEmbeddedObject* pcOld, CEmbeddedObject* mpcObject)
{
	return 0;
}

void CEmbeddedObject::CopyFroms(CEmbeddedObject* pcOld)
{
}

void CEmbeddedObject::SetDistToRoot(int iDistToRoot)
{
}

int CEmbeddedObject::DistToRoot(void)
{
	return UNATTACHED_DIST_TO_ROOT;
}

OIndex CEmbeddedObject::GetOI(void)
{
	return INVALID_O_INDEX;
}

BOOL CEmbeddedObject::IsNamed()
{
	return FALSE;
}

char* CEmbeddedObject::GetName()
{
	return NULL;
}

BOOL CEmbeddedObject::IsDirty()
{
	return FALSE;
}

void CEmbeddedObject::ClearIndex()
{
}

BOOL CEmbeddedObject::IsBaseObject(void)
{
	return FALSE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CBaseObject* CEmbeddedObject::GetEmbeddingContainer(void)
{
	CEmbeddedObject*	pcContainer;

	pcContainer = this;
	while (pcContainer->IsEmbedded())
	{
		pcContainer = (CEmbeddedObject*)pcContainer->mpcEmbedded;
	}

	return (CBaseObject*)pcContainer;
}




//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CEmbeddedObject::IsNotEmbedded(void)
{
	return mpcEmbedded == NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CEmbeddedObject::IsEmbedded(void)
{
	return mpcEmbedded != NULL;
}



//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CEmbeddedObject::GetEmbeddedIndex(CEmbeddedObject* pcEmbedded)
{
	if (pcEmbedded == this)
	{
		return 0;
	}
	else
	{
		return -1;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CEmbeddedObject::GetNumEmbedded(void)
{
	return 1;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CEmbeddedObject::SetEmbedded(CBaseObject* pcEmbedded)
{
	mpcEmbedded = pcEmbedded;
}

