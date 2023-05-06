#ifndef __TRANSIENT_SEQUENCE_H__
#define __TRANSIENT_SEQUENCE_H__
#include "SequenceConnection.h"


class CTransientSequence : public CSequenceConnection
{
protected:
	OIndex	moiNext;

public:
	void	Init(void);
	void	Kill(void);

	OIndex	GetNext(void);
	OIndex	PeekNext(void);
};


extern CTransientSequence	gcTransientSequence;


void TransientSequenceInit(void);
void TransientSequenceKill(void);


#endif // !__TRANSIENT_SEQUENCE_H__

