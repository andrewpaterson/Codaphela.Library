#include "IndexedDataFiles.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIndexedDataFiles::Init(CDurableFileController* pcDurableFileControl, char* szExtension)
{
	CIndexedFiles::Init(pcDurableFileControl, szExtension);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIndexedDataFiles::Kill(void)
{
	CIndexedFiles::Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexedDataFiles::WriteData(CIndexedDataDescriptor* pcDescriptor, void* pvData)
{
	if (pcDescriptor->HasFile())
	{
		return WriteExisting(pcDescriptor, pvData);
	}
	else
	{
		return WriteNew(pcDescriptor, pvData);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexedDataFiles::WriteNew(CIndexedDataDescriptor* pcIndexDescriptor, void* pvData)
{
	CIndexedFile*	pcIndexedFile;
	filePos			iIndex;
	int				iDataSize;

	iDataSize = pcIndexDescriptor->GetDataSize();
	if (iDataSize != 0)
	{
		pcIndexedFile = GetOrCreateFile(iDataSize);

		iIndex = pcIndexedFile->Write(pvData);
		if (iIndex == -1)
		{
			return FALSE;
		}

		pcIndexDescriptor->SetIndexes(pcIndexedFile->GetFileIndex(), iIndex);
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexedDataFiles::WriteExisting(CIndexedDataDescriptor* pcIndexDescriptor, void* pvData)
{
	CIndexedFile*	pcIndexedFile;
	filePos			iResult;

	pcIndexedFile = GetFile(pcIndexDescriptor->GetFileIndex());
	if (pcIndexedFile)
	{
		iResult = pcIndexedFile->Write(pcIndexDescriptor->GetIndexInFile(), pvData);
		return iResult == 1;
	}
	else
	{
		return FALSE;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexedDataFiles::Read(CIndexedDataDescriptor* pcIndexDescriptor, void* pvData)
{
	CIndexedFile*	pcIndexedFile;

	pcIndexedFile = GetFile(pcIndexDescriptor->GetFileIndex());
	if (!pcIndexedFile)
	{
		return FALSE;
	}

	return pcIndexedFile->Read(pcIndexDescriptor->GetIndexInFile(), pvData);
}

