#include "TestLib/Assert.h"
#include "TestReadImage.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
Ptr<CImage> TestReadImage(char* szDirectory, char* szFilename)
{
	CFileUtil			cFileUtil;
	CChars				szInputFilename;
	Ptr<CImage>			pImage;

	szInputFilename.Init();
	cFileUtil.CurrentDirectory(&szInputFilename);
	cFileUtil.AppendToPath(&szInputFilename, "Input");
	if (!StrEmpty(szDirectory))
	{
		cFileUtil.AppendToPath(&szInputFilename, szDirectory);
	}
	cFileUtil.AppendToPath(&szInputFilename, szFilename);
	AssertTrue(cFileUtil.Exists(szInputFilename.Text()));

	pImage = ReadImage(szInputFilename.Text(), IT_Unknown, true);
	AssertTrue(pImage.IsNotNull());
	szInputFilename.Kill();

	return pImage;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
Ptr<CArrayImageCel> TestReadCels(char* szDirectory, char* szFilename, int iColumnCount, int iRowCount)
{
	Ptr<CImage>				pImage;
	CImageDivider			cImageDivider;
	CImageDividerNumbers	cNumbers;
	Ptr<CArrayImageCel>		pCels;

	pImage = TestReadImage(szDirectory, szFilename);

	cNumbers.InitGeneral(-1, -1, iColumnCount, iRowCount, 0, 0, 0, 0);
	cImageDivider.Init(pImage, NULL);
	cImageDivider.GenerateFromNumbers(&cNumbers);
	pCels = cImageDivider.GetDestImageCels();

	AssertTrue(pCels.IsNotNull());
	AssertSize(iColumnCount * iRowCount, pCels->NumElements());

	AssertSize(2, pCels.NumStackFroms());
	AssertSize(0, pCels.NumHeapFroms());

	cImageDivider.Kill();

	AssertSize(1, pCels.NumStackFroms());
	AssertSize(0, pCels.NumHeapFroms());

	return pCels;
}

