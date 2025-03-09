#ifndef __JAVA_TOKEN_H__
#define __JAVA_TOKEN_H__
#include "BaseLib/Define.h"
#include "BaseLib/Constructable.h"
#include "BaseLib/Chars.h"
#include "BaseLib/TextPosition.h"


class CJavaToken
{
CONSTRUCTABLE(CJavaToken);
protected:
	CJavaToken*		mpcNext;
	STextPosition	msPos;

public:
			void			Init(STextPosition* psPosition);
	virtual void			Kill(void);

			void			SetNext(CJavaToken* pcNext);
	virtual void			Print(CChars* pszDest) =0;
	virtual char*			GetType(void) =0;
			CJavaToken*		GetNext(void);
			STextPosition*	GetPosition(void);

	virtual bool			IsSeparator(void);
	virtual bool			IsKeyword(void);
	virtual bool			IsIdentifier(void);
	virtual bool			IsScope(void);
	virtual bool			IsAmbiguous(void);
	virtual bool			IsOperator(void);
	virtual bool			IsLiteral(void);
	virtual bool			IsAnnotation(void);
};


typedef CArrayTemplatePtr<CJavaToken> CJavaTokenPtrArray;


#endif // !__JAVA_TOKEN_H__

