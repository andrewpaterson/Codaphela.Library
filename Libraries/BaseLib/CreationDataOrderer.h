#ifndef __CREATION_DATA_ORDERER_H__
#define __CREATION_DATA_ORDERER_H__
#include "Constructable.h"
#include "IndexTreeDataOrderer.h"
#include "LifeCycle.h"


class CCreationDataOrderer : public CIndexTreeDataOrderer
{
CONSTRUCTABLE(CCreationDataOrderer)
public:
	void	Init(void);
	void	Kill(void) override;

	void	Put(CIndexTreeDataNode* psNode) override;
	void	Get(CIndexTreeDataNode* psNode) override;
	void	Remove(CIndexTreeDataNode* psNode) override;
	void	HasKey(CIndexTreeDataNode* psNode) override;

public:
	static CLifeInit<CIndexTreeDataOrderer> Create(void);
};


#endif // __CREATION_DATA_ORDERER_H__

