#ifndef _FAT_32_COMMON_H__
#define _FAT_32_COMMON_H__
#include <stdint.h>


#pragma pack(push, 1)

// Starting at offset 36 into the BPB, this is the structure for a FAT12/16 FS
typedef struct struct_BPBFAT1216_struct
{
    uint8_t     BS_DriveNumber;      
    uint8_t     BS_Reserved1;        
    uint8_t     BS_BootSig;          
    uint32_t    BS_VolumeID;         
    uint8_t     BS_VolumeLabel[11];  
    uint8_t     BS_FileSystemType[8];
} BPB1216_struct;


// Starting at offset 36 into the BPB, this is the structure for a FAT32 FS
typedef struct struct_BPBFAT32_struct
{
    uint32_t    FATSize;      
    uint16_t    ExtFlags;           
    uint16_t    FSVersion;          
    uint32_t    RootCluster;  
    uint16_t    FSInfo;             
    uint16_t    BkBootSec;          
    uint8_t     Reserved[12];       
    uint8_t     BS_DriveNumber;     
    uint8_t     BS_Reserved1;       
    uint8_t     BS_BootSig;         
    uint32_t    BS_VolumeID;  
    uint8_t     BS_VolumeLabel[11]; 
    uint8_t     BS_FileSystemType[8];
} BPB32_struct;


typedef struct struct_BPB_struct
{
    uint8_t     BS_JumpBoot[3];
    char        BS_OEMName[8];
    uint16_t    BytesPerSector;     
    uint8_t     SectorsPerCluster;  
    uint16_t    ReservedSectorCount;
    uint8_t     NumFATs;            
    uint16_t    RootEntryCount;     
    uint16_t    TotalSectors16;     
    uint8_t     Media;              
    uint16_t    FATSize16;          
    uint16_t    SectorsPerTrack;    
    uint16_t    NumberOfHeads;      
    uint32_t    HiddenSectors;
    uint32_t    TotalSectors32;     
    union 
    {
        BPB1216_struct fat16;
        BPB32_struct fat32;
    } FSTypeSpecificData;
} BPB_struct;


typedef struct struct_FatFile83 
{
    char        filename[8];
    char        extension[3];
    uint8_t     attributes;
    uint8_t     reserved;
    uint8_t     creationTimeMs;
    uint16_t    creationTime;
    uint16_t    creationDate;
    uint16_t    lastAccessTime;
    uint16_t    eaIndex;
    uint16_t    modifiedTime;
    uint16_t    modifiedDate;
    uint16_t    firstCluster;
    uint32_t    fileSize;
} FatFile83;


typedef struct struct_FatFileLFN 
{
    uint8_t     sequence_number;
    uint16_t    name1[5];      // 5 Chars of name (UTF 16???)
    uint8_t     attributes;     // Always 0x0f
    uint8_t     reserved;       // Always 0x00
    uint8_t     checksum;       // Checksum of DOS Filename.  See Docs.
    uint16_t    name2[6];      // 6 More chars of name (UTF-16)
    uint16_t    firstCluster;  // Always 0x0000
    uint16_t    name3[2];
} FatFileLFN;


typedef union struct_FatFileEntry 
{
    FatFile83 msdos;
    FatFileLFN lfn;
} FatFileEntry;


#pragma pack(pop)


// "Legacy" functions
uint32_t fat_size(BPB_struct* bpb);
int total_sectors(BPB_struct* bpb);
int root_dir_sectors(BPB_struct* bpb);
int cluster_count(BPB_struct* bpb);
int fat_type(BPB_struct* bpb);
int first_data_sector(BPB_struct* bpb);
int first_sector_of_cluster(BPB_struct* bpb, int N);
int data_sectors(BPB_struct* bpb);
int fat_sector_number(BPB_struct* bpb, int N);
int fat_entry_offset(BPB_struct* bpb, int N);
int fat_entry_for_cluster(BPB_struct* bpb, uint8_t* buffer, int N);

#endif // _FAT_32_COMMON_H__

