#ifndef __NETWORK_CONNECTION_H__
#define __NETWORK_CONNECTION_H__
#include "BaseLib/PrimitiveTypes.h"
#include "BaseLib/ConnectSocket.h"
#include "BaseLib/ListenSocket.h"
#include "ProcessChannel.h"


//The loopback network; connections to it never leave the computer.
#define NETWORK_LOOPBACK_ADDRESS	"127.0.0.1"


//A process channel over a TCP connection.  One process listens with a
//CListenSocket and Accepts, the other Connects.
class CNetworkConnection : public CProcessChannel
{
protected:
	CConnectSocket	mcSocket;

public:
	void			Init(void);
	void			Kill(void);

	bool			Connect(const char* szAddress, int iPort);
	bool			Accept(CListenSocket* pcListen, int iTimeoutMilliseconds);

	using			CProcessChannel::Send;
	bool			Send(uint32 uiType, void* pvData, size uiSize);
	EChannelResult	Receive(uint32* puiType, void* pvData, size uiMaxSize, size* puiSize, int iTimeoutMilliseconds);
	void			Close(void);
	bool			IsOpen(void);

protected:
	bool			Discard(size uiSize);
};


#endif // __NETWORK_CONNECTION_H__

