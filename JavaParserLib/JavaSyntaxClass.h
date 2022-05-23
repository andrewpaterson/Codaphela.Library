#ifndef __JAVA_SYNTAX_CLASS_H__
#define __JAVA_SYNTAX_CLASS_H__
#include "BaseLib/ArrayTemplatePtr.h"
#include "JavaSyntaxType.h"
#include "JavaSyntaxClassCommon.h"


class CJavaSyntaxClass : public CJavaSyntaxClassCommon
{
CONSTRUCTABLE(CJavaSyntaxClass);
protected:
	CJavaSyntaxType*	mpcType;

public:
	void 	Init(CJavaSyntaxTree* pcTree, CJavaSyntax* pcParent);
	void 	Kill(void);

	char*	GetType(void) override;
	void	Print(CChars* pszDest, int iDepth);

	BOOL	IsClass(void) override;

	void	SetSyntaxType(CJavaSyntaxType* pcType);
};


#endif // !__JAVA_SYNTAX_CLASS_H__

