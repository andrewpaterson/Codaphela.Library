#include "BaseObject.h"
#include "HollowObject.h"
#include "Objects.h"
#include "HollowEmbeddedObject.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CHollowEmbeddedObject::Save(CObjectSerialiser* pcFile)
{
	return FALSE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CHollowEmbeddedObject::Load(CObjectDeserialiser* pcFile)
{
	return FALSE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CHollowEmbeddedObject::RemoveTo(CEmbeddedObject* pcTo)
{
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CHollowEmbeddedObject::GetNumEmbedded(void)
{
	return 0;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CEmbeddedObject* CHollowEmbeddedObject::GetEmbeddedObject(int iIndex)
{
	if (iIndex == 0)
	{
		return this;
	}

	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CHollowEmbeddedObject::GetDistToStack(void)
{
	return GetHollowObject()->GetDistToStack();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CHollowEmbeddedObject::GetDistToRoot(void)
{
	return GetHollowObject()->GetDistToRoot();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CHollowEmbeddedObject::IsHollow(void)
{
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CBaseObject* CHollowEmbeddedObject::Dehollow(void)
{
	CBaseObject*	pcEmbedding;
	int				iEmbeddedIndex;
	CBaseObject*	pcThisDehollowed;

	iEmbeddedIndex = GetEmbeddedIndex();

	pcEmbedding = mpcEmbedded->Dehollow();
	pcThisDehollowed = (CBaseObject*)pcEmbedding->GetEmbeddedObject(iEmbeddedIndex);
	return pcThisDehollowed;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CHollowEmbeddedObject::GetEmbeddedIndex(void)
{
	return mpcEmbedded->GetEmbeddedIndex(this);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CHollowEmbeddedObject::KillDontFree(void)
{
	KillInternalData();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CHollowObject* CHollowEmbeddedObject::GetHollowObject(void)
{
	return (CHollowObject*)mpcEmbedded;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CHollowEmbeddedObject::SetPointedTosExpectedDistToRoot(int iDistToRoot)
{
	NotImplemented(__METHOD__);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CHollowEmbeddedObject::SetDistToRoot(int iDistToRoot)
{
	NotImplemented(__METHOD__);
	return FALSE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CObjects* CHollowEmbeddedObject::GetObjects(void)
{
	CHollowObject*	pcHollow;

	pcHollow = GetHollowObject();
	return pcHollow->GetObjects();
}



//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CStackPointers* CHollowEmbeddedObject::GetStackPointers(void)
{
	CObjects*	pcObjects;

	pcObjects = GetObjects();
	if (pcObjects)
	{
		return pcObjects->GetStackPointers();
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
void CHollowEmbeddedObject::GetTos(CArrayEmbeddedObjectPtr* papcTos)
{
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CHollowEmbeddedObject::NumTos(void)
{
	return 0;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CHollowEmbeddedObject::ValidateConsistency(void)
{
	ValidateNotEmbedded(__METHOD__);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CHollowEmbeddedObject::UpdateAttachedEmbeddedObjectTosDistToRoot(CDistCalculatorParameters* pcParameters, int iExpectedDist)
{
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CHollowEmbeddedObject::CollectEmbeddedObjectDetachedFroms(CDistCalculatorParameters* pcParameters)
{
	return 0;
}

