#ifndef __JAVA_MODIFIER_H__
#define __JAVA_MODIFIER_H__
#include "BaseLib/PrimitiveTypes.h"
#include "BaseLib/Chars.h"


enum EJavaModifier
{
	JM_none =		0x00,
	JM_public =		0x01,
	JM_protected =	0x02,
	JM_private =	0x04,
	JM_static =		0x08,
	JM_abstract =	0x10,
	JM_final =		0x20,
	JM_strictfp =	0x40,
};


class CJavaModifiers
{
protected:
	uint8	muiModifiers;

public:
	void Init(void);
	void Kill(void);

	void Set(EJavaModifier eModifier);

	BOOL IsNone(void);
	BOOL IsPublic(void);
	BOOL IsProtected(void);
	BOOL IsPrivate(void);
	BOOL IsStatic(void);
	BOOL IsAbstract(void);
	BOOL IsFinal(void);
	BOOL IsStrictfp(void);

	BOOL IsPackageModifier(void);
	BOOL IsValid(void);

	void Print(CChars* pszDest);
};


#endif // !__JAVA_MODIFIER_H__

