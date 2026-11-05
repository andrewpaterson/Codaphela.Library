#ifndef __TEST_PLAYER_H__
#define __TEST_PLAYER_H__
#include "BaseLib/ArrayChars.h"
#include "StandardLib/Action.h"


class CPlayer
{
protected:
	CArrayChars		maszEvents;

public:
	void 			Init(void);
	void 			Kill(void);
	void 			Up(CAction* pcAction, void* pvContext);
	void 			Down(CAction* pcAction, void* pvContext);
	void 			Left(CAction* pcAction, void* pvContext);
	void 			Right(CAction* pcAction, void* pvContext);
	void 			Fire(CAction* pcAction, void* pvContext);
	void 			RecordAction(CAction* pcAction, char* szText, void* pvContext);
	CArrayChars*	GetEvents(void);
};


#endif // __TEST_PLAYER_H__

