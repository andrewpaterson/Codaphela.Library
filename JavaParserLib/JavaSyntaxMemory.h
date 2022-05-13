#ifndef __JAVA_SYNTAX_MEMORY_H__
#define __JAVA_SYNTAX_MEMORY_H__
#include "BaseLib/Constructable.h"
#include "BaseLib/MemoryStackExtended.h"
#include "BaseLib/ArrayTemplatePtr.h"
#include "JavaSyntaxFile.h"
#include "JavaSyntaxPackage.h"
#include "JavaSyntaxImport.h"
#include "JavaSyntaxClass.h"
#include "JavaSyntaxEnum.h"
#include "JavaSyntaxInterface.h"
#include "JavaSyntaxClassGeneric.h"
#include "JavaSyntaxType.h"
#include "JavaSyntaxTypeGeneric.h"


typedef CArrayTemplatePtr<CJavaSyntax>	CSyntaxPtrArray;


class CJavaSyntaxMemory
{
CONSTRUCTABLE(CJavaSyntaxMemory);
protected:
	CMemoryStackExtended	mcStack;
	CSyntaxPtrArray			mapcSyntaxes;

public:
	void						Init(void);
	void						Kill(void);

	CJavaSyntaxFile*			CreateFile(CJavaSyntaxTree* pcTree);
	CJavaSyntaxPackage*			CreatePackage(CJavaSyntaxTree* pcTree);
	CJavaSyntaxImport*			CreateImport(CJavaSyntaxTree* pcTree);
	CJavaSyntaxClass*			CreateClass(CJavaSyntaxTree* pcTree);
	CJavaSyntaxEnum*			CreateEnum(CJavaSyntaxTree* pcTree);
	CJavaSyntaxInterface*		CreateInterface(CJavaSyntaxTree* pcTree);
	CJavaSyntaxClassGeneric*	CreateClassGeneric(CJavaSyntaxTree* pcTree);
	CJavaSyntaxType*			CreateType(CJavaSyntaxTree* pcTree);
	CJavaSyntaxTypeGeneric*		CreateTypeGeneric(CJavaSyntaxTree* pcTree);

protected:
	template<class M>	M*	Create(CJavaSyntaxTree* pcTree);
};


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class M>	
M* CJavaSyntaxMemory::Create(CJavaSyntaxTree* pcTree)
{
	M* pcSyntax;

	pcSyntax = (M*)mcStack.Add(sizeof(M));
	if (pcSyntax)
	{
		mapcSyntaxes.Add(pcSyntax);

		new(pcSyntax) M;
		pcSyntax->Init(pcTree);
	}

	return pcSyntax;
}

#endif // !__JAVA_SYNTAX_MEMORY_H__

