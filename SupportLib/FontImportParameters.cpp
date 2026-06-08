#include "BaseLib/FileUtil.h"
#include "FontImportParameters.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CFontImportParameters::Init(char* szImageFileName, char* szCharacterFileName, int iCharWidth, int iCharHeight)
{
	CFileUtil	cFileUtil;
	bool		bExists1;
	bool		bExists2;

	mszImageFileName.Init(szImageFileName);
	mszCharacterFileName.Init(szCharacterFileName);

	msCharsGrid.Init(-1, -1);
	msCharSize.Init(iCharWidth, iCharHeight);
	msTopLeftOffset.Init(0, 0);
	msInterspace.Init(0, 0);

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
void CFontImportParameters::Kill(void)
{
	mszCharacterFileName.Kill();
	mszImageFileName.Kill();
	mszFontName.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
char* CFontImportParameters::FontName(void)
{
	return mszFontName.Text();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
char* CFontImportParameters::GetImageFileName(void)
{
	return mszImageFileName.Text();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
char* CFontImportParameters::GetCharacterFileName(void)
{
	return mszCharacterFileName.Text();
}

