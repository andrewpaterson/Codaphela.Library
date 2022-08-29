#ifndef __FAT32_DEBUG_H__
#define __FAT32_DEBUG_H__
#include "Fat32.h"
#include "Fat32Common.h"


void printBPB(BPB_struct* s);
void print_sector(unsigned char* sector);
void print_media(unsigned char mediatype);
void print_TFFile(TFFile* fp);
void print_tf_info(TFInfo* t);

void print_FatFileEntry(FatFileEntry* entry);
void print_FatFileLFN(FatFileLFN* entry);
void print_FatFile83(FatFile83* entry);


#endif

