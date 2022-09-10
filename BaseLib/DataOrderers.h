#ifndef __ACCESS_DATA_ORDERERS_H__
#define __ACCESS_DATA_ORDERERS_H__
#include "MapStringTemplate.h"
#include "IndexTreeDataOrderer.h"


class CDataOrderers
{
protected:
	CMapStringTemplate<CIndexTreeDataOrderer*>	mmszClasses;

public:
	void					Init(void);
	void					Kill(void);

	bool					Add(CIndexTreeDataOrderer* pcMalloc);

	CIndexTreeDataOrderer*	Read(CFileReader* pcFileReader);
	bool					Write(CFileWriter* pcFileWriter, CIndexTreeDataOrderer* pcDataOrderer);
};


void DataOrderersInit(void);
void DataOrderersInit(CConstructors* pcConstructors, CDataOrderers* pcDataOrderers);
void DataOrderersKill(void);
bool DataOrderersValidate(void);


extern CDataOrderers	gcDataOrderers;
extern bool				gbDataOrderers;


#endif // !__ACCESS_DATA_ORDERERS_H__

