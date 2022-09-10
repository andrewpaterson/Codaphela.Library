#ifndef __JAVA_SYNTAX_EXTENT_H__
#define __JAVA_SYNTAX_EXTENT_H__
#include "JavaSyntaxTypeCommon.h"


class CJavaSyntaxType;
class CJavaSyntaxExtent : public CJavaSyntaxTypeCommon
{
CONSTRUCTABLE(CJavaSyntaxExtent);
protected:
	bool				mbWildCard;
	CJavaSyntaxType*	mpcExtends;

public:
	void 	Init(CJavaSyntaxTree* pcTree, CJavaSyntax* pcParent);
	void 	Kill(void);

	char*	GetType(void) override;
	void	TypePrint(CChars* pszDest, int iDepth) override;
	void	PrettyPrint(CChars* pszDest, int iBlockDepth = 0) override;

	bool	IsExtent(void) override;
	void	SetWildCard(bool bWildCard);
	void	SetExtends(CJavaSyntaxType* pcExtends);
	bool	IsWildCard(void);
};


#endif // !__JAVA_SYNTAX_EXTENT_H__

