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
	void	TypePrint(CChars* pszDest, int iDepth) override;
	void	PrettyPrint(CChars* pszDest, int iBlockDepth) override;

	bool	IsEnum(void) override;
	bool	IsCompoundStatement(void);
};


typedef CArrayTemplateEmbeddedPtr<CJavaSyntaxEnum, 6>	CEnumArray;


#endif // !__JAVA_SYNTAX_ENUM_H__

