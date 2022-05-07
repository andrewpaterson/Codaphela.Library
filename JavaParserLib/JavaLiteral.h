#ifndef __JAVA_LITERAL_H__
#define __JAVA_LITERAL_H__
#include "BaseLib/Chars.h"
#include "JavaToken.h"


class CJavaLiteral : public CJavaToken
{
CONSTRUCTABLE(CJavaLiteral);
public:
			void 	Init(void);
			void 	Kill(void);

			char*	GetType(void) override;

			BOOL	IsLiteral(void) override;
	virtual BOOL	IsCharacter(void);
	virtual BOOL	IsBoolean(void);
	virtual BOOL	IsString(void);
	virtual BOOL	IsInteger(void);
	virtual BOOL	IsFloat(void);
	virtual BOOL	IsNull(void);
};


#endif // !__JAVA_LITERAL_H__

