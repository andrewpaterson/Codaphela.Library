#ifndef __JAVA_SYNTAX_LITERAL_H__
#define __JAVA_SYNTAX_LITERAL_H__
#include "JavaTokenLiteral.h"
#include "JavaTokenCharacter.h"
#include "JavaTokenBoolean.h"
#include "JavaTokenString.h"
#include "JavaTokenInteger.h"
#include "JavaTokenFloat.h"
#include "JavaTokenNull.h"
#include "JavaSyntaxExpressionCommon.h"


class CJavaSyntaxLiteral : public CJavaSyntaxExpressionCommon
{
CONSTRUCTABLE(CJavaSyntaxLiteral);
protected:
	CJavaTokenLiteral*	mpcLiteral;

public:
	void 					Init(CJavaSyntaxTree* pcTree, CJavaSyntax* pcParent);
	void 					Kill(void);

	char*					GetType(void) override;
	void					TypePrint(CChars* pszDest, int iDepth) override;
	void					PrettyPrint(CChars* pszDest, int iBlockDepth = 0) override;

	void					SetLiteral(CJavaTokenLiteral* pcLiteral);

	BOOL					IsLiteral(void) override;
	BOOL					IsCharacter(void);
	BOOL					IsBoolean(void);
	BOOL					IsString(void);
	BOOL					IsInteger(void);
	BOOL					IsFloat(void);
	BOOL					IsNull(void);

	CJavaTokenCharacter*	GetCharacter(void);
	CJavaTokenBoolean*		GetBoolean(void);
	CJavaTokenString*		GetString(void);
	CJavaTokenInteger*		GetInteger(void);
	CJavaTokenFloat*		GetFloat(void);
	CJavaTokenNull*			GetNull(void);

	BOOL					IsCompoundStatement(void) override;
};


#endif // !__JAVA_SYNTAX_LITERAL_H__

