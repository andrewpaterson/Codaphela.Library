#ifndef __SEQUENCE_H__
#define __SEQUENCE_H__
#include "SequenceConfig.h"
#include "DurableFileController.h"
#include "IndexedGeneral.h"
#include "SequenceConnection.h"

class CSequence : public CSequenceConnection
{
protected:
	CDurableFileController		mcFileController;
	OIndex						moiNext;
	OIndex						moiReserved;  //The index written to the Sequence file.  Indices below it may be handed out without writing.
	CDurableFile				mcFile;
	CSequenceConfig*			mpcConfig;
	CLife<CSequenceConfig>		mcConfig;

public:
	bool	Init(CLifeInit<CSequenceConfig> cConfig);
	void	Kill(void);

	OIndex	GetNext(void);
	OIndex	PeekNext(void);

protected:
	OIndex	ReadIndex(void);
	bool	WriteIndex(OIndex oi);
	bool	Reserve(OIndex oiReserved);
};


#endif // __SEQUENCE_H__

