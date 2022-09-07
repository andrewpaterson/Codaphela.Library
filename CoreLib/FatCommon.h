#ifndef __FAT_COMMON_H__
#define __FAT_COMMON_H__

// Defines the maximun sector size (in bytes) that this library should
// support. An attempt to mount a volume with a sector size larger than this
// value will fail.
#define MAX_SECTOR_LENGTH				0x200


#define INT16_BYTE0		0
#define INT16_BYTE1		1
#define INT32_BYTE0		0
#define INT32_BYTE1		1
#define INT32_BYTE2		2
#define INT32_BYTE3		3
#define INT32_WORD0		0
#define INT32_WORD1		1


#endif // ! __FAT_COMMON_H__

