#ifndef __FONT_IMPORT_PARAMETERS_H__
#define __FONT_IMPORT_PARAMETERS_H__
#include "BaseLib/PrimitiveTypes.h"
#include "BaseLib/Chars.h"
#include "BaseLib/Int32Vec2.h"


class CFontImportParameters
{
public:
	CChars		mszImageFileName;
	CChars		mszCharacterFileName;
	CChars		mszFontName;
	SInt32Vec2	msCharsGrid;		//How many rows and columns of characters in the image.
	SInt32Vec2	msCharSize;			//Size in pixels of each character.
	SInt32Vec2	msTopLeftOffset;	//Offset from which the char grid starts.
	SInt32Vec2	msInterspace;		//How much unused space between chars in the grid.

public:
	bool	Init(char* szFileName, char* szCharArray, int iCharWidth, int iCharHeight);
	void	Kill(void);

	char*	FontName(void);
	char*	GetImageFileName(void);
	char*	GetCharacterFileName(void);
};


#endif // __FONT_IMPORT_PARAMETERS_H__

