#ifndef __JAVA_SYNTAX_H__
#define __JAVA_SYNTAX_H__
#include "BaseLib/Chars.h"


class CJavaSyntaxTree;
class CJavaSyntax
{
CONSTRUCTABLE(CJavaSyntax);
protected:
	CJavaSyntaxTree*	mpcTree;
	CJavaSyntax*		mpcParent;

public:
			void		Init(CJavaSyntaxTree* pcTree, CJavaSyntax* pcParent);
	virtual void		Kill(void);

	virtual char*		GetType(void) =0;
	virtual void		Print(CChars* pszDest, int iDepth);
			void		Dump(void);

	virtual BOOL		IsFile(void);
	virtual BOOL		IsImport(void);
	virtual BOOL		IsPackage(void);
	virtual BOOL		IsTopLevel(void);
	virtual BOOL		IsEnum(void);
	virtual BOOL		IsInterface(void);
	virtual BOOL		IsClass(void);
	virtual BOOL		IsType(void);
	virtual BOOL		IsGeneric(void);

	virtual BOOL		IsError(void);
	virtual BOOL		IsMismatch(void);
};


#endif // !__JAVA_SYNTAX_H__

