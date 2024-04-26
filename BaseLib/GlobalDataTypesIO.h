#ifndef __GLOBAL_DATA_TYPES_IO_H__
#define __GLOBAL_DATA_TYPES_IO_H__
#include "DatasIO.h"


extern CDatasIO		gcDataTypesIO;
extern bool			gbDataTypesIO;


void DataIOInit(void);
void DataIOKill(void);
bool DataIOValidate(void);


#endif // __GLOBAL_DATA_TYPES_IO_H__

