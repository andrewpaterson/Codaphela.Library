#include "Objects.h"
#include "BaseObject.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
char* CEmbeddedObject::PrintObject(CChars* psz)
{
	return PrintObject(psz, IsEmbedded());
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
char* CEmbeddedObject::PrintObject(CChars* psz, bool bEmbedded, bool bPointerMemory)
{
	int		iDistToRoot;

	if (bPointerMemory)
	{
		psz->Append(PointerToString(this));
		psz->Append(" ");
	}
	psz->Append("[");
	iDistToRoot = GetDistToRoot();
	if (iDistToRoot >= 0 && iDistToRoot <= 9)
	{
		psz->Append(" ");
	}
	psz->Append(iDistToRoot);
	if (!IsAllocatedInObjects())
	{
		psz->Append(",0");
	}
	psz->Append("]:");

	if (bEmbedded)
	{
		psz->Append("(");
	}
	psz->Append(ClassName());
	psz->Append("(");
	psz->Append(ClassSize());
	psz->Append(") Index:");
	psz->Append(GetIndex());
	if (IsNamed())
	{
		psz->Append(" Name:");
		psz->Append(GetName());
	}
	psz->Append(" Froms:");
	psz->Append(CEmbeddedObject::NumHeapFroms());
	psz->Append(",");
	psz->Append(CEmbeddedObject::NumStackFroms());
	if (bEmbedded)
	{
		psz->Append(")");
	}
	return psz->Text();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CEmbeddedObject::DumpHeapFroms(void)
{
	size			uiNumElements;
	size			i;
	CBaseObject*	pcFromObject;
	CChars			sz;

	sz.Init();
	uiNumElements = CEmbeddedObject::NumHeapFroms();
	for (i = 0; i < uiNumElements; i++)
	{
		pcFromObject = CEmbeddedObject::GetHeapFrom(i);
		sz.Append(i);
		sz.Append(": ");
		pcFromObject->PrintState(&sz);
		sz.AppendNewLine();
	}
	sz.DumpKill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
char* ObjectToString(CEmbeddedObject* pcObject)
{
	return ObjectToString(pcObject, (pcObject != NULL) && pcObject->IsEmbedded());
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
char* ObjectToString(CEmbeddedObject* pcObject, bool bEmbedded)
{
	CChars	szChars;
	size	uiCount;
	char*	sz;

	szChars.Init();
	if (pcObject)
	{
		pcObject->PrintObject(&szChars, bEmbedded);
	}
	else
	{
		szChars.Append("NULL");
	}

	uiCount = IncrementLogToStringCount();
	sz = gaszLogToStringScratchPad[uiCount];
	StrCpySafe(sz, szChars.Text(), LOG_TO_STRING_MAX_LENGTH);
	szChars.Kill();
	return sz;
}

