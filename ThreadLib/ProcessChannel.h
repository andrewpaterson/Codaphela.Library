#ifndef __PROCESS_CHANNEL_H__
#define __PROCESS_CHANNEL_H__
#include "BaseLib/PrimitiveTypes.h"
#include "BaseLib/StackMemory.h"


//Receive waits until a message arrives or the channel is closed.
#define CHANNEL_WAIT_FOREVER	-1


enum EChannelResult
{
	CR_Message,		//A message was received.
	CR_Timeout,		//No message arrived in time.
	CR_Closed,		//The other process closed the channel and every message it sent has been received.
	CR_TooLarge,	//The message was larger than the buffer and was dropped.  Its type and size are returned.
	CR_Error,
};


//Every channel sends this before the data of each message.  Both ends are on
//the same computer so it is not converted to network byte order.
struct SProcessChannelHeader
{
	uint32	uiType;
	uint32	uiSize;
};


//A two way connection between two processes that carries whole messages.
//Each message has a type, chosen by the caller, and its data.  Messages arrive
//in the order they were sent.  How a channel is created and connected depends
//on what carries it; see CNetworkConnection and CSharedMemoryChannel.
class CProcessChannel
{
public:
	virtual bool			Send(uint32 uiType, void* pvData, size uiSize) =0;

	//Waits up to iTimeoutMilliseconds; 0 returns at once and CHANNEL_WAIT_FOREVER does not time out.
	virtual EChannelResult	Receive(uint32* puiType, void* pvData, size uiMaxSize, size* puiSize, int iTimeoutMilliseconds) =0;

	virtual void			Close(void) =0;
	virtual bool			IsOpen(void) =0;

			bool			Send(uint32 uiType);

protected:
	//The header followed by a copy of the data, in pcTemp.  Its size is
	//sizeof(SProcessChannelHeader) + uiSize.  NULL if uiSize is too large.
	SProcessChannelHeader*	BuildMessage(CStackMemory<>* pcTemp, uint32 uiType, void* pvData, size uiSize);
};


#endif // __PROCESS_CHANNEL_H__

