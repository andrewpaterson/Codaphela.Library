#include "Logger.h"
#include "IndexedFile.h"
#include "IndexTreeFileDebug.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int SIndexTreeDebugNode::InitFromBuffer(void* pvBuffer, size uiBufferSize, uint32 uiFileIndex, uint32 uiIndexInFile)
{
	uint8*	pucMemory;
	int		iPos;
	uint32	uiFileSize;
	uint32	iFileDataSize;
	int		iNumCleared;
	size	i;

	memset(this, 0, sizeof(SIndexTreeDebugNode));
	this->uiFileIndex = uiFileIndex;
	this->uiIndexInFile = uiIndexInFile;
	this->iFileNodeSize = (uint16)uiBufferSize;

	pucMemory = (uint8*)pvBuffer;
	iPos = 0;

	iNumCleared = 0;
	for (i = 0; i < uiBufferSize; i++)
	{
		if (((char*)pvBuffer)[i] == INDEX_FILE_EMPTY_CHAR)
		{
			iNumCleared++;
		}
	}

	if (iNumCleared == uiBufferSize)
	{
		bEmpty = true;
		return -1;
	}

	uiFileSize = *((int*)&pucMemory[iPos]);  iPos += sizeof(uint32);
	if (uiBufferSize != uiFileSize)
	{
		gcLogger.Error2(__METHOD__, " Node buffer size [", IntToString(uiBufferSize), "] did not match node size in file [", IntToString(uiFileSize), "].", NULL);
		return -1;
	}

	iFileDataSize = *((int*)&pucMemory[iPos]);  iPos += sizeof(uint32);
	uiDataSize = *((uint16*)&pucMemory[iPos]);  iPos += sizeof(uint16);

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
void SIndexTreeDebugNode::InitBroken(size uiIndexInParent)
{
	memset(this, 0, sizeof(SIndexTreeDebugNode));
	uiFileIndex = MAX_UINT;
	uiIndexInFile = MAX_UINT;
	this->uiIndexInParent = (uint8)uiIndexInParent;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void SIndexTreeDebugNode::Print(CChars* sz)
{
	sz->Append("FileNumber[");
	sz->Append(uiFileIndex);
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

