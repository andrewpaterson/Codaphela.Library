#ifndef __PP_DEFINE_H__
#define __PP_DEFINE_H__
#include "BaseLib/ArrayTemplate.h"
#include "BaseLib/ArrayChars.h"
#include "BaseLib/ExternalString.h"
#include "PPLine.h"


#define DEFINE_FLAGS_BRACKETED	0x01
#define DEFINE_FLAGS_SPECIAL	0x02
#define DEFINE_FLAGS_IN_MAP		0x04
#define DEFINE_FLAGS_VARIADIC	0x08
#define DEFINE_FLAGS_KILLED		0x80


class CDefineMap;
class CDefine
{
protected:
	CPPLine			mcReplacement;
	CArrayChars		maszArguments;
	int64			mlliID;			//If a token is undef'd and redef'd it's muiID will change.
	CDefineMap*		mpcDefineMap;
	int				miFlags;
	CChars			mszName;

public:
	void 			Init(CExternalString* pcName, int64 uiID, CDefineMap* pcDefineMap);
	void 			Kill(void);
	void 			AddArgument(CExternalString* pcName);
	void			SetVariadic(void);
	void 			AddReplacmentToken(CPPToken* pcToken);
	bool 			Equals(CDefine* pcOther);
	bool			CanProcessArguments(int iNumArguments);
	char*			GetName(void);
	int64			GetID(void);
	bool			IsBacketed(void);
	bool			IsVariadic(void);
	bool			IsSpecial(void);
	bool			IsInMap(void);
	void			SetSpecial(bool b);
	void			SetBracketed(bool b);
	void			SetDefineMap(CDefineMap* pcDefineMap);
	CArrayChars*	GetArguments(void);
	CPPLine*		GetReplacement(void);
	void			Dump(void);
};


#endif // !__PP_DEFINE_H__

