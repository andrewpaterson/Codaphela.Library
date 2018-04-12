#include "FileIndex.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CFilePosIndex::Init(void)
{
	miFile = -1;
	mulliFilePos = -1LL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CFilePosIndex::Init(int iFile, filePos ulliFilePos)
{
	miFile = iFile;
	mulliFilePos = ulliFilePos;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CFilePosIndex::Kill(void)
{
	miFile = -1;
	mulliFilePos = 0;
}

//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CFilePosIndex::HasFile(void)
{
	return miFile != -1;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CFilePosIndex::SetIndex(int iFile, filePos ulliFilePos)
{
	miFile = iFile;
	mulliFilePos = ulliFilePos;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CFileDataIndex::Init(void)
{
	miFile = -1;
	muiIndex = -1LL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CFileDataIndex::Init(int iFile, unsigned int uiIndex)
{
	miFile = iFile;
	muiIndex = uiIndex;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CFileDataIndex::Kill(void)
{
	miFile = -1;
	muiIndex = 0;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CFileDataIndex::HasFile(void)
{
	return miFile != -1;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CFileDataIndex::SetIndex(int iFile, unsigned int uiIndex)
{
	miFile = iFile;
	muiIndex = uiIndex;
}

