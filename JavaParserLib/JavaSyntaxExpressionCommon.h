#ifndef __JAVA_SYNTAX_EXPRESSION_COMMON_H__
#define __JAVA_SYNTAX_EXPRESSION_COMMON_H__
#include "JavaSyntax.h"

// (ExpressionCommon); x = (ExpressionCommon)

class CJavaSyntaxExpressionCommon : public CJavaSyntax
{
CONSTRUCTABLE(CJavaSyntaxExpressionCommon);
protected:

public:
	void 	Init(CJavaSyntaxTree* pcTree, CJavaSyntax* pcParent);
	void 	Kill(void);

	char*	GetType(void) override;
	void	Print(CChars* pszDest, int iDepth);

	BOOL	IsExpressionCommon(void) override;
};


#endif // !__JAVA_SYNTAX_EXPRESSION_COMMON_H__

