#include "BaseLib/FileUtil.h"
#include "FontImportParameters.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CFontImportParams::Init(char* szImageFileName, char* szCharacterFileName, int iCharWidth, int iCharHeight)
{
	CFileUtil	cFileUtil;
	bool		bExists1;
	bool		bExists2;

	mszImageFileName.Init(szImageFileName);
	mszCharacterFileName.Init(szCharacterFileName);

	msCharsGrid.Init(-1, -1);
	msCharSize.Init(iCharWidth, iCharHeight);
	miSpaceWidth = iCharWidth;
	msTopLeftOffset.Init(0, 0);
	msInterspace.Init(0, 0);
	mbLeftToRightFirst = true;
	miDescent = 0;
	miAscent = iCharHeight;

	mszFontName.Init(mszImageFileName);
	cFileUtil.RemovePath(&mszFontName);
	cFileUtil.RemoveExtension(&mszFontName);

	bExists1 = cFileUtil.Exists(mszImageFileName.Text());
	bExists2 = cFileUtil.Exists(mszCharacterFileName.Text());
	return (bExists1 && bExists2);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CFontImportParams::Kill(void)
{
	mszCharacterFileName.Kill();
	mszImageFileName.Kill();
	mszFontName.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
char* CFontImportParams::FontName(void)
{
	return mszFontName.Text();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
char* CFontImportParams::GetImageFileName(void)
{
	return mszImageFileName.Text();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
char* CFontImportParams::GetCharacterFileName(void)
{
	return mszCharacterFileName.Text();
}

