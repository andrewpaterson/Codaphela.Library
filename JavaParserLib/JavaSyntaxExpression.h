#ifndef __JAVA_SYNTAX_EXPRESSION_H__
#define __JAVA_SYNTAX_EXPRESSION_H__
#include "JavaSyntax.h"


class CJavaSyntaxExpression : public CJavaSyntax
{
CONSTRUCTABLE(CJavaSyntaxExpression);
protected:

public:
	void 	Init(CJavaSyntaxTree* pcTree, CJavaSyntax* pcParent);
	void 	Kill(void);

	char* GetType(void) override;
	void	Print(CChars* pszDest, int iDepth);

	BOOL	IsExpression(void) override;
};


#endif // !__JAVA_SYNTAX_EXPRESSION_H__

