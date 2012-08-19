#include "ObjectGraphSerialiser.h"
#include "PointerObject.h"
#include "DependentObjectSerialiser.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDependentObjectSerialiser::Init(CObjectGraphSerialiser* pcGraphWriter, CBaseObject* pcObject)
{
	mpcGraphWriter = pcGraphWriter;
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
BOOL CDependentObjectSerialiser::WriteDependent(CBaseObject* pcObject)
{
	BOOL		bResult;

	bResult = PrivateWritePointer(pcObject);
	if ((pcObject) && (bResult))
	{
		mpcGraphWriter->AddDependent(pcObject);
	}
	return bResult;
}
