#ifndef __JAVA_SYNTAX_H__
#define __JAVA_SYNTAX_H__
#include "BaseLib/Chars.h"
#include "JavaTokenIdentifier.h"


class CJavaSyntaxTree;
class CJavaSyntax
{
CONSTRUCTABLE(CJavaSyntax);
protected:
	CJavaSyntaxTree*	mpcTree;
	CJavaSyntax*		mpcParent;

public:
	virtual void		Init(CJavaSyntaxTree* pcTree, CJavaSyntax* pcParent);
	virtual void		Kill(void);
			void		Clear(void);

	virtual char*		GetType(void) =0;
	virtual void		TypePrint(CChars* pszDest, int iDepth);
	virtual void		PrettyPrint(CChars* pszDest, int iBlockDepth) =0;
			void		Dump(void);

	virtual bool		IsFile(void);
	virtual bool		IsImport(void);
	virtual bool		IsPackage(void);
	virtual bool		IsEnum(void);
	virtual bool		IsInterface(void);
	virtual bool		IsClass(void);
	virtual bool		IsType(void);
	virtual bool		IsGeneric(void);
	virtual bool		IsExtent(void);

	virtual bool		IsClassBlock(void);
	virtual bool		IsOriginalFor(void);
	virtual bool		IsIterativeFor(void);
	virtual bool		IsVariableDeclaration(void);
	virtual bool		IsVariableInitialiser(void);
	virtual bool		IsVoidExpression(void);
	virtual bool		IsValueExpression(void);
	virtual bool		IsArrayValueExpression(void);
	virtual bool		IsMethodCall(void);
	virtual bool		IsParentheses(void);
	virtual bool		IsOperator(void);

	virtual bool		IsStatement(void);
	virtual bool		IsClassCommon(void);
	virtual bool		IsTypeCommon(void);
	virtual bool		IsVariableCommon(void);
	virtual bool		IsExpressionCommon(void);
	virtual bool		IsLiteral(void);

	virtual bool		IsError(void);
	virtual bool		IsMismatch(void);

protected:
	void	PrintTokenArray(CChars* pszDest, CIdentifierArray* papcTokens);
};


#endif // !__JAVA_SYNTAX_H__

