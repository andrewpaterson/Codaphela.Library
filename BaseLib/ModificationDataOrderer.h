#ifndef __MODIFICATION_DATA_ORDERER_H__
#define __MODIFICATION_DATA_ORDERER_H__
#include "IndexTreeDataOrderer.h"


class CModificationDataOrderer : public CIndexTreeDataOrderer
{
public:
	void	Init(void);
	void	Kill(void);

	void	Put(CIndexTreeDataNode* psNode);
	void	Get(CIndexTreeDataNode* psNode);
	void	Remove(CIndexTreeDataNode* psNode);
	void	HasKey(CIndexTreeDataNode* psNode);
};


#endif // __MODIFICATION_DATA_ORDERER_H__
