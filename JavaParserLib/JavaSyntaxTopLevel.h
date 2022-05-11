#ifndef __JAVA_SYNTAX_TOP_LEVEL_H__
#define __JAVA_SYNTAX_TOP_LEVEL_H__
#include "BaseLib/ArrayTemplatePtr.h"
#include "JavaSyntax.h"


class CJavaSyntaxTopLevel : public CJavaSyntax
{
CONSTRUCTABLE(CJavaSyntaxTopLevel);
protected:
public:
	void 	Init(CJavaSyntaxTree* pcTree);
	void 	Kill(void);

	char*	GetType(void) override;

	BOOL	IsTopLevel(void) override;
};


typedef CArrayTemplatePtr<CJavaSyntaxTopLevel>	CJavaSyntaxTopLevelPtrArray;


#endif // !__JAVA_SYNTAX_TOP_LEVEL_H__

