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
	CDurableFile				mcFile;
	CSequenceConfig*			mpcConfig;
	CLife<CSequenceConfig>		mcConfig;

public:
	BOOL	Init(CLifeInit<CSequenceConfig> cConfig);
	void	Kill(void);

	OIndex	GetNext(void);
	OIndex	PeekNext(void);

protected:
	OIndex	ReadIndex(void);
	BOOL	WriteIndex(OIndex oi);
};


#endif // !__SEQUENCE_H__

