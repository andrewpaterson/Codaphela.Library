#ifndef __JAVA_SYNTAX_ENUM_H__
#define __JAVA_SYNTAX_ENUM_H__
#include "BaseLib/ArrayTemplatePtr.h"
#include "JavaSyntaxClassCommon.h"


class CJavaSyntaxEnum : public CJavaSyntaxClassCommon
{
CONSTRUCTABLE(CJavaSyntaxEnum);
protected:
public:
	void 	Init(CJavaSyntaxTree* pcTree, CJavaSyntax* pcParent);
	void 	Kill(void);

	char*	GetType(void) override;
	void	Print(CChars* pszDest, int iDepth);

	BOOL	IsEnum(void) override;
};


typedef CArrayTemplatePtr<CJavaSyntaxEnum>	CJavaSyntaxEnumPtrArray;


#endif // !__JAVA_SYNTAX_ENUM_H__

