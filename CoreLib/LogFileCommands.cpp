#include "LogFileCommands.h"

//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CLogFileCommand::Init(ELogFileCommand eCommand)
{
	this->eCommand = eCommand;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CLogFileCommand::IsWrite(void)
{
	return eCommand == LFC_Write;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CLogFileCommand::IsOpen(void)
{
	return eCommand == LFC_Open;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CLogFileCommand::IsClose(void)
{
	return eCommand == LFC_Close;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CLogFileCommand::IsDelete(void)
{
	return eCommand == LFC_Delete;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CLogFileCommandOpen::Init(EFileMode eMode)
{
	CLogFileCommand::Init(LFC_Open);
	this->eMode = eMode;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CLogFileCommandOpen::Open(CAbstractFile* pcFile)
{
	BOOL	bResult;

	bResult = pcFile->Open(eMode);
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CLogFileCommandWrite::Init(filePos iPosition, filePos iSize)
{
	CLogFileCommand::Init(LFC_Write);
	this->iPosition = iPosition;
	this->iSize = iSize;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CLogFileCommandWrite::Init(filePos iPosition, void* pvSource, filePos iSize)
{
	void*					pvData;

	Init(iPosition, iSize);

	pvData = RemapSinglePointer(this, sizeof(CLogFileCommandWrite));
	memcpy_fast(pvData, (void*)pvSource, (int)(iSize));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CLogFileCommandWrite::Write(CAbstractFile* pcFile)
{
	void*		pvData;
	filePos		iWritten;

	pcFile->Seek(iPosition, EFSO_SET);
	pvData = RemapSinglePointer(this, sizeof(CLogFileCommandWrite));

	iWritten = pcFile->Write(pvData, iSize, 1);
	if (iWritten != 1)
	{
		return FALSE;
	}
	else
	{
		return TRUE;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CLogFileCommandClose::Init(void)
{
	CLogFileCommand::Init(LFC_Close);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CLogFileCommandClose::Close(CAbstractFile* pcFile)
{
	BOOL bResult;

	bResult = pcFile->Close();
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CLogFileCommandDelete::Init(void)
{
	CLogFileCommand::Init(LFC_Delete);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CLogFileCommandDelete::Delete(CAbstractFile* pcFile)
{
	BOOL bResult;

	bResult = pcFile->Delete();
	return bResult;
}
