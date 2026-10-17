#include <string.h>
#include "BaseLib/Logger.h"
#include "BaseLib/LogString.h"
#include "BaseLib/PointerRemapper.h"
#include "ProcessChannel.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CProcessChannel::Send(uint32 uiType)
{
	return Send(uiType, NULL, 0);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
SProcessChannelHeader* CProcessChannel::BuildMessage(CStackMemory<>* pcTemp, uint32 uiType, void* pvData, size uiSize)
{
	SProcessChannelHeader*	psHeader;

	if (uiSize > MAX_UINT32 - sizeof(SProcessChannelHeader))
	{
		gcLogger.Error2(__METHOD__, " Message of [", SizeToString(uiSize), "] bytes is too large to send.", NULL);
		return NULL;
	}

	psHeader = (SProcessChannelHeader*)pcTemp->Init(sizeof(SProcessChannelHeader) + uiSize);
	psHeader->uiType = uiType;
	psHeader->uiSize = (uint32)uiSize;
	if (uiSize > 0)
	{
		memcpy(RemapSinglePointer(psHeader, sizeof(SProcessChannelHeader)), pvData, uiSize);
	}
	return psHeader;
}

