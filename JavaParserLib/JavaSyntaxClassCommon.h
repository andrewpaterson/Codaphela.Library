#ifndef __JAVA_SYNTAX_TOP_LEVEL_H__
#define __JAVA_SYNTAX_TOP_LEVEL_H__
#include "BaseLib/ArrayTemplatePtr.h"
#include "JavaSyntax.h"
#include "JavaTokenIdentifier.h"


class CJavaSyntaxClassCommon : public CJavaSyntax
{
CONSTRUCTABLE(CJavaSyntaxClassCommon);
protected:
	BOOL					mbPublic;

public:
	void 	Init(CJavaSyntaxTree* pcTree, CJavaSyntax* pcParent);
	void 	Kill(void);

	char*	GetType(void) override;

	BOOL	IsClassCommon(void) override;

	void	SetPublic(BOOL bPublic);
	BOOL	IsPublic(void);
};


typedef CArrayTemplatePtr<CJavaSyntaxClassCommon>	CJavaSyntaxClassCommonPtrArray;


#endif // !__JAVA_SYNTAX_TOP_LEVEL_H__

