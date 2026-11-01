#ifndef __PLAYER_LISTENER_H__
#define __PLAYER_LISTENER_H__
#include "StandardLib/Listener.h"
#include "StandardLib/Unknown.h"


class CPlayerListener : public CListener
{
CONSTRUCTABLE(CPlayerListener);
public:

	virtual void Up(CUnknown* pcSource, void* pvContext) {};
	virtual void Down(CUnknown* pcSource, void* pvContext) {};
	virtual void Left(CUnknown* pcSource, void* pvContext) {};
	virtual void Right(CUnknown* pcSource, void* pvContext) {};
	virtual void Fire(CUnknown* pcSource, void* pvContext) {};
};


#endif // __PLAYER_LISTENER_H__

