#ifndef __SEQUENCE_H__
#define __SEQUENCE_H__
#include "SequenceConfig.h"
#include "DurableFileController.h"
#include "IndexedGeneral.h"
#include "SequenceConnection.h"

//Hands out OIndexes from GetFirstIndex up to (not including) GetEndIndex of its config.  ReserveRange takes a range out of
//the sequence, written to its file at once, for a range-limited sequence (CValueSequenceConfig::CreateRange) to hand out.
class CSequence : public CSequenceConnection
{
protected:
	CDurableFileController		mcFileController;
	OIndex						moiNext;
	OIndex						moiReserved;  //The index written to the Sequence file.  Indices below it may be handed out without writing.
	OIndex						moiEnd;
	CDurableFile				mcFile;
	CSequenceConfig*			mpcConfig;
	CLife<CSequenceConfig>		mcConfig;

public:
	bool	Init(CLifeInit<CSequenceConfig> cConfig);
	void	Kill(void);

	OIndex	GetNext(void);
	OIndex	PeekNext(void);
	OIndex	GetEnd(void);
	OIndex	ReserveRange(OIndex iCount);

protected:
	OIndex	ReadIndex(void);
	bool	WriteIndex(OIndex oi);
	bool	Reserve(OIndex oiReserved);
	bool	MoveNext(OIndex oiNext);
};


#endif // __SEQUENCE_H__

