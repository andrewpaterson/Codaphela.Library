#ifndef __JAVA_SYNTAX_MEMORY_H__
#define __JAVA_SYNTAX_MEMORY_H__
#include "BaseLib/Constructable.h"
#include "BaseLib/MemoryStackExtended.h"
#include "BaseLib/ArrayTemplatePtr.h"
#include "JavaSyntaxFile.h"
#include "JavaSyntaxPackage.h"

typedef CArrayTemplatePtr<CJavaSyntax>	CSyntaxPtrArray;


class CJavaSyntaxMemory
{
CONSTRUCTABLE(CJavaSyntaxMemory);
protected:
	CMemoryStackExtended	mcStack;
	CSyntaxPtrArray			mapcSyntaxes;

public:
	void				Init(void);
	void				Kill(void);

	CJavaSyntaxFile*	CreateFile(CJavaSyntaxTree* pcTree);
	CJavaSyntaxPackage*	CreatePackage(CJavaSyntaxTree* pcTree);

};


#endif // !__JAVA_SYNTAX_MEMORY_H__

