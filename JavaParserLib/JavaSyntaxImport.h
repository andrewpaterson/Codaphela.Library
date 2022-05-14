#ifndef __JAVA_SYNTAX_IMPORT_H__
#define __JAVA_SYNTAX_IMPORT_H__
#include "BaseLib/ArrayTemplatePtr.h"
#include "JavaSyntax.h"
#include "JavaTokenIdentifier.h"


class CJavaSyntaxImport : public CJavaSyntax
{
CONSTRUCTABLE(CJavaSyntaxImport);
protected:
	CJavaTokenIdentifierPtrArray		mapcIdentifiers;
	BOOL						mbStatic;
	BOOL						mbWild;

public:
	void 	Init(CJavaSyntaxTree* pcTree);
	void 	Kill(void);

	char*	GetType(void) override;
	void	AddIdentifier(CJavaTokenIdentifier* pcIdentifier);
	void	SetStatic(BOOL bStatic);
	void	SetWild(BOOL bWild);

	BOOL	IsImport(void) override;
};


typedef CArrayTemplatePtr<CJavaSyntaxImport>	CJavaSyntaxImportPtrArray;


#endif // !__JAVA_SYNTAX_IMPORT_H__

