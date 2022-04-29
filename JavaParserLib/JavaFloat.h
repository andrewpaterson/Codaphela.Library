#ifndef __JAVA_FLOAT_H__
#define __JAVA_FLOAT_H__
#include "BaseLib/Chars.h"
#include "BaseLib/PrimitiveTypes.h"
#include "JavaLiteral.h"


enum EJavaFloatType
{
	JFT_float32,
	JFT_float64,
	 
	JFT_Unknown = -1
};


class CJavaFloat : public CJavaLiteral
{
CONSTRUCTABLE(CJavaFloat);
protected:
	float64			mfValue;
	EJavaFloatType	meType;

public:
	void 	Init(float32 fValue);
	void 	Init(float64 fValue);
	void 	Kill(void);

	char*	GetType(void);

	void	Print(CChars* pszDest);
	BOOL	IsFloat(void);
};


#endif // !__JAVA_FLOAT_H__

