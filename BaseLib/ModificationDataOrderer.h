#ifndef __MODIFICATION_DATA_ORDERER_H__
#define __MODIFICATION_DATA_ORDERER_H__
#include "Constructable.h"
#include "IndexTreeDataOrderer.h"
#include "LifeCycle.h"


class CModificationDataOrderer : public CIndexTreeDataOrderer
{
CONSTRUCTABLE(CModificationDataOrderer);
public:
	void	Init(void);
	void	Kill(void);

	void	Put(CIndexTreeDataNode* psNode);
	void	Get(CIndexTreeDataNode* psNode);
	void	Remove(CIndexTreeDataNode* psNode);
	void	HasKey(CIndexTreeDataNode* psNode);

public:
	static CLifeInit<CIndexTreeDataOrderer> Create(void);
};


#endif // __MODIFICATION_DATA_ORDERER_H__

