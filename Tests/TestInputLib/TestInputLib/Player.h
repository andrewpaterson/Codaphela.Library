#ifndef __TEST_PLAYER_H__
#define __TEST_PLAYER_H__
#include "BaseLib/ArrayChars.h"
#include "StandardLib/Action.h"
#include "PlayerListener.h"


class CPlayer : public CPlayerListener
{
CONSTRUCTABLE(CPlayer);
protected:
	CArrayChars		maszEvents;

public:
	void 			Init(void);
	void 			Kill(void);
	void 			Up(CUnknown* pcSource, void* pvContext);
	void 			Down(CUnknown* pcSource, void* pvContext);
	void 			Left(CUnknown* pcSource, void* pvContext);
	void 			Right(CUnknown* pcSource, void* pvContext);
	void 			Fire(CUnknown* pcSource, void* pvContext);
	void 			RecordAction(CAction* pcAction, char* szText, void* pvContext);
	CArrayChars*	GetEvents(void);
};


#endif // __TEST_PLAYER_H__

