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
#include "JavaSyntaxType.h"
#include "JavaSyntaxExtent.h"
#include "JavaSyntaxGeneric.h"
#include "JavaSyntaxBlock.h"


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

	CJavaSyntaxFile*			CreateFile(CJavaSyntaxTree* pcTree, CJavaSyntax* pcParent);
	CJavaSyntaxPackage*			CreatePackage(CJavaSyntaxTree* pcTree, CJavaSyntax* pcParent);
	CJavaSyntaxImport*			CreateImport(CJavaSyntaxTree* pcTree, CJavaSyntax* pcParent);
	CJavaSyntaxClass*			CreateClass(CJavaSyntaxTree* pcTree, CJavaSyntax* pcParent);
	CJavaSyntaxEnum*			CreateEnum(CJavaSyntaxTree* pcTree, CJavaSyntax* pcParent);
	CJavaSyntaxInterface*		CreateInterface(CJavaSyntaxTree* pcTree, CJavaSyntax* pcParent);
	CJavaSyntaxType*			CreateType(CJavaSyntaxTree* pcTree, CJavaSyntax* pcParent);
	CJavaSyntaxExtent*			CreateExtent(CJavaSyntaxTree* pcTree, CJavaSyntax* pcParent);
	CJavaSyntaxGeneric*			CreateGeneric(CJavaSyntaxTree* pcTree, CJavaSyntax* pcParent);
	CJavaSyntaxBlock*			CreateBlock(CJavaSyntaxTree* pcTree, CJavaSyntax* pcParent);

protected:
	template<class M>	M*	Create(CJavaSyntaxTree* pcTree, CJavaSyntax* pcParent);
};


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class M>	
M* CJavaSyntaxMemory::Create(CJavaSyntaxTree* pcTree, CJavaSyntax* pcParent)
{
	M* pcSyntax;

	pcSyntax = (M*)mcStack.Add(sizeof(M));
	if (pcSyntax)
	{
		mapcSyntaxes.Add(pcSyntax);

		new(pcSyntax) M;
		pcSyntax->Init(pcTree, pcParent);
	}

	return pcSyntax;
}

#endif // !__JAVA_SYNTAX_MEMORY_H__

