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
BOOL CIndexedDataFiles::WriteNew(CIndexDescriptor* pcDescriptor, void* pvData)
{
	CIndexedFile*	pcIndexedFile;
	filePos			iIndex;
	int				iDataSize;

	iDataSize = pcDescriptor->GetDataSize();
	if (iDataSize != 0)
	{
		pcIndexedFile = GetOrCreateFile(iDataSize);

		if (pcIndexedFile->mbNew)
		{
			pcIndexedFile->mbNew = FALSE;
			WriteIndexedFileDescriptors();
		}

		iIndex = pcIndexedFile->Write(pvData);
		if (iIndex == -1)
		{
			return FALSE;
		}

		//CIndexDescriptor (pcDescriptor) adjusted here.
		pcDescriptor->File(pcIndexedFile->miFileIndex, iIndex);
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
BOOL CIndexedDataFiles::WriteExisting(CIndexDescriptor* pcDescriptor, void* pvData)
{
	CIndexedFile*	pcIndexedFile;
	filePos			iResult;

	pcIndexedFile = GetFile(pcDescriptor->GetFileIndex());
	if (pcIndexedFile)
	{
		iResult = pcIndexedFile->Write(pcDescriptor->GetIndexInFile(), pvData);
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
BOOL CIndexedDataFiles::Read(CIndexDescriptor* pcDescriptor, void* pvData)
{
	CIndexedFile*	pcIndexedFile;

	pcIndexedFile = GetFile(pcDescriptor->GetFileIndex());
	if (!pcIndexedFile)
	{
		return FALSE;
	}

	return pcIndexedFile->Read(pcDescriptor->GetIndexInFile(), pvData);
}


