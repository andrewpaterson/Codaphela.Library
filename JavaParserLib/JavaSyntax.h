#ifndef __JAVA_SYNTAX_H__
#define __JAVA_SYNTAX_H__
#include "BaseLib/Chars.h"


class CJavaSyntaxTree;
class CJavaSyntax
{
CONSTRUCTABLE(CJavaSyntax);
protected:
	CJavaSyntaxTree*	mpcTree;

public:
			void		Init(CJavaSyntaxTree* pcTree);
	virtual void		Kill(void);

	virtual char*		GetType(void) =0;

	virtual BOOL		IsFile(void);
	virtual BOOL		IsImport(void);
	virtual BOOL		IsPackage(void);
	virtual BOOL		IsTopLevel(void);
};


#endif // !__JAVA_SYNTAX_H__

