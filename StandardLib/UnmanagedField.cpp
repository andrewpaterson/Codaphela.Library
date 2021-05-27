#include "Class.h"
#include "Classes.h"
#include "UnmanagedField.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CUnmanagedField::Init(EPrimitiveType eType, ptrdiff_t iOffset, CClass* pcContainingClass, char* szName)
{
	CClasses*	pcClasses;
	CClass*		pcFieldClass;

	pcClasses = pcContainingClass->GetClasses();
	pcFieldClass = pcClasses->Get(eType);
	CField::Init(pcFieldClass, iOffset, pcContainingClass, szName);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CUnmanagedField::Kill(void)
{
	CField::Kill();
}

