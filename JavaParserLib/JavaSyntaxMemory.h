#ifndef __JAVA_SYNTAX_MEMORY_H__
#define __JAVA_SYNTAX_MEMORY_H__
#include "BaseLib/Constructable.h"
#include "BaseLib/MemoryStackExtended.h"
#include "BaseLib/ArrayTemplatePtr.h"
#include "JavaSyntaxFile.h"

typedef CArrayTemplatePtr<CJavaSyntaxFile>	CSyntaxPtrArray;


class CJavaSyntaxMemory
{
CONSTRUCTABLE(CJavaSyntaxMemory);
protected:
	CMemoryStackExtended	mcStack;
	CSyntaxPtrArray			mapcSyntaxs;

public:
	void				Init(void);
	void				Kill(void);

	CJavaSyntaxFile*	CreateFile(CJavaSyntaxTree* pcTree);

};


#endif // !__JAVA_SYNTAX_MEMORY_H__

