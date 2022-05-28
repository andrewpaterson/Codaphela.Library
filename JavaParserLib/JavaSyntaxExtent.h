#ifndef __JAVA_SYNTAX_EXTENT_H__
#define __JAVA_SYNTAX_EXTENT_H__
#include "JavaSyntaxTypeCommon.h"


class CJavaSyntaxType;
class CJavaSyntaxExtent : public CJavaSyntaxTypeCommon
{
CONSTRUCTABLE(CJavaSyntaxExtent);
protected:
	BOOL				mbWildCard;
	CJavaSyntaxType*	mpcExtends;

public:
	void 	Init(CJavaSyntaxTree* pcTree, CJavaSyntax* pcParent);
	void 	Kill(void);

	char*	GetType(void) override;
	void	TypePrint(CChars* pszDest, int iDepth) override;

	BOOL	IsExtent(void) override;
	void	SetWildCard(BOOL bWildCard);
	void	SetExtends(CJavaSyntaxType* pcExtends);
};


#endif // !__JAVA_SYNTAX_EXTENT_H__

