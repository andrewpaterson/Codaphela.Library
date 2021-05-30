#ifndef __GLOBAL_DATA_TYPES_IO_H__
#define __GLOBAL_DATA_TYPES_IO_H__
#include "DatasIO.h"


extern CDatasIO		gcDataTypesIO;
extern BOOL			gbDataTypesIO;


void DataIOInit(void);
void DataIOKill(void);
BOOL DataIOValidate(void);


#endif // __GLOBAL_DATA_TYPES_IO_H__

