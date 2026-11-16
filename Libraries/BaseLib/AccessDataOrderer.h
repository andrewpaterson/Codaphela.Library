#ifndef __ACCESS_DATA_ORDERER_H__
#define __ACCESS_DATA_ORDERER_H__
#include "IndexTreeDataOrderer.h"
#include "LifeCycle.h"


class CAccessDataOrderer : public CIndexTreeDataOrderer
{
CONSTRUCTABLE(CAccessDataOrderer)
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


#endif // __ACCESS_DATA_ORDERER_H__

