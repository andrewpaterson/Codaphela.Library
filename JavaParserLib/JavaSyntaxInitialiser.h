#ifndef __JAVA_SYNTAX_INITIALISER_H__
#define __JAVA_SYNTAX_INITIALISER_H__
#include "JavaSyntax.h"
#include "JavaTokenLiteral.h"
#include "JavaSyntaxValueExpression.h"
#include "JavaSyntaxArrayValueExpression.h"


class CJavaSyntaxInitialiser : public CJavaSyntax
{
CONSTRUCTABLE(CJavaSyntaxInitialiser);
protected:
	CJavaSyntaxArrayValueExpression*	mpcArrayValue;
	CJavaSyntaxValueExpression*			mpValue;

public:
	void 	Init(CJavaSyntaxTree* pcTree, CJavaSyntax* pcParent);
	void 	Kill(void);

	char*	GetType(void) override;
	void	Print(CChars* pszDest, int iDepth);

	BOOL	IsInitialiser(void) override;
};


#endif // !__JAVA_SYNTAX_INITIALISER_H__

