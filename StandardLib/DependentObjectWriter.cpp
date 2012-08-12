#include "ObjectGraphWriter.h"
#include "PointerObject.h"
#include "DependentObjectWriter.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDependentObjectWriter::Init(CObjectGraphWriter* pcGraphWriter, CBaseObject* pcObject)
{
	mpcGraphWriter = pcGraphWriter;
	CObjectWriter::Init(pcObject);
}


void CDependentObjectWriter::Kill(void)
{
	CObjectWriter::Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CDependentObjectWriter::WritePointer(CPointerObject pObject)
{
	CBaseObject*	pcBaseObject;

	pcBaseObject = &pObject;
	return WriteDependent(pcBaseObject);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CDependentObjectWriter::WriteDependent(CBaseObject* pcObject)
{
	BOOL		bResult;

	bResult = PrivateWritePointer(pcObject);
	if ((pcObject) && (bResult))
	{
		mpcGraphWriter->AddDependent(pcObject);
	}
	return bResult;
}
