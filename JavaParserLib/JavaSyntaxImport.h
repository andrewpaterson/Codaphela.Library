#ifndef __JAVA_SYNTAX_IMPORT_H__
#define __JAVA_SYNTAX_IMPORT_H__
#include "BaseLib/ArrayTemplateEmbeddedPtr.h"
#include "JavaSyntax.h"
#include "JavaTokenIdentifier.h"


class CJavaSyntaxImport : public CJavaSyntax
{
CONSTRUCTABLE(CJavaSyntaxImport);
protected:
	CIdentifierArray	mapcIdentifiers;
	bool									mbStatic;
	bool									mbWild;

public:
	void 	Init(CJavaSyntaxTree* pcTree, CJavaSyntax* pcParent);
	void 	Kill(void);

	char*	GetType(void) override;
	void	TypePrint(CChars* pszDest, int iDepth) override;
	void	PrettyPrint(CChars* pszDest, int iBlockDepth = 0);

	void	AddIdentifier(CJavaTokenIdentifier* pcIdentifier);
	void	SetStatic(bool bStatic);
	void	SetWild(bool bWild);

	bool	IsImport(void) override;
};


typedef CArrayTemplateEmbeddedPtr<CJavaSyntaxImport, 6>	CImportArray;


#endif // !__JAVA_SYNTAX_IMPORT_H__

