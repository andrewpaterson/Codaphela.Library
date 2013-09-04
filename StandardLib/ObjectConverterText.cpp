#include "BaseLib/TextFile.h"
#include "Unknowns.h"
#include "Objects.h"
#include "String.h"
#include "ObjectSourceText.h"
#include "ObjectConverterText.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CObjectConverterText::Init(void)
{
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CObjectConverterText::Kill(void)
{
	CObjectConverter::Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
char* CObjectConverterText::GetFileExtension(void)
{
	return "TXT";
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CObjectConverterText::IsFor(CAbstractFile* pcFile)
{
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CObjectSource* CObjectConverterText::CreateSource(CAbstractFile* pcFile, char* szFileName)
{
	CObjectSourceText*	pcSource;

	if ((!pcFile) || (!szFileName))
	{
		return NULL;
	}

	pcSource = UMalloc(CObjectSourceText);
	pcSource->Init(this, pcFile, szFileName);
	return pcSource;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CBaseObject* CObjectConverterText::Convert(CObjectSource* pcSource, char* szObjectName)
{
	CTextFile			cTextFile;
	Ptr<CString>		pcString;
	BOOL				bResult;
	CObjectSourceText*	pcSourceText;

	pcSourceText = (CObjectSourceText*)pcSource;
	cTextFile.Init();

	bResult = cTextFile.Read(pcSource->GetFile());
	if (!bResult)
	{
		cTextFile.Kill();
		return NULL;
	}

	pcString = OMalloc(CString);
	
	if (!pcString)
	{
		return NULL;
	}
	pcString->Init(cTextFile.Text());

	cTextFile.Kill();
	return (CBaseObject*)pcString.ClearObject(FALSE);
}

