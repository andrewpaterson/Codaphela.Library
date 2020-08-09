#ifndef __CREATION_DATA_ORDERER_H__
#define __CREATION_DATA_ORDERER_H__
#include "Constructable.h"
#include "IndexTreeDataOrderer.h"


class CCreationDataOrderer : public CIndexTreeDataOrderer
{
CONSTRUCTABLE(CCreationDataOrderer);
public:
	void	Init(void);
	void	Kill(void);

	void	Put(CIndexTreeDataNode* psNode);
	void	Get(CIndexTreeDataNode* psNode);
	void	Remove(CIndexTreeDataNode* psNode);
	void	HasKey(CIndexTreeDataNode* psNode);
};


#endif // __CREATION_DATA_ORDERER_H__

