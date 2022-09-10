#include "BaseObject.h"
#include "HollowObject.h"
#include "Objects.h"
#include "HollowEmbeddedObject.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CHollowEmbeddedObject::Save(CObjectWriter* pcFile)
{
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CHollowEmbeddedObject::Load(CObjectReader* pcFile)
{
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CHollowEmbeddedObject::RemovePointerTo(CEmbeddedObject* pcTo)
{
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
uint16 CHollowEmbeddedObject::GetNumEmbedded(void)
{
	return 0;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CEmbeddedObject* CHollowEmbeddedObject::GetEmbeddedObject(uint16 iIndex)
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
bool CHollowEmbeddedObject::IsHollow(void)
{
	return true;
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
void CHollowEmbeddedObject::FreeInternal(void)
{
	FreePointers();
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
void CHollowEmbeddedObject::SetPointerTosExpectedDistToRoot(int iDistToRoot)
{
	NotImplemented(__METHOD__);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CHollowEmbeddedObject::SetDistToRoot(int iDistToRoot)
{
	NotImplemented(__METHOD__);
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CObjects* CHollowEmbeddedObject::GetObjectsThisIn(void)
{
	CHollowObject*	pcHollow;

	pcHollow = GetHollowObject();
	return pcHollow->GetObjectsThisIn();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CStackPointers* CHollowEmbeddedObject::GetStackPointers(void)
{
	return &gcStackPointers;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CHollowEmbeddedObject::GetPointerTos(CArrayTemplateEmbeddedObjectPtr* papcTos)
{
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CHollowEmbeddedObject::NumPointerTos(void)
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
void CHollowEmbeddedObject::UpdateAttachedEmbeddedObjectPointerTosDistToRoot(CDistCalculatorParameters* pcParameters, int iExpectedDist)
{
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CHollowEmbeddedObject::CollectDetachedFroms(CDistCalculatorParameters* pcParameters)
{
	return 0;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CHollowEmbeddedObject::IsInitialised(void)
{
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CHollowEmbeddedObject::IsDirty(void)
{
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CHollowEmbeddedObject::IsRoot(void)
{
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CHollowEmbeddedObject::TestedForRoot(void)
{
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CHollowEmbeddedObject::SetDirty(bool bDirty)
{
}

