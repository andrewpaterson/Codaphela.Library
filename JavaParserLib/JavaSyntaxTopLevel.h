#ifndef __JAVA_SYNTAX_TOP_LEVEL_H__
#define __JAVA_SYNTAX_TOP_LEVEL_H__
#include "BaseLib/ArrayTemplatePtr.h"
#include "JavaSyntax.h"
#include "JavaTokenIdentifier.h"


class CJavaSyntaxTopLevel : public CJavaSyntax
{
CONSTRUCTABLE(CJavaSyntaxTopLevel);
protected:
	BOOL					mbPublic;

public:
	void 	Init(CJavaSyntaxTree* pcTree);
	void 	Kill(void);

	char*	GetType(void) override;

	BOOL	IsTopLevel(void) override;

	void	SetPublic(BOOL bPublic);
};


typedef CArrayTemplatePtr<CJavaSyntaxTopLevel>	CJavaSyntaxTopLevelPtrArray;


#endif // !__JAVA_SYNTAX_TOP_LEVEL_H__

