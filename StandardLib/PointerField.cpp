#include "Class.h"
#include "Classes.h"
#include "PointerField.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CPointerField::Init(ptrdiff_t iOffset, CClass* pcContainingClass, char* szName)
{
	CClasses*	pcClasses;

	pcClasses = pcContainingClass->GetClasses();
	if (iOffset >= 0)
	{
		CField::Init(pcClasses->GetPrimitiveClasses()->GetPointer(), iOffset, pcContainingClass, szName);
	}
	else
	{
		CField::Init(NULL, 0, pcContainingClass, szName);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CPointerField::Kill(void)
{
	CField::Kill();
}

