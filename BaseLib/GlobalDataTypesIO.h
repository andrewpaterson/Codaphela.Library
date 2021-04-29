#ifndef __GLOBAL_DATA_TYPES_IO_H__
#define __GLOBAL_DATA_TYPES_IO_H__
#include "DataTypesIO.h"


extern CDataTypesIO			gcDataTypesIO;
extern BOOL					gbDataTypesIO;


void DataIOInit(void);
void DataIOKill(void);
BOOL DataIOValidate(void);


#endif // __GLOBAL_DATA_TYPES_IO_H__

