#ifndef __FONT_IMPORT_PARAMETERS_H__
#define __FONT_IMPORT_PARAMETERS_H__
#include "BaseLib/PrimitiveTypes.h"
#include "BaseLib/Chars.h"
#include "BaseLib/Int2.h"


class CFontImportParams
{
public:
	CChars	mszImageFileName;
	CChars	mszCharacterFileName;
	CChars	mszFontName;
	SInt2	msCharsGrid;
	SInt2	msCharSize;
	int		miSpaceWidth;
	SInt2	msTopLeftOffset;
	SInt2	msInterspace;
	int		miAscent;
	int		miDescent;
	bool	mbLeftToRightFirst;

public:
	bool	Init(char* szFileName, char* szCharArray, int iCharWidth, int iCharHeight);
	void	Kill(void);

	char*	FontName(void);
	char*	GetImageFileName(void);
	char*	GetCharacterFileName(void);
};


#endif // __FONT_IMPORT_PARAMETERS_H__

