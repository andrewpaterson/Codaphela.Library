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
void CObjectConverterText::Init(CObjects* pcObjects)
{
	CObjectConverter::Init(pcObjects);
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
const char* CObjectConverterText::GetFileExtension(void)
{
	return "TXT";
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CObjectConverterText::IsFor(CAbstractFile* pcFile)
{
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CObjectSource* CObjectConverterText::CreateSource(CAbstractFile* pcFile, const char* szFilename)
{
	CObjectSourceText*	pcSource;

	if ((!pcFile) || (!szFilename))
	{
		return NULL;
	}

	pcSource = UMalloc(CObjectSourceText);
	pcSource->Init(this, pcFile, szFilename);
	return pcSource;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CBaseObject* CObjectConverterText::Convert(CObjectSource* pcSource, const char* szObjectName)
{
	CTextFile			cTextFile;
	Ptr<CString>		pString;
	bool				bResult;

	cTextFile.Init();

	bResult = cTextFile.Read(pcSource->GetFile());
	if (!bResult)
	{
		cTextFile.Kill();
		return NULL;
	}

	//Converted objects are named: it is how objects in the Codabase reference them.
	pString = mpcObjects->Malloc<CString>(szObjectName);
	if (pString.IsNull())
	{
		cTextFile.Kill();
		return NULL;
	}
	pString->Init(cTextFile.Text());
	cTextFile.Kill();
	return (CBaseObject*)pString.Return();
}

