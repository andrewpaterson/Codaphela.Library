#ifndef __JAVA_SYNTAX_STATEMENT_H__
#define __JAVA_SYNTAX_STATEMENT_H__
#include "BaseLib/ArrayTemplateEmbeddedPtr.h"
#include "JavaSyntax.h"


//Everything in a Class- or Code Block is a statement.
class  CJavaSyntaxStatement : public CJavaSyntax
{
CONSTRUCTABLE(CJavaSyntaxStatement);
protected:
public:
	void 	Init(CJavaSyntaxTree* pcTree, CJavaSyntax* pcParent);
	void 	Kill(void);

	char*	GetType(void) override;
	void	Print(CChars* pszDest, int iDepth);

	BOOL	IsStatement(void) override;
};

typedef CArrayTemplateEmbeddedPtr<CJavaSyntaxStatement, 6> CJavaSyntaxStatementPtrEmbeddedArray;


#endif // !__JAVA_SYNTAX_STATEMENT_H__

