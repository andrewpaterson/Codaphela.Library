#ifndef __ACCESS_DATA_ORDERER_H__
#define __ACCESS_DATA_ORDERER_H__
#include "IndexTreeDataOrderer.h"


class CAccessDataOrderer : public CIndexTreeDataOrderer
{
CONSTRUCTABLE(CAccessDataOrderer);
public:
	void	Init(void);
	void	Kill(void);

	void	Put(CIndexTreeDataNode* psNode);
	void	Get(CIndexTreeDataNode* psNode);
	void	Remove(CIndexTreeDataNode* psNode);
	void	HasKey(CIndexTreeDataNode* psNode);
};


#endif // __ACCESS_DATA_ORDERER_H__

