#ifndef __PREPROCESSOR_DIRECTIVE_H__
#define __PREPROCESSOR_DIRECTIVE_H__
#include "PPDirective.h"
#include "BaseLib/Chars.h"


class CDirectiveMap;
class CPreprocessorDirective
{
protected:
	EPreprocessorDirective	meType;
	CChars					mszName;
	int64					muiID;
	CDirectiveMap*			mpcDirectiveMap;

public:
	void					Init(CExternalString* pcName, EPreprocessorDirective eType, int64 lliID, CDirectiveMap* pcDirectiveMap);
	void					Kill(void);

	char*					GetName(void);
	int						GetNameLength(void);
	EPreprocessorDirective	GetType(void);

	void					Dump(void);
};


#endif // !__PREPROCESSOR_DIRECTIVE_H__

