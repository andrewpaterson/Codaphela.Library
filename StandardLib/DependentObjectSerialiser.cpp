#include "ObjectGraphSerialiser.h"
#include "PointerObject.h"
#include "DependentObjectSerialiser.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDependentObjectSerialiser::Init(CObjectGraphSerialiser* pcGraphSerialiser, CBaseObject* pcObject)
{
	mpcGraphSerialiser = pcGraphSerialiser;
	CObjectSerialiser::Init(pcObject);
}


void CDependentObjectSerialiser::Kill(void)
{
	CObjectSerialiser::Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CDependentObjectSerialiser::WritePointer(CPointerObject pObject)
{
	CBaseObject*	pcBaseObject;

	pcBaseObject = &pObject;
	return WriteDependent(pcBaseObject);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CDependentObjectSerialiser::WriteDependent(CBaseObject* pcBaseObject)
{
	BOOL		bResult;

	bResult = WritePointerHeader(pcBaseObject);
	if ((pcBaseObject) && (bResult))
	{
		mpcGraphSerialiser->AddDependent(pcBaseObject);
	}
	return bResult;
}

