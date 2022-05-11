#ifndef __JAVA_SYNTAX_IMPORT_H__
#define __JAVA_SYNTAX_IMPORT_H__
#include "BaseLib/ArrayTemplatePtr.h"
#include "JavaSyntax.h"


class CJavaSyntaxImport : public CJavaSyntax
{
CONSTRUCTABLE(CJavaSyntaxImport);
protected:
public:
	void 	Init(CJavaSyntaxTree* pcTree);
	void 	Kill(void);

	char*	GetType(void) override;

	BOOL	IsImport(void) override;
};


typedef CArrayTemplatePtr<CJavaSyntaxImport>	CJavaSyntaxImportPtrArray;


#endif // !__JAVA_SYNTAX_IMPORT_H__

