#ifndef __JAVA_TOKEN_H__
#define __JAVA_TOKEN_H__
#include "BaseLib/Define.h"
#include "BaseLib/Constructable.h"
#include "BaseLib/Chars.h"


class CJavaToken
{
CONSTRUCTABLE(CJavaToken);
protected:
	CJavaToken*		mpcNext;

public:
			void		Init(void);
	virtual void		Kill(void);

			void		SetNext(CJavaToken* pcNext);
	virtual void		Print(CChars* pszDest) =0;
	virtual char*		GetType(void) =0;
			CJavaToken* GetNext(void);

	virtual BOOL		IsSeparator(void);
	virtual BOOL		IsComment(void);
	virtual BOOL		IsKeyword(void);
	virtual BOOL		IsIdentifier(void);
	virtual BOOL		IsGeneric(void);
	virtual BOOL		IsAmbiguous(void);
	virtual BOOL		IsOperator(void);
	virtual BOOL		IsLiteral(void);
	virtual BOOL		IsAnnotation(void);
};


#endif // !__JAVA_TOKEN_H__

