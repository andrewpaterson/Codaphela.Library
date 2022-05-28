#ifndef __JAVA_SYNTAX_TYPE_H__
#define __JAVA_SYNTAX_TYPE_H__
#include "JavaSyntaxTypeCommon.h"


class CJavaSyntaxGeneric;
class CJavaSyntaxType : public CJavaSyntaxTypeCommon
{
CONSTRUCTABLE(CJavaSyntaxType);
protected:
	CJavaSyntaxGeneric*		mpcGeneric;

public:
	void 	Init(CJavaSyntaxTree* pcTree, CJavaSyntax* pcParent);
	void 	Kill(void);

	char*	GetType(void) override;
	void	TypePrint(CChars* pszDest, int iDepth) override;
	char*	PrettyPrint(CChars* pszDest) override;

	BOOL	IsType(void) override;
	void	SetGeneric(CJavaSyntaxGeneric* pcGeneric);
};


typedef CArrayTemplatePtr<CJavaSyntaxType>	CJavaSyntaxTypePtrArray;


#endif // !__JAVA_SYNTAX_TYPE_H__

