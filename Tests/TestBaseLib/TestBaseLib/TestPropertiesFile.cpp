#include "BaseLib/FileUtil.h"
#include "BaseLib/DiskFile.h"
#include "BaseLib/PropertiesFile.h"
#include "TestLib/Assert.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestPropertiesFileWrite(void)
{
	CPropertiesFile		cFile;
	CFileUtil			cFileUtil;
	CTextFile			cText;
	CFileBasic			cBasic;
	char*				szText;

	cFileUtil.MakeDir("Output");
	cFileUtil.Delete("Output/Properties.txt");

	cFile.Init("Output/Properties.txt");
	cFile.Set("BELL", "Phone");
	cFile.Set("Studio", "IO");
	cFile.Set("Yogurt", "");
	cFile.Set("Calypso", "");
	cFile.Write();
	cFile.Kill();

	cText.Init();
	cText.Read(DiskFile("Output/Properties.txt"));
	szText = cText.Text();
	AssertString("BELL = Phone\nCalypso\nStudio = IO\nYogurt\n", szText);
	cText.Kill();

	cFile.Init("Output/Properties.txt");
	cFile.Read();
	AssertString("Phone", cFile.Get("BELL"));
	AssertString("IO", cFile.Get("Studio"));
	AssertString("", cFile.Get("Yogurt"));
	AssertString("", cFile.Get("Calypso"));
	AssertNull(cFile.Get("Camps"));

	cFile.Kill();

	cFileUtil.Delete("Output/Properties.txt");
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestPropertiesFileRead(void)
{
	CPropertiesFile		cFile;
	CFileUtil			cFileUtil;
	CTextFile			cText;
	CFileBasic			cBasic;

	cFileUtil.MakeDir("Output");
	cFileUtil.Delete("Output/Properties.txt");

	cText.Init("\nAge = Chaos\n  Nothing\nToday=Yodel\n\n");
	cText.Write("Output/Properties.txt");
	cText.Kill();

	cFile.Init("Output/Properties.txt");
	cFile.Read();
	AssertSize(3, cFile.NumProperties());
	AssertString("Chaos", cFile.Get("Age"));
	AssertString("Yodel", cFile.Get("Today"));
	AssertString("", cFile.Get("Nothing"));
	AssertNull(cFile.Get(""));
	AssertNull(cFile.Get("Calypso"));

	cFile.Kill();

	cFileUtil.Delete("Output/Properties.txt");
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestPropertiesFile(void)
{
	BeginTests();
		
	TestPropertiesFileRead();
	TestPropertiesFileWrite();

	TestStatistics();
}

