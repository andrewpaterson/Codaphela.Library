#ifndef __JAVA_SYNTAX_INITIALISER_H__
#define __JAVA_SYNTAX_INITIALISER_H__
#include "JavaSyntax.h"
#include "JavaTokenLiteral.h"
#include "JavaSyntaxValueExpression.h"
#include "JavaSyntaxArrayValueExpression.h"


class CJavaSyntaxVariableInitialiser : public CJavaSyntax
{
CONSTRUCTABLE(CJavaSyntaxVariableInitialiser);
protected:
	CJavaSyntaxValueExpression*			mpcSingleValueExpression;
	CJavaSyntaxArrayValueExpression*	mpcArrayValueExpression;

public:
	void 								Init(CJavaSyntaxTree* pcTree, CJavaSyntax* pcParent);
	void 								Kill(void);

	char*								GetType(void) override;
	void								TypePrint(CChars* pszDest, int iDepth) override;
	void								PrettyPrint(CChars* pszDest, int iBlockDepth = 0) override;

	bool								IsVariableInitialiser(void) override;
	bool								IsArrayExpression(void);
	bool								IsSingleExpression(void);

	void								SetArrayValueExpression(CJavaSyntaxArrayValueExpression* pcArrayValueExpression);
	void								SetSingleValueExpression(CJavaSyntaxValueExpression* pValueExpression);

	CJavaSyntaxArrayValueExpression*	GetArrayValueExpression(void);
	CJavaSyntaxValueExpression*			GetSingleValueExpression(void);

};


#endif // !__JAVA_SYNTAX_INITIALISER_H__

