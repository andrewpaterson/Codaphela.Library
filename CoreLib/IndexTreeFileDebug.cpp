#include "BaseLib/Logger.h"
#include "IndexedFile.h"
#include "IndexTreeFileDebug.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int SIndexTreeDebugNode::InitFromBuffer(void* pvBuffer, int iBufferSize, int iFile, unsigned int uiIndex)
{
	unsigned char*			pucMemory;
	int						iPos;
	int						iFileSize;
	int						iNumCleared;
	int						i;

	memset(this, 0, sizeof(SIndexTreeDebugNode));
	iFileNumber = iFile;
	uiIndexInFile = uiIndex;
	iFileNodeSize = iBufferSize;

	pucMemory = (unsigned char*)pvBuffer;
	iPos = 0;

	iNumCleared = 0;
	for (i = 0; i < iBufferSize; i++)
	{
		if (((char*)pvBuffer)[i] == INDEX_FILE_EMPTY_CHAR)
		{
			iNumCleared++;
		}
	}

	if (iNumCleared == iBufferSize)
	{
		bEmpty = TRUE;
		return -1;
	}

	iFileSize = *((int*)&pucMemory[iPos]);  iPos += sizeof(int);
	if (iBufferSize != iFileSize)
	{
		gcLogger.Error2(__METHOD__, " Node buffer size [", IntToString(iBufferSize), "] did not match node size in file [", IntToString(iFileSize), "].", NULL);
	}

	uiDataSize = *((unsigned short*)&pucMemory[iPos]);  iPos += sizeof(unsigned short);

	uiFirstIndex = pucMemory[iPos];  iPos++;
	uiLastIndex = pucMemory[iPos];  iPos++;
	uiIndexInParent = pucMemory[iPos];  iPos++;
	sFlags = pucMemory[iPos];  iPos++;
	iPos += uiDataSize;
	return iPos;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void SIndexTreeDebugNode::InitBroken(unsigned char uiIndexInParent)
{
	memset(this, 0, sizeof(SIndexTreeDebugNode));
	iFileNumber = -1;
	uiIndexInFile = -1;
	this->uiIndexInParent = uiIndexInParent;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void SIndexTreeDebugNode::Print(CChars* sz)
{
	sz->Append("FileNumber[");
	sz->Append(iFileNumber);
	sz->Append("], ");

	sz->Append("IndexInFile[");
	sz->Append(uiIndexInFile);
	sz->Append("], ");

	sz->Append("FileNodeSize[");
	sz->Append(iFileNodeSize);
	sz->Append("], ");

	sz->Append("Empty[");
	sz->AppendBool(bEmpty);
	sz->Append("], ");

	sz->Append("DataSize[");
	sz->Append(uiDataSize);
	sz->Append("], ");

	sz->Append("FirstIndex[");
	sz->Append(uiFirstIndex);
	sz->Append("], ");

	sz->Append("LastIndex[");
	sz->Append(uiLastIndex);
	sz->Append("], ");

	sz->Append("IndexInParent[");
	sz->Append(uiIndexInParent);
	sz->Append("], ");

	sz->Append("Flags[");
	sz->AppendHexHiLo(&sFlags, 1);
	sz->Append("]");
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void SIndexTreeDebugNode::Dump(void)
{
	CChars sz;

	sz.Init();
	Print(&sz);
	sz.AppendNewLine();
	sz.Dump();
	sz.Kill();
}

