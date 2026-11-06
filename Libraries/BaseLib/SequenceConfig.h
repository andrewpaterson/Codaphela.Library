#ifndef __SEQUENCE_CONFIG_H__
#define __SEQUENCE_CONFIG_H__
#include "LifeCycle.h"
#include "Constructable.h"
#include "ConstructorCall.h"
#include "Killable.h"
#include "DurableSync.h"
#include "IndexedGeneral.h"


//The number of indices a sequence reserves on disk at a time.  Indices left in a block when the process dies are never handed out.
#define SEQUENCE_BLOCK_SIZE		256


class CSequenceConfig : public CConstructable, public CKillable
{
CONSTRUCTABLE(CSequenceConfig)
public:
			void		Kill(void);

	virtual	const char*		Getdirectory(void) =0;
	virtual	EDurableSync	GetSync(void) =0;
	virtual	size			GetBlockSize(void) =0;
	virtual	OIndex			GetFirstIndex(void) =0;	//The first index handed out by a new sequence.
	virtual	OIndex			GetEndIndex(void) =0;	//One past the last index the sequence may hand out (MAX_O_INDEX: no end).
};


#endif // __SEQUENCE_CONFIG_H__

