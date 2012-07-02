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
	CIndexedFile*	pcFile;
	int				iIndex;
	int				iDataSize;

	iDataSize = pcDescriptor->GetDataSize();
	if (iDataSize != 0)
	{
		pcFile = GetOrCreateFile(iDataSize);

		if (pcFile->mbNew)
		{
			pcFile->mbNew = FALSE;
			WriteIndexedFileDescriptors();
		}

		iIndex = pcFile->Write(pvData);
		if (iIndex == -1)
		{
			return FALSE;
		}

		//CIndexDescriptor (pcDescriptor) adjusted here.
		pcDescriptor->File(pcFile->miFileIndex, iIndex);
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
	CIndexedFile*		pcFile;

	pcFile = GetFile(pcDescriptor->GetFileIndex());
	if (pcFile)
	{
		return pcFile->Write(pcDescriptor->GetIndexInFile(), pvData);
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
	CIndexedFile*		pcFile;

	pcFile = GetFile(pcDescriptor->GetFileIndex());
	if (!pcFile)
	{
		return FALSE;
	}

	return pcFile->Read(pcDescriptor->GetIndexInFile(), pvData);
}


