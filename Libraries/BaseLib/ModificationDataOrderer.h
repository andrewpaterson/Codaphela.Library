#ifndef __MODIFICATION_DATA_ORDERER_H__
#define __MODIFICATION_DATA_ORDERER_H__
#include "Constructable.h"
#include "IndexTreeDataOrderer.h"
#include "LifeCycle.h"


class CModificationDataOrderer : public CIndexTreeDataOrderer
{
CONSTRUCTABLE(CModificationDataOrderer)
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


#endif // __MODIFICATION_DATA_ORDERER_H__

