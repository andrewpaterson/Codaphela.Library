#ifndef __JAVA_SYNTAX_IMPORT_H__
#define __JAVA_SYNTAX_IMPORT_H__
#include "BaseLib/ArrayTemplateEmbeddedPtr.h"
#include "JavaSyntax.h"
#include "JavaTokenIdentifier.h"


class CJavaSyntaxImport : public CJavaSyntax
{
CONSTRUCTABLE(CJavaSyntaxImport);
protected:
	CJavaTokenIdentifierPtrEmbeddedArray	mapcIdentifiers;
	BOOL									mbStatic;
	BOOL									mbWild;

public:
	void 	Init(CJavaSyntaxTree* pcTree, CJavaSyntax* pcParent);
	void 	Kill(void);

	char*	GetType(void) override;
	void	Print(CChars* pszDest, int iDepth);

	void	AddIdentifier(CJavaTokenIdentifier* pcIdentifier);
	void	SetStatic(BOOL bStatic);
	void	SetWild(BOOL bWild);

	BOOL	IsImport(void) override;
};


typedef CArrayTemplatePtr<CJavaSyntaxImport>	CJavaSyntaxImportPtrArray;


#endif // !__JAVA_SYNTAX_IMPORT_H__

