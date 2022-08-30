#include <string.h>
#include <stdio.h>
#include "Fat32.h"
#include "Fat32Debug.h"
#include "Fat32Common.h"
#include "MemoryDrive.h"


CMemoryDrive    gcDrive;


// USERLAND
int read_sector(uint8_t* data, uint32_t sector)
{
    if (gcDrive.CFileDrive::Read(sector, data))
    {
        return TF_ERR_NO_ERROR;
    }
    else
    {
        return 1;
    }
}

int write_sector(uint8_t* data, uint32_t sector)
{
    if (gcDrive.CFileDrive::Write(sector, data))
    {
        return TF_ERR_NO_ERROR;
    }
    else
    {
        return 1;
    }
}


//#define TF_DEBUG

TFInfo tf_info;
TFFile tf_file_handles[TF_FILE_HANDLES];
#ifdef TF_DEBUG
TFStats tf_stats;
#endif

/*
 * Fetch a single sector from disk.
 * ARGS
 *   sector - the sector number to fetch.
 * SIDE EFFECTS
 *   tf_info.buffer contains the DRIVE_SECTOR_SIZE byte sector requested
 *   tf_info.currentSector contains the sector number retrieved
 *   if tf_info.buffer already contained a fetched sector, and was marked dirty, that sector is
 *   tf_store()d back to its appropriate location before executing the fetch.
 * RETURN
 *   the return code given by the users read_sector() (should be zero for NO ERROR, nonzero otherwise)
 */
int tf_fetch(uint32_t sector) {
    int rc = 0;
    // Don't actually do the fetch if we already have it in memory
    if (sector == tf_info.currentSector)
    {
        return TF_ERR_NO_ERROR;
    }

    // If the sector we already have prefetched is dirty, write it before reading out the new one
    if (tf_info.sectorFlags & TF_FLAG_DIRTY) 
    {
        rc |= tf_store();
    }

    // Do the read, pass up the error flag
    rc |= read_sector(tf_info.buffer, sector);
    if (!rc) tf_info.currentSector = sector;
    return rc;
}

/*
 * Store the current sector back to disk
 * SIDE EFFECTS
 *   DRIVE_SECTOR_SIZE bytes of tf_info.buffer are stored on disk in the sector specified by tf_info.currentSector
 * RETURN
 *   the error code given by the users write_sector() (should be zero for NO ERROR, nonzero otherwise)
 */
int tf_store()
{
    tf_info.sectorFlags &= ~TF_FLAG_DIRTY;
    return write_sector(tf_info.buffer, tf_info.currentSector);
}

/*
 * Initialize the filesystem
 * Reads filesystem info from disk into tf_info and checks that info for validity
 * SIDE EFFECTS
 *   Sector 0 is fetched into tf_info.buffer
 *   If TF_DEBUG is specified tf_stats is initialized
 * RETURN
 *   0 for a successfully initialized filesystem, nonzero otherwise.
 */
int tf_init(void)
{
    BPB_struct* bpb;
    uint32_t fat_size, root_dir_sectors, data_sectors, cluster_count, temp;
    TFFile* fp;
    FatFileEntry e;

    // Initialize the runtime portion of the TFInfo structure, and read sec0
    tf_info.currentSector = -1;
    tf_info.sectorFlags = 0;
    tf_fetch(0);

    // Cast to a BPB, so we can extract relevant data
    bpb = (BPB_struct*)tf_info.buffer;

    /* Some sanity checks to make sure we're really dealing with FAT here
     * see fatgen103.pdf pg. 9ff. for details */

     /* BS_jmpBoot needs to contain specific instructions */
    if (!(bpb->BS_JumpBoot[0] == 0xEB && bpb->BS_JumpBoot[2] == 0x90) && !(bpb->BS_JumpBoot[0] == 0xE9))
    {
        //tf_init FAILED: stupid jmp instruction isn't exactly right...
        return TF_ERR_BAD_FS_TYPE;
    }

    /* Only specific bytes per sector values are allowed
     * FIXME: Only DRIVE_SECTOR_SIZE bytes are supported by thinfat at the moment */
    if (bpb->BytesPerSector != DRIVE_SECTOR_SIZE)
    {
        //tf_init FAILED: Bad Filesystem Type (!=DRIVE_SECTOR_SIZE bytes/sector)
        return TF_ERR_BAD_FS_TYPE;
    }

    if (bpb->ReservedSectorCount == 0)
    {
        //tf_init() FAILED: ReservedSectorCount == 0
        return TF_ERR_BAD_FS_TYPE;
    }

    /* Valid media types */
    if ((bpb->Media != 0xF0) && ((bpb->Media < 0xF8) || (bpb->Media > 0xFF)))
    {
        //tf_init() FAILED: Invalid Media Type!  (0xf0, or 0xf8 <= type <= 0xff)
        return TF_ERR_BAD_FS_TYPE;
    }

    // See the FAT32 SPEC for how this is all computed
    fat_size = (bpb->FATSize16 != 0) ? bpb->FATSize16 : bpb->FSTypeSpecificData.fat32.FATSize;
    root_dir_sectors = ((bpb->RootEntryCount * 32) + (bpb->BytesPerSector - 1)) / (DRIVE_SECTOR_SIZE); // The DRIVE_SECTOR_SIZE here is a hardcoded bpb->bytesPerSector (TODO: Replace /,* with shifts?)
    tf_info.totalSectors = (bpb->TotalSectors16 != 0) ? bpb->TotalSectors16 : bpb->TotalSectors32;
    data_sectors = tf_info.totalSectors - (bpb->ReservedSectorCount + (bpb->NumFATs * fat_size) + root_dir_sectors);
    tf_info.sectorsPerCluster = bpb->SectorsPerCluster;
    cluster_count = data_sectors / tf_info.sectorsPerCluster;
    tf_info.reservedSectors = bpb->ReservedSectorCount;
    tf_info.firstDataSector = bpb->ReservedSectorCount + (bpb->NumFATs * fat_size) + root_dir_sectors;

    // Now that we know the total count of clusters, we can compute the FAT type
    if (cluster_count < 65525)
    {
        //tf_init FAILED: cluster_count < 65525
        return TF_ERR_BAD_FS_TYPE;
    }
    else tf_info.type = TF_TYPE_FAT32;

    // TODO ADD SANITY CHECKING HERE (CHECK THE BOOT SIGNATURE, ETC... ETC...)
    tf_info.rootDirectorySize = 0xffffffff;
    temp = 0;

    // Like recording the root directory size!
    // TODO, THis probably isn't necessary.  Remove later
    fp = tf_fopen("/", "r");
    do 
    {
        temp += sizeof(FatFileEntry);
        tf_fread((uint8_t*)&e, sizeof(FatFileEntry), fp);
    } 
    while (e.msdos.filename[0] != '\x00');
    tf_fclose(fp);
    tf_info.rootDirectorySize = temp;

    tf_fclose(fp);
    tf_release_handle(fp);
    return TF_ERR_NO_ERROR;
}

/*
 * Return the FAT entry for the given cluster
 * ARGS
 *   cluster - The cluster number for the requested FAT entry
 * SIDE EFFECTS
 *   Retreives whatever sector happens to contain that FAT entry (if it's not already in memory)
 * RETURN
 *   The value of the fat entry for the specified cluster.
 */
uint32_t tf_get_fat_entry(uint32_t cluster) 
{
    uint32_t offset = cluster * 4;
    tf_fetch(tf_info.reservedSectors + (offset / DRIVE_SECTOR_SIZE)); // DRIVE_SECTOR_SIZE is hardcoded bpb->bytesPerSector
    return *((uint32_t*)&(tf_info.buffer[offset % DRIVE_SECTOR_SIZE]));
}

/*
 * Sets the fat entry on disk for a given cluster to the specified value.
 * ARGS
 *   cluster - The cluster number for which to set the FAT entry
 *     value - The new value for the FAT entry
 * SIDE EFFECTS
 *   Fetches whatever sector happens to contain the pertinent fat entry (if it's not already in memory)
 * RETURN
 *   0 for no error, or nonzero for error with fetch
 * TODO
 *   Does the sector modified here need to be flagged as dirty?
 */
int tf_set_fat_entry(uint32_t cluster, uint32_t value) 
{
    uint32_t    offset;
    int         rc;
    offset = cluster * 4; // FAT32
    rc = tf_fetch(tf_info.reservedSectors + (offset / DRIVE_SECTOR_SIZE)); // DRIVE_SECTOR_SIZE is hardcoded bpb->bytesPerSector
    if (*((uint32_t*)&(tf_info.buffer[offset % DRIVE_SECTOR_SIZE])) != value) 
    {
        tf_info.sectorFlags |= TF_FLAG_DIRTY; // Mark this sector as dirty
        *((uint32_t*)&(tf_info.buffer[offset % DRIVE_SECTOR_SIZE])) = value;
    }
    return rc;
}


/*
 * Return the index of the first sector for the provided cluster
 * ARGS
 *   cluster - The cluster of interest
 * RETURN
 *   The first sector of the provided cluster
 */
uint32_t tf_first_sector(uint32_t cluster) 
{
    return ((cluster - 2) * tf_info.sectorsPerCluster) + tf_info.firstDataSector;
}

/*
 * Walks the path provided, returning a valid file pointer for each successive level in the path.
 *
 * example:  tf_walk("/home/ryan/myfile.txt", fp to "/")
 *           Call once: returns pointer to string: home/ryan/myfile.txt, fp now points to directory for /
 *          Call again: returns pointer to string: ryan/myfile.txt, fp now points to directory for /home
 *          Call again: returns pointer to string: myfile.txt, fp now points to directory for /home/ryan
 *          Call again: returns pointer to the end of the string, fp now points to /home/ryan/myfile.txt
 *          Call again: returns NULL pointer. fp is unchanged
 * ARGS
 *   filename - a string containing the full path
 *
 * SIDE EFFECTS
 *   The filesystem is traversed, so files are opened and closed, sectors are read, etc...
 * RETURN
 *   A string pointer to the next level in the path, or NULL if this is the end of the path
 */
char* tf_walk(char* filename, TFFile* fp) 
{
    FatFileEntry    entry;
    
    // We're out of path. this walk is COMPLETE
    if (*filename == '/') 
    {
        filename++;
        if (*filename == '\x00') return NULL;
    }
    // There's some path left
    if (*filename != '\x00') 
    {
        // fp is the handle for the current directory
        // filename is the name of the current file in that directory
        // Go fetch the FatFileEntry that corresponds to the current file
        // Remember that tf_find_file is only going to search from the beginning of the filename
        // up until the first path separation character
        if (tf_find_file(fp, filename)) 
        {
            // This happens when we couldn't actually find the file
            fp->flags = 0xff;
            //[DEBUG-tf_walk] Exiting - not found
            return NULL;
        }

        tf_fread((uint8_t*)&entry, sizeof(FatFileEntry), fp);

        // Walk over path separators
        while ((*filename != '/') && (*filename != '\x00'))
        {
            filename += 1;
        }
        if (*filename == '/')
        {
            filename += 1;
        }

        // Set up the file pointer now that we've got information for the next level in the path hierarchy
        fp->parentStartCluster = fp->startCluster;
        fp->startCluster = ((uint32_t)(entry.msdos.eaIndex & 0xffff) << 16) | (entry.msdos.firstCluster & 0xffff);
        fp->attributes = entry.msdos.attributes;
        fp->currentCluster = fp->startCluster;
        fp->currentClusterIdx = 0;
        fp->currentSector = 0;
        fp->currentByte = 0;
        fp->pos = 0;
        fp->flags = TF_FLAG_OPEN;
        fp->size = (entry.msdos.attributes & TF_ATTR_DIRECTORY) ? 0xffffffff : entry.msdos.fileSize;
        if (*filename == '\x00')
        {
            return NULL;
        }
        return filename;
    }
    // We're out of path.  This walk is COMPLETE.
    return NULL;
}


/*
 * Searches the list of system file handles for a free one, and returns it.
 * RETURN
 *   NULL if no system file handles are free, or the free handle if one is available.
 */
TFFile* tf_get_free_handle() 
{
    int         i;
    TFFile*     fp;
    for (i = 0; i < TF_FILE_HANDLES; i++) 
    {
        fp = &tf_file_handles[i];
        if (fp->flags & TF_FLAG_OPEN) continue;
        // We get here if we find a free handle
        fp->flags = TF_FLAG_OPEN;
        return fp;
    }
    return NULL;
}


/*
 * Release a filesystem handle (mark as available)
 */
void tf_release_handle(TFFile* fp) 
{
    fp->flags &= ~TF_FLAG_OPEN;
}


// Convert a character to uppercase
// TODO: Re-do how filename conversions are done.
uint8_t upper(uint8_t c) 
{
    if (c >= 'a' && c <= 'z') 
    {
        return c + ('A' - 'a');
    }
    else 
    {
        return c;
    }
}


//AP - This method looks very iffy 
void tf_choose_sfn(char* dest, char* src, TFFile* fp)
{
    char    temp[13];
    int     results;
    int     num = 1;
    TFFile  xfile;

    // throwaway fp that doesn't muck with the original
    memcpy(&xfile, fp, sizeof(TFFile));

    for (;;)
    {
        results = tf_shorten_filename(dest, src, num);
        switch (results)
        {
        case 0: // ok
            // does the file collide with the current directory?
            //tf_fseek(xfile, 0, 0);
            memcpy(temp, dest, 8);
            memcpy(temp + 9, dest + 8, 3);
            temp[8] = '.';
            temp[12] = 0;

            if (0 > tf_find_file(&xfile, temp))
            {
                return;
            }

            num++;
            return;

        case -1: // error
            //[DEBUG-tf_choose_sfn] error selecting short filename!
            return;
        }
    }
}

/*
 * Take the long filename (filename only, not full path) specified by src,
 * and convert it to an 8.3 compatible filename, storing the result at dst
 * TODO: This should return something, an error code for conversion failure.
 * TODO: This should handle special chars etc.
 * TODO: Test for short filenames, (<7 characters)
 * TODO: Modify this to use the basis name generation algorithm described in the FAT32 whitepaper.
 */
int tf_shorten_filename(char* dest, char* src, char num) 
{
    int     l = strlen(src);
    int     i;
    int     lossy_flag = 0;
    char*   tmp;

    // strip through and chop special chars

    tmp = strrchr(src, '.');
    // copy the extension
    for (i = 0; i < 3; i++)
    {
        while (tmp != 0 &&
              *tmp != 0 &&
              !(*tmp < 0x7f &&
                *tmp > 20 &&  //AP - This looks iffy.
                *tmp != 0x22 &&
                *tmp != 0x2a &&
                *tmp != 0x2e &&
                *tmp != 0x2f &&
                *tmp != 0x3a &&
                *tmp != 0x3c &&
                *tmp != 0x3e &&
                *tmp != 0x3f &&
                *tmp != 0x5b &&
                *tmp != 0x5c &&
                *tmp != 0x5d &&
                *tmp != 0x7c))
        {
            tmp++;
        }

        if (tmp == 0 || *tmp == 0)
        {
            *(dest + 8 + i) = ' ';
        }
        else
        {
            *(dest + 8 + i) = upper(*(tmp++));
        }
    }

    // Copy the basename
    i = 0;
    tmp = strrchr(src, '.');
    for (;;)
    {
        if (i == 8)
        {
            break;
        }

        if (src == tmp)
        {
            dest[i++] = ' ';
            continue;
        }

        if ((*dest == ' ')) 
        { 
            lossy_flag = 1; 
        }
        else 
        {
            while (*src != 0 &&
                  !(*src < 0x7f &&
                    *src > 20 &&  //AP - This looks iffy.
                    *src != 0x22 &&
                    *src != 0x2a &&
                    *src != 0x2e &&
                    *src != 0x2f &&
                    *src != 0x3a &&
                    *src != 0x3c &&
                    *src != 0x3e &&
                    *src != 0x3f &&
                    *src != 0x5b &&
                    *src != 0x5c &&
                    *src != 0x5d &&
                    *src != 0x7c))
            {
                src++;
            }

            if (*src == 0)
            {
                dest[i] = ' ';
            }
            else if (*src == ',' || 
                     *src == '[' || 
                     *src == ']')
            {
                dest[i] = '_';
            }
            else
            {
                dest[i] = upper(*(src++));
            }
        }

        i++;
    }

    // now that they are populated, do analysis.
    // if num>4, do 2 letters
    if (num > 4)
    {
        snprintf(dest + 2, 6, "%.4X~", num);
        dest[7] = '1';
    }
    else
    {
        tmp = strchr(dest, ' ');
        //printf("\r\n=-=-=- tf_shorten_filename:  %x - %x = %x",
        //       tmp, dest, tmp-dest);
        if (tmp == 0 || tmp - dest > 6)
        {
            dest[6] = '~';
            dest[7] = num + 0x30;
        }
        else
        {
            *tmp++ = '~';
            *tmp++ = num + 0x30;
        }
    }

    /*
    // Copy the basename
    while(1) {
        if(i==8) break;
        if((i==6) || (*src == '.') || (*src == '\x00'))break;
        if((*dest == ' '))  {lossy_flag = 1; } else {
            *(dest++) = upper(*(src++));
        }
        i+=1;
    }
    // Funny tail if basename was too long
    if(i==6) {
        *(dest++) = '~';
        *(dest++) = num+0x30;        // really? hard coded? wow. FIXME: make check filesystem
        i+=2;
    }
    // Or Not
    else {
        while(i<8) {
            *(dest++) = ' ';
            i++;
        }
    }
    // Last . in the filename
    src = strrchr(src, '.');

    *(dest++) = ' ';
    *(dest++) = ' ';
    *(dest++) = ' ';
    dest -= 3;
    //*(dest++) = '\x00';   // this field really *is* 11 bytes long, no terminating NULL necessary.
    //dest -= 4;            // and thank you to not since it clobbers the next byte (.attributes)
    if(src != NULL) {
        src +=1;
        while(i < 11) {     // this field really *is* 11 bytes long, no terminating NULL necessary.
            if(*src == '\x00') break;
        *(dest++) = upper(*(src++));
        i+=1;
        }
    }*/
    return TF_ERR_NO_ERROR;
}

/*
 * Create a LFN entry from the filename provided.
 * - The entry is constructed from all, or the first 13 characters in the filename (whichever is smaller)
 * - If filename is <=13 bytes long, the NULL pointer is returned
 * - If the filename >13 bytes long, an entry is constructed for the first 13 bytes, and a pointer is
 *   returned to the remainder of the filename.
 * ARGS
 *   filename - string containing the filename for which to create an entry
 *   entry - pointer to a FatFileEntry structure, which is populated as an LFN entry
 * RETURN
 *   NULL if this is the last entry for this file, or a string pointer to the remainder of the string
 *   if the entire filename won't fit in one entry
 * WARNING
 *   Because this function works in forward order, and LFN entries are stored in reverse, it does
 *   NOT COMPUTE LFN SEQUENCE NUMBERS.  You have to do that on your own.  Also, because the function
 *   acts on partial filenames IT DOES NOT COMPUTE LFN CHECKSUMS.  You also have to do that on your own.
 * TODO
 *   Test and further improve on this function
 */
char* tf_create_lfn_entry(char* filename, FatFileEntry* entry) 
{
    int i, done = 0;

    for (i = 0; i < 5; i++) 
    {
        if (!done)
        {
            entry->lfn.name1[i] = (unsigned short)*(filename);
        }
        else
        {
            entry->lfn.name1[i] = 0xffff;
        }
        if (*filename++ == '\x00')
        {
            done = 1;
        }
    }

    for (i = 0; i < 6; i++) 
    {
        if (!done)
        {
            entry->lfn.name2[i] = (unsigned short)*(filename);
        }
        else
        {
            entry->lfn.name2[i] = 0xffff;
        }
        if (*filename++ == '\x00')
        {
            done = 1;
        }
    }

    for (i = 0; i < 2; i++) 
    {
        if (!done)
        {
            entry->lfn.name3[i] = (unsigned short)*(filename);
        }
        else
        {
            entry->lfn.name3[i] = 0xffff;
        }
        if (*filename++ == '\x00')
        {
            done = 1;
        }
    }

    entry->lfn.attributes = 0x0f;
    entry->lfn.reserved = 0;
    entry->lfn.firstCluster = 0;
    if (done)
    {
        return NULL;
    }

    if (*filename)
    {
        return filename;
    }
    else
    {
        return NULL;
    }
}


// Taken from http://en.wikipedia.org/wiki/File_Allocation_Table
//
uint8_t tf_lfn_checksum(const char* pFcbName)
{
    int     i;
    uint8_t sum = 0;

    for (i = 11; i; i--)
    {
        sum = ((sum & 1) << 7) + (sum >> 1) + *pFcbName++;
    }
    return sum;
}


int tf_place_lfn_chain(TFFile* fp, char* filename, char* sfn) 
{
    // Windows does reverse chaining:  0x44, 0x03, 0x02, 0x01
    char*           strptr = filename;
    int             entries = 1;
    int             i;
    char*           last_strptr = filename;
    FatFileEntry    entry;
    uint8_t         seq;
    //uint8_t sfn[12];

    //tf_choose_sfn(sfn, filename, fp);
    //tf_shorten_filename(sfn, filename, 1);
    //sfn[11] = 0;     // tf_shorten_filename no longer does this...

    // create the chains - probably only to get a count!?
    // FIXME: just pre-calculate and don't do all this recomputing!
    while (strptr = tf_create_lfn_entry(strptr, &entry)) 
    {
        last_strptr = strptr;
        entries += 1;
    }

    // LFN sequence number (first byte of LFN)
    seq = entries | 0x40;
    for (i = 0; i < entries; i++) 
    {
        tf_create_lfn_entry(last_strptr, &entry);
        entry.lfn.sequence_number = seq;
        entry.lfn.checksum = tf_lfn_checksum(sfn);

        tf_fwrite((uint8_t*)&entry, sizeof(FatFileEntry), 1, fp);
        seq = ((seq & ~0x40) - 1);
        last_strptr -= 13;
    }
    return TF_ERR_NO_ERROR;
}


int tf_create(char* filename) 
{
    TFFile*         fp = tf_parent(filename, "r", false);
    FatFileEntry    entry;
    uint32_t        cluster;
    char*           temp;

    if (!fp)
    {
        return 1;
    }
        
    tf_fclose(fp);
    fp = tf_parent(filename, "r+", false);
    // Now we have the directory in which we want to create the file, open for overwrite
    do 
    {
        //"seek" to the end
        tf_fread((uint8_t*)&entry, sizeof(FatFileEntry), fp);
        //Skipping existing directory entry... 
    } 
    while (entry.msdos.filename[0] != '\x00');

    // Back up one entry, this is where we put the new filename entry
    tf_fseek(fp, -((int32_t)sizeof(FatFileEntry)), fp->pos);
    cluster = tf_find_free_cluster();
    tf_set_fat_entry(cluster, TF_MARK_EOC32); // Marks the new cluster as the last one (but no longer free)
    
    entry.msdos.attributes = 0;
    entry.msdos.creationTimeMs = 0x25;
    entry.msdos.creationTime = 0x7e3c;
    entry.msdos.creationDate = 0x4262;
    entry.msdos.lastAccessTime = 0x4262;
    entry.msdos.eaIndex = (cluster >> 16) & 0xffff;
    entry.msdos.modifiedTime = 0x7e3c;
    entry.msdos.modifiedDate = 0x4262;
    entry.msdos.firstCluster = cluster & 0xffff;
    entry.msdos.fileSize = 0;

    temp = strrchr(filename, '/') + 1;
   
    tf_choose_sfn(entry.msdos.filename, temp, fp);

    tf_place_lfn_chain(fp, temp, entry.msdos.filename);
    //tf_choose_sfn(entry.msdos.filename, temp, fp);
    //tf_shorten_filename(entry.msdos.filename, temp);
    //printf("\r\n==== tf_create: SFN: %s", entry.msdos.filename);
    tf_fwrite((uint8_t*)&entry, sizeof(FatFileEntry), 1, fp);
    memset(&entry, 0, sizeof(FatFileEntry));
    //entry.msdos.filename[0] = '\x00';
    tf_fwrite((uint8_t*)&entry, sizeof(FatFileEntry), 1, fp);
    tf_fclose(fp);

    return TF_ERR_NO_ERROR;
}

/* tf_mkdir attempts to create a new directory in the filesystem.  duplicates
are *not* allowed!
returns 1 on failure
returns 0 on success
*/
int tf_mkdir(char* filename, int mkParents) 
{
    // FIXME: verify that we can't create multiples of the same one.
    // FIXME: figure out how the root directory location is determined.
    char orig_fn[TF_MAX_PATH];
    TFFile* fp;
    FatFileEntry entry, blank;

    uint32_t psc;
    uint32_t cluster;
    char* temp;

    strncpy(orig_fn, filename, TF_MAX_PATH - 1);
    orig_fn[TF_MAX_PATH - 1] = 0;

    memset(&blank, 0, sizeof(FatFileEntry));

    fp = tf_fopen(filename, "r");
    if (fp)  // if not NULL, the filename already exists.
    {
        tf_fclose(fp);
        tf_release_handle(fp);
        if (mkParents)
        {
            return TF_ERR_NO_ERROR;
        }
        return 1;
    }

    fp = tf_parent(filename, "r+", mkParents);
    if (!fp)
    {
        return 1;
    }

    // Now we have the directory in which we want to create the file, open for overwrite
    do 
    {
        tf_fread((uint8_t*)&entry, sizeof(FatFileEntry), fp);
    } 
    while (entry.msdos.filename[0] != '\x00');
    // Back up one entry, this is where we put the new filename entry
    tf_fseek(fp, -((int32_t)sizeof(FatFileEntry)), fp->pos);

    // go find some space for our new friend
    cluster = tf_find_free_cluster();
    tf_set_fat_entry(cluster, TF_MARK_EOC32); // Marks the new cluster as the last one (but no longer free)

    // set up our new directory entry
    // TODO shorten these entries with memset
    entry.msdos.attributes = TF_ATTR_DIRECTORY;
    entry.msdos.creationTimeMs = 0x25;
    entry.msdos.creationTime = 0x7e3c;
    entry.msdos.creationDate = 0x4262;
    entry.msdos.lastAccessTime = 0x4262;
    entry.msdos.eaIndex = (cluster >> 16) & 0xffff;
    entry.msdos.modifiedTime = 0x7e3c;
    entry.msdos.modifiedDate = 0x4262;
    entry.msdos.firstCluster = cluster & 0xffff;
    entry.msdos.fileSize = 0;
    temp = strrchr(filename, '/') + 1;

    tf_choose_sfn(entry.msdos.filename, temp, fp);

    tf_place_lfn_chain(fp, temp, entry.msdos.filename);
    //tf_choose_sfn(entry.msdos.filename, temp, fp);
    //tf_shorten_filename(entry.msdos.filename, temp, 1);
    //printf("\r\n==== tf_mkdir: SFN: %s", entry.msdos.filename);
    //    dbg_printf("  3 mkdir: attr: %x ", entry.msdos.attributes); // attribute byte still getting whacked.
    //entry.msdos.attributes = TF_ATTR_DIRECTORY ;
    //    dbg_printf("  4 mkdir: attr: %x ", entry.msdos.attributes);
    tf_fwrite((uint8_t*)&entry, sizeof(FatFileEntry), 1, fp);

    psc = fp->startCluster; // store this for later

    // placing a 0 at the end of the FAT
    tf_fwrite((uint8_t*)&blank, sizeof(FatFileEntry), 1, fp);
    tf_fclose(fp);
    tf_release_handle(fp);

    fp = tf_fopen(orig_fn, "w");

    // set up .
    memcpy(entry.msdos.filename, ".          ", 11);
    //entry.msdos.attributes = TF_ATTR_DIRECTORY;
    //entry.msdos.firstCluster = cluster & 0xffff    
    tf_fwrite((uint8_t*)&entry, sizeof(FatFileEntry), 1, fp);

    // set up ..
    memcpy(entry.msdos.filename, "..         ", 11);
    //entry.msdos.attributes = TF_ATTR_DIRECTORY;
    //entry.msdos.firstCluster = cluster & 0xffff
    tf_fwrite((uint8_t*)&entry, sizeof(FatFileEntry), 1, fp);

    // placing a 0 at the end of the FAT
    tf_fwrite((uint8_t*)&blank, sizeof(FatFileEntry), 1, fp);

    tf_fclose(fp);
    tf_release_handle(fp);
    return TF_ERR_NO_ERROR;
}


//**** COMPLETE UNTESTED tf_listdir ****//
// this may be better served by simply opening the directory directly through C2
// parsing is not a huge deal...
// returns 1 when valid entry, 0 when done.
int tf_listdir(char* filename, FatFileEntry* entry, TFFile** fp) 
{
    // May Require a terminating "/."
    // FIXME: What do we do with the results?!  perhaps take in a fp and assume
    //  that if not NULL, it's already in the middle of a listdir!  if NULL
    //  we do the tf_parent thing and then start over.  if not, we return the 
    //  next FatFileEntry almost like a callback...  and return NULL when we've
    //  reached the end.  almost like.. gulp... strtok.  ugh.  maybe not.  
    //  still, it may suck less than other things... i mean, by nature, listdir 
    //  is inherently dynamic in size, and we have no re/malloc.

    if (*fp == NULL)
    {
        *fp = tf_parent(filename, "r", false);
    }

    if (!*fp) 
    { 
        return 1; 
    }

    // do magic here.
    for (;;)
    {
        tf_fread((uint8_t*)entry, sizeof(FatFileEntry), *fp);
        switch (((uint8_t*)entry)[0]) {
        case 0x05:
            // pending delete files under some implementations.  ignore it.
            break;
        case 0xe5:
            // freespace (deleted file)
            break;
        case 0x2e:
            // '.' or '..'
            break;
        case 0x00:
            // no further entries exist, and this one is available
            tf_fclose(*fp);
            *fp = NULL;
            return TF_ERR_NO_ERROR;

        default:
            return 1;
        }
    }

    return TF_ERR_NO_ERROR;
}


TFFile* tf_fopen(char* filename, const char* mode) 
{
    TFFile* fp;

    fp = tf_fnopen(filename, mode, strlen(filename));
    if (fp == NULL) 
    {
        if (strchr(mode, '+') || strchr(mode, 'w') || strchr(mode, 'a')) 
        {
            tf_create(filename);
        }
        return tf_fnopen(filename, mode, strlen(filename));
    }
    return fp;
}


//
// Just like fopen, but only look at n uint8_tacters of the path
TFFile* tf_fnopen(char* filename, const char* mode, int n) 
{
    // Request a new file handle from the system
    TFFile*     fp = tf_get_free_handle();
    char        myfile[256];
    char*       temp_filename = myfile;
    uint32_t    cluster;

    if (fp == NULL)
    {
        return (TFFile*)-1;
    }

    strncpy(myfile, filename, n);
    myfile[n] = 0;
    fp->currentCluster = 2;           // FIXME: this is likely supposed to be the first cluster of the Root directory...
                                    // however, this is set in the BPB...
    fp->startCluster = 2;
    fp->parentStartCluster = 0xffffffff;
    fp->currentClusterIdx = 0;
    fp->currentSector = 0;
    fp->currentByte = 0;
    fp->attributes = TF_ATTR_DIRECTORY;
    fp->pos = 0;
    fp->flags |= TF_FLAG_ROOT;
    fp->size = 0xffffffff;
    //fp->size=tf_info.rootDirectorySize;
    fp->mode = TF_MODE_READ | TF_MODE_WRITE | TF_MODE_OVERWRITE;

    while (temp_filename != NULL) 
    {
        temp_filename = tf_walk(temp_filename, fp);
        if (fp->flags == 0xff) 
        {
            tf_release_handle(fp);
            return NULL;
        }
    }

    if (strchr(mode, 'r')) 
    {
        fp->mode |= TF_MODE_READ;
    }

    if (strchr(mode, 'a')) 
    {
        tf_unsafe_fseek(fp, fp->size, 0);
        fp->mode |= TF_MODE_WRITE | TF_MODE_OVERWRITE;
    }

    if (strchr(mode, '+'))
    {
        fp->mode |= TF_MODE_OVERWRITE | TF_MODE_WRITE;
    }
    
    if (strchr(mode, 'w')) 
    {
        /* Opened for writing. Truncate file only if it's not a directory*/
        if (!(fp->attributes & TF_ATTR_DIRECTORY)) 
        {
            fp->size = 0;
            tf_unsafe_fseek(fp, 0, 0);
            /* Free the clusterchain starting with the second one if the file
             * uses more than one */
            if ((cluster = tf_get_fat_entry(fp->startCluster)) != TF_MARK_EOC32) 
            {
                tf_free_clusterchain(cluster);
                tf_set_fat_entry(fp->startCluster, TF_MARK_EOC32);
            }
        }
        fp->mode |= TF_MODE_WRITE;
    }

    strncpy(fp->filename, myfile, n);

    fp->filename[n] = 0;
    return fp;
}

int tf_free_clusterchain(uint32_t cluster) 
{
    uint32_t fat_entry;
    
    fat_entry = tf_get_fat_entry(cluster);
    while (fat_entry < TF_MARK_EOC32) 
    {
        if (fat_entry <= 2)        // catch-all to save root directory from corrupted stuff
        {
            //SOMETHING WICKED THIS WAY COMES!  End of FAT cluster chain is <=2 (end should be 0x0ffffff8)
            break;
        }
        
        tf_set_fat_entry(cluster, 0x00000000);
        fat_entry = tf_get_fat_entry(fat_entry);
        cluster = fat_entry;
    }
    return TF_ERR_NO_ERROR;
}



int tf_fseek(TFFile* fp, int32_t base, long offset) 
{
    uint64_t pos = base + offset;
    if (pos >= fp->size)
    {
        return TF_ERR_INVALID_SEEK;
    }
    return tf_unsafe_fseek(fp, base, offset);
}

/*
 * TODO: Make it so seek fails aren't destructive to the file handle
 */
int tf_unsafe_fseek(TFFile* fp, int32_t base, long offset) 
{
    uint32_t cluster_idx;
    uint64_t pos = base + offset;
    uint32_t mark = tf_info.type ? TF_MARK_EOC32 : TF_MARK_EOC16;
    uint32_t temp;

    // We're only allowed to seek one past the end of the file (For writing new stuff)
    if (pos > fp->size) 
    {
        //[DEBUG-tf_unsafe_fseek] SEEK ERROR
        return TF_ERR_INVALID_SEEK;
    }
    if (pos == fp->size) 
    {
        fp->size += 1;
        fp->flags |= TF_FLAG_SIZECHANGED;
    }

    // Compute the cluster index of the new location
    cluster_idx = (uint32_t)(pos / (tf_info.sectorsPerCluster * DRIVE_SECTOR_SIZE)); // The cluster we want in the file

    // If the cluster index matches the index we're already at, we don't need to look in the FAT
    // If it doesn't match, we have to follow the linked list to arrive at the correct cluster 
    if (cluster_idx != fp->currentClusterIdx) 
    {
        temp = cluster_idx;
        /* Shortcut: If we are looking for a cluster that comes *after* the current we don't
         * need to start at the beginning */
        if (cluster_idx > fp->currentClusterIdx) 
        {
            cluster_idx -= fp->currentClusterIdx;
        }
        else 
        {
            fp->currentCluster = fp->startCluster;
        }
        fp->currentClusterIdx = temp;
        while (cluster_idx > 0) 
        {
            // TODO Check file mode here for r/w/a/etc...
            temp = tf_get_fat_entry(fp->currentCluster); // next, next, next
            if ((temp & 0x0fffffff) != mark)
            {
                fp->currentCluster = temp;
            }
            else 
            {
                // We've reached the last cluster in the file (omg)
                // If the file is writable, we have to allocate new space
                // If the file isn't, our job is easy, just report an error
                // Also, probably report an error if we're out of space
                temp = tf_find_free_cluster_from(fp->currentCluster);
                tf_set_fat_entry(fp->currentCluster, temp); // Allocates new space
                tf_set_fat_entry(temp, mark); // Marks the new cluster as the last one
                fp->currentCluster = temp;
            }
            cluster_idx--;

            if (fp->currentCluster >= mark) 
            {
                if (cluster_idx > 0) 
                {
                    return TF_ERR_INVALID_SEEK;
                }
            }
        }
        // We now have the correct cluster number (whether we had to fetch it from the fat, or realized we already had it)
        // Now we need just compute the correct sector and byte index into the cluster
    }
    fp->currentByte = pos % (tf_info.sectorsPerCluster * DRIVE_SECTOR_SIZE); // The offset into the cluster
    fp->pos = (uint32_t)pos;

    return TF_ERR_NO_ERROR;
}

/*
 * Given a file handle to the current directory and a filename, populate the provided FatFileEntry with the
 * file information for the given file.
 * SIDE EFFECT: the position in current_directory will be set to the beginning of the fatfile entry (for overwriting purposes)
 * returns 0 on match, -1 on fail
 */
int tf_find_file(TFFile* current_directory, char* name) 
{
    int rc;
    tf_fseek(current_directory, 0, 0);

    for (;;)
    {
        rc = tf_compare_filename(current_directory, name);
        if (rc < 0)
        {
            break;
        }
        else if (rc == 1)    // found!
        {
            return TF_ERR_NO_ERROR;
        }
    }
    return -1;
}


/*! tf_compare_filename_segment compares a given filename against a particular
FatFileEntry (a 32-byte structure pulled off disk, all of these are back-to-back
in a typical Directory entry on the disk)
figures out formatted name, does comparison, and returns 0:failure, 1:match
*/
int tf_compare_filename_segment(FatFileEntry* entry, char* name) 
{
    int i, j;
    char reformatted_file[16];
    char* entryname = entry->msdos.filename;
    
    if (entry->msdos.attributes != 0x0f) 
    {
        // Filename
        j = 0;
        for (i = 0; i < 8; i++) 
        {
            if (entryname[i] != ' ') 
            {
                reformatted_file[j++] = entryname[i];
            }
        }
        reformatted_file[j++] = '.';
        // Extension
        for (i = 8; i < 11; i++)
        {
            if (entryname[i] != ' ')
            {
                reformatted_file[j++] = entryname[i];
            }
        }
    }
    else
    {
        j = 0;
        for (i = 0; i < 5; i++)
        {
            reformatted_file[j++] = (uint8_t)entry->lfn.name1[i];
        }
        for (i = 0; i < 6; i++)
        {
            reformatted_file[j++] = (uint8_t)entry->lfn.name2[i];
        }
        for (i = 0; i < 2; i++)
        {
            reformatted_file[j++] = (uint8_t)entry->lfn.name3[i];
        }
    }

    reformatted_file[j++] = '\x00';
    i = 0;
    while ((name[i] != '/') && (name[i] != '\x00'))
    {
        i++; // will this work for 8.3?  this should be calculated in the section with knowledge of lfn/8.3
    }

    // FIXME: only compare the 13 or less bytes left in the reformatted_file string... but that doesn't match all the way to the end of the test string....


    // the role of 'i' changes here to become the return value.  perhaps this doesn't gain us enough in performance to avoid using a real retval?
    /// PROBLEM: THIS FUNCTION assumes that if the length of the "name" is tested by the caller.
    ///   if the LFN pieces all match, but the "name" is longer... this will never fail.
    if (i > 13)
    {
        if (_strnicmp(name, reformatted_file, 13))
        {
            // 0 (doesn't match)
            i = 0;
        }
        else
        {
            // 1 (match)
            i = 1;
        }
    }
    else
    {
        if (reformatted_file[i] != 0 || _strnicmp(name, reformatted_file, i))
        {
            i = 0;
            //0 (doesn't match)
        }
        else
        {
            i = 1;
            //1 (match)
        }
    }
    return i;
}


// 
// Reads a single FatFileEntry from fp, compares it to the MSDOS filename specified by *name
// Returns:
//   1 for entry matches filename.  Side effect: fp seeks to that entry
//   0 for entry doesn't match filename.  Side effect: fp seeks to next entry
//   -1 for couldn't read an entry, due to EOF or other fread error
//
int tf_compare_filename(TFFile* fp, char* name) 
{
    uint32_t        i;
    uint32_t        j = 0;
    uint32_t        namelen;
    FatFileEntry    entry;
    char*           compare_name = name;
    uint32_t        lfn_entries;

    // Read a single directory entry
    tf_fread((uint8_t*)&entry, sizeof(FatFileEntry), fp);

    // Fail if its bogus
    if (entry.msdos.filename[0] == 0x00) return -1;

    // If it's a DOS entry, then:
    if (entry.msdos.attributes != 0x0f) 
    {
        // If it's a match, seek back an entry to the beginning of it, return 1
        if (1 == tf_compare_filename_segment(&entry, name)) 
        { //, true)) 
            tf_fseek(fp, -((int32_t)sizeof(FatFileEntry)), fp->pos);
            //[DEBUG-tf_compare_filename] 8.3 Exiting... returning 1 (match)
            return 1;
        }
        else 
        {
            //[DEBUG-tf_compare_filename] 8.3 Exiting... returning 0 (doesn't match)
            return TF_ERR_NO_ERROR;
        }
    }
    else if ((entry.lfn.sequence_number & 0xc0) == 0x40)
    {
        //CHECK FOR 0x40 bit set or this is not the first (last) LFN entry!
        // If this is the first LFN entry, mask off the extra bit (0x40) and you get the number of entries in the chain
        lfn_entries = entry.lfn.sequence_number & ~0x40;

        // Seek to the last entry in the chain (LFN entries store name in reverse, so the last shall be first)
        tf_fseek(fp, (int32_t)sizeof(FatFileEntry) * (lfn_entries - 1), fp->pos);

        // get the length of the file first off.  LFN count should be easily checked from here.
        namelen = strlen(name);
        if (((namelen + 12) / LFN_ENTRY_CAPACITY) != lfn_entries)
        {
            // skip this LFN, it isn't it.
            //  not necessary, we're already there.  // tf_fseek(fp, (int32_t)((i))*sizeof(FatFileEntry), fp->pos);
            //[DEBUG-tf_compare_filename] LFN Exiting... returning 0  (no match)
            return TF_ERR_NO_ERROR;
        }

        for (i = 0; i < lfn_entries; i++) 
        {
            // Seek back one and read it
            tf_fseek(fp, -((int32_t)sizeof(FatFileEntry)), fp->pos);
            tf_fread((uint8_t*)&entry, sizeof(FatFileEntry), fp);

            // Compare it.  If it's not a match, jump to the end of the chain, return failure 
            // Otherwise, continue looping until there's no entries left.
            if (!tf_compare_filename_segment(&entry, compare_name)) 
            {
                tf_fseek(fp, (int32_t)((i)) * sizeof(FatFileEntry), fp->pos);
                //[DEBUG-tf_compare_filename] LFN Exiting... returning 0  (no match)
                return TF_ERR_NO_ERROR;
            }

            tf_fseek(fp, -((int32_t)sizeof(FatFileEntry)), fp->pos);

            compare_name += 13;
        }

        // If we made it here, match was a success!  Return so... 
        // ONLY if next entry is valid!
        tf_fseek(fp, (int32_t)sizeof(FatFileEntry) * lfn_entries, fp->pos);
        //[DEBUG-tf_compare_filename] LFN Exiting... returning 1
        return 1;
    }
    //[DEBUG-tf_compare_filename] (---) Exiting... returning -1
    return -1;
}


int tf_fread(uint8_t* dest, int size, TFFile* fp) 
{
    uint32_t sector;
    while (size > 0) 
    {
        sector = tf_first_sector(fp->currentCluster) + (fp->currentByte / DRIVE_SECTOR_SIZE);
        tf_fetch(sector);       // wtfo?  i know this is cached, but why!?
        
        *dest++ = tf_info.buffer[fp->currentByte % DRIVE_SECTOR_SIZE];
        size--;

        if (fp->attributes & TF_ATTR_DIRECTORY) 
        {
            //dbg_printf("READING DIRECTORY");
            if (tf_fseek(fp, 0, fp->pos + 1)) 
            {
                return -1;
            }
        }
        else 
        {
            if (tf_fseek(fp, 0, fp->pos + 1)) 
            {
                return -1;
            }
        }
    }
    return TF_ERR_NO_ERROR;
}


int tf_fwrite(uint8_t* src, int size, int count, TFFile* fp)
{
    int i, tracking, segsize;

    fp->flags |= TF_FLAG_DIRTY;
    while (count > 0)
    {
        i = size;
        fp->flags |= TF_FLAG_SIZECHANGED;
        while (i > 0)
        {  
            // FIXME: even this new algorithm could be more efficient by elegantly combining count/size
            tf_fetch(tf_first_sector(fp->currentCluster) + (fp->currentByte / DRIVE_SECTOR_SIZE));
            tracking = fp->currentByte % DRIVE_SECTOR_SIZE;
            segsize = (i < DRIVE_SECTOR_SIZE ? i : DRIVE_SECTOR_SIZE);

            memcpy(&tf_info.buffer[tracking], src, segsize);
            tf_info.sectorFlags |= TF_FLAG_DIRTY; // Mark this sector as dirty

            if (fp->pos + segsize > fp->size)
            {
                fp->size += segsize - (fp->pos % DRIVE_SECTOR_SIZE);
            }

            if (tf_unsafe_fseek(fp, 0, fp->pos + segsize)) {
                return -1;
            }

            i -= segsize;
            src += segsize;
        }
        count--;
    }
    return size - i;
}

int tf_fputs(char* src, TFFile* fp) 
{
    return tf_fwrite((uint8_t*)src, 1, strlen(src), fp);
}


int tf_fclose(TFFile* fp) 
{
    int rc;

    rc = tf_fflush(fp);
    fp->flags &= ~TF_FLAG_OPEN; // Mark the file as available for the system to use
    // FIXME: is there any reason not to release the handle here?
    return rc;
}


/* tf_parent attempts to open the parent directory of whatever file you request
returns basically a fp the tf_fnopen returns
*/
TFFile* tf_parent(char* filename, const char* mode, int mkParents) 
{
    TFFile*     retval;
    const char* f2;

    f2 = strrchr((char const*)filename, '/');
    retval = tf_fnopen(filename, "rw", (int)(f2 - filename));
    // if retval == NULL, why!?  we could be out of handles
    if (retval == NULL && mkParents)
    {   // warning: recursion could fry some resources on smaller procs
        char tmpbuf[260];
        if (f2 - filename > 260)
        {
            return NULL;
        }
        strncpy(tmpbuf, filename, f2 - filename);
        tmpbuf[f2 - filename] = 0;

        tf_mkdir(tmpbuf, mkParents);
        retval = tf_parent(filename, mode, mkParents);
    }
    else if (retval == (void*)-1)
    {
        //[DEBUG-tf_parent] uh oh.  tf_fopen() return -1, out of handles?
    }

    return retval;
}


int tf_fflush(TFFile* fp) 
{
    int rc = 0;
    TFFile* dir;
    FatFileEntry entry;
    char* filename = entry.msdos.filename;

    if (!(fp->flags & TF_FLAG_DIRTY))
    {
        return TF_ERR_NO_ERROR;
    }

    // First write any pending data to disk
    if (tf_info.sectorFlags & TF_FLAG_DIRTY) 
    {
        rc = tf_store();
    }
    // Now go modify the directory entry for this file to reflect changes in the file's size
    // (If they occurred)
    if (fp->flags & TF_FLAG_SIZECHANGED)
    {

        if (fp->attributes & 0x10)
        {
            // TODO Deal with changes in the root directory size here
        }
        else
        {
            // Open the parent directory
            dir = tf_parent(fp->filename, "r+", false);
            if (dir == (void*)-1)
            {
                //[DEBUG-tf_fflush] FAILED to get parent!
                return -1;
            }

            filename = (char*)strrchr((char const*)fp->filename, '/');

            // Seek to the entry we want to modify and pull it from disk
            tf_find_file(dir, filename + 1);
            tf_fread((uint8_t*)&entry, sizeof(FatFileEntry), dir);
            tf_fseek(dir, -((int32_t)sizeof(FatFileEntry)), dir->pos);

            // Modify the entry in place to reflect the new file size
            entry.msdos.fileSize = fp->size - 1;
            tf_fwrite((uint8_t*)&entry, sizeof(FatFileEntry), 1, dir); // Write fatfile entry back to disk
            tf_fclose(dir);
        }
        fp->flags &= ~TF_FLAG_SIZECHANGED;
    }

    fp->flags &= ~TF_FLAG_DIRTY;
    return rc;
}


/*
 * Remove a file from the filesystem
 * @param filename - The full path of the file to be removed
 * @return
 */
int tf_remove(char* filename)
{
    TFFile*         fp;
    FatFileEntry    entry;
    int             rc;
    uint32_t        startCluster;

    // Sanity check
    fp = tf_fopen(filename, "r");
    if (fp == NULL)
    {
        return -1; // return an error if we're removing a file that doesn't exist
    }

    startCluster = fp->startCluster; // Remember first cluster of the file so we can remove the clusterchain
    tf_fclose(fp);

    // TODO Don't leave an orphaned LFN
    fp = tf_parent(filename, "r+", false);
    rc = tf_find_file(fp, (strrchr(filename, '/') + 1));
    if (!rc) 
    {
        for (;;)
        {
            rc = tf_fseek(fp, sizeof(FatFileEntry), fp->pos);
            if (rc) break;
            tf_fread((uint8_t*)&entry, sizeof(FatFileEntry), fp); // Read one entry ahead
            tf_fseek(fp, -((int32_t)(2 * sizeof(FatFileEntry))), fp->pos);
            tf_fwrite((uint8_t*)&entry, sizeof(FatFileEntry), 1, fp);
            if (entry.msdos.filename[0] == 0) break;
        }
        fp->size -= sizeof(FatFileEntry);
        fp->flags |= TF_FLAG_SIZECHANGED;
    }

    tf_fclose(fp);
    tf_free_clusterchain(startCluster); // Free the data associated with the file

    return TF_ERR_NO_ERROR;
}


// Walk the FAT from the very first data sector and find a cluster that's available
// Return the cluster index 
// TODO: Rewrite this function so that you can start finding a free cluster at somewhere other than the beginning
uint32_t tf_find_free_cluster() 
{
    uint32_t    i;
    uint32_t    entry;
    uint32_t    totalClusters;

    totalClusters = tf_info.totalSectors / tf_info.sectorsPerCluster;
    for (i = 0; i < totalClusters; i++) 
    {
        entry = tf_get_fat_entry(i);
        if ((entry & 0x0fffffff) == 0) break;

    }
    return i;
}


/* Optimize search for a free cluster */
uint32_t tf_find_free_cluster_from(uint32_t c)
{
    uint32_t i, entry, totalClusters;

    totalClusters = tf_info.totalSectors / tf_info.sectorsPerCluster;
    for (i = c; i < totalClusters; i++)
    {
        entry = tf_get_fat_entry(i);
        if ((entry & 0x0fffffff) == 0) break;
    }

    /* We couldn't find anything here so search from the beginning */
    if (i == totalClusters)
    {
        return tf_find_free_cluster();
    }

    return i;
}

/* Initialize the FileSystem metadata on the media (yes, the "FORMAT" command
    that Windows doesn't allow for large volumes */
uint32_t tf_initializeMedia(uint32_t totalSectors)       // this should take in some lun number to make this all good...   but we'll do that when we get read/write_sector lun-aware.  Also, hardcoded sector configuration.
{
    gcDrive.Init(totalSectors * DRIVE_SECTOR_SIZE);

    uint8_t sectorBuf0[DRIVE_SECTOR_SIZE];
    uint8_t sectorBuf[DRIVE_SECTOR_SIZE];
    BPB_struct bpb; // = (BPB_struct*)sectorBuf0;
    uint32_t scl, ssa, fat;

    memset(sectorBuf0, 0x00, 0x200);
    memset(&bpb, 0, sizeof(bpb));

    // jump instruction
    bpb.BS_JumpBoot[0] = 0xEB;
    bpb.BS_JumpBoot[1] = 0x58;
    bpb.BS_JumpBoot[2] = 0x90;

    // OEM name
    memcpy(bpb.BS_OEMName, " mkdosfs", 8);

    // BPB
    bpb.BytesPerSector = 0x200;        // hard coded, must be a define somewhere
    bpb.SectorsPerCluster = 32;        // this may change based on drive size
    bpb.ReservedSectorCount = 32;
    bpb.NumFATs = 2;
    //bpb.RootEntryCount = 0;
    //bpb.TotalSectors16 = 0;
    bpb.Media = 0xf8;
    //bpb.FATSize16 = 0;
    bpb.SectorsPerTrack = 32;          // unknown here
    bpb.NumberOfHeads = 64;            // ?
    //bpb.HiddenSectors = 0;
    bpb.TotalSectors32 = totalSectors;
    // BPB-FAT32 Extension
    bpb.FSTypeSpecificData.fat32.FATSize = totalSectors / 4095;
    //bpb.FSTypeSpecificData.fat32.ExtFlags = 0;
    //bpb.FSTypeSpecificData.fat32.FSVersion = 0;
    bpb.FSTypeSpecificData.fat32.RootCluster = 2;
    bpb.FSTypeSpecificData.fat32.FSInfo = 1;
    bpb.FSTypeSpecificData.fat32.BkBootSec = 6;
    //memset( bpb.FSTypeSpecificData.fat32.Reserved, 0x00, 12 );
    //bpb.FSTypeSpecificData.fat32.BS_DriveNumber = 0;
    //bpb.FSTypeSpecificData.fat32.BS_Reserved1 = 0;
    bpb.FSTypeSpecificData.fat32.BS_BootSig = 0x29;
    bpb.FSTypeSpecificData.fat32.BS_VolumeID = 0xf358ddc1;      // hardcoded volume id.  this is weird.  should be generated each time.
    memset(bpb.FSTypeSpecificData.fat32.BS_VolumeLabel, 0x20, 11);
    memcpy(bpb.FSTypeSpecificData.fat32.BS_FileSystemType, "FAT32   ", 8);
    memcpy(sectorBuf0, &bpb, sizeof(bpb));

    memcpy(sectorBuf0 + 0x5a, "\x0e\x1f\xbe\x77\x7c\xac\x22\xc0\x74\x0b\x56\xb4\x0e\xbb\x07\x00\xcd\x10\x5e\xeb\xf0\x32\xe4\xcd\x17\xcd\x19\xeb\xfeThis is not a bootable disk.  Please insert a bootable floppy and\r\npress any key to try again ... \r\n", 129);

    fat = (bpb.ReservedSectorCount);

        // ending signatures
    sectorBuf0[0x1fe] = 0x55;
    sectorBuf0[0x1ff] = 0xAA;
    write_sector(sectorBuf0, 0);

    // set up key sectors...

    ssa = (bpb.NumFATs * bpb.FSTypeSpecificData.fat32.FATSize) + fat;

    // FSInfo sector
    memset(sectorBuf, 0x00, 0x200);
    *((uint32_t*)sectorBuf) = 0x41615252;
    *((uint32_t*)(sectorBuf + 0x1e4)) = 0x61417272;
    *((uint32_t*)(sectorBuf + 0x1e8)) = 0xffffffff; // last known number of free data clusters on volume
    *((uint32_t*)(sectorBuf + 0x1ec)) = 0xffffffff; // number of most recently known to be allocated cluster
    *((uint32_t*)(sectorBuf + 0x1f0)) = 0;  // reserved
    *((uint32_t*)(sectorBuf + 0x1f4)) = 0;  // reserved
    *((uint32_t*)(sectorBuf + 0x1f8)) = 0;  // reserved
    *((uint32_t*)(sectorBuf + 0x1fc)) = 0xaa550000;
    write_sector(sectorBuf, 1);
    fat = (bpb.ReservedSectorCount);

    memset(sectorBuf, 0x00, 0x200);
    for (scl = 2; scl < bpb.SectorsPerCluster; scl++)
    {
        memset(sectorBuf, 0x00, 0x200);
        write_sector(sectorBuf, scl);
    }
    // write backup copy of metadata
    write_sector(sectorBuf0, 6);



    // make Root Directory 

    // whack ROOT directory file: SSA = RSC + FN x SF + ceil((32 x RDE)/SS)  and LSN = SSA + (CN-2) x SC
    // this clears the first cluster of the root directory
    memset(sectorBuf, 0x00, 0x200);     // 0x00000000 is the unallocated marker
    for (scl = ssa + bpb.SectorsPerCluster; scl >= ssa; scl--)
    {
        write_sector(sectorBuf, scl);
    }

    /*// whack a few clusters 1/4th through the partition as well.
    // FIXME: This is a total hack, based on observed behavior.  use determinism
    for (scl=(10 * bpb->SectorsPerCluster); scl>0; scl--)
    {
        dbg_printf("wiping sector %x", scl+(bpb->TotalSectors32 / 2048));
        write_sector( sectorBuf, scl+(bpb->TotalSectors32 / 2048) );
    }*/

    memset(sectorBuf, 0x00, 0x200);     // 0x00000000 is the unallocated marker
    for (scl = fat; scl < ssa / 2; scl++)
    {
        write_sector(sectorBuf, scl);
        write_sector(sectorBuf, scl + (ssa / 2));
    }

    //SSA = RSC + FN x SF + ceil((32 x RDE)/SS)  and LSN = SSA + (CN-2) x SC

    // now set up first sector and write.
    *((uint32_t*)(sectorBuf + 0x000)) = 0x0ffffff8;   // special - EOF marker
    *((uint32_t*)(sectorBuf + 0x004)) = 0x0fffffff;   // special and clean
    *((uint32_t*)(sectorBuf + 0x008)) = 0x0ffffff8;   // root directory (one cluster)
    write_sector(sectorBuf, bpb.SectorsPerCluster);

    return TF_ERR_NO_ERROR;
}


/*! tf_get_open_handles()
    returns a bitfield where the handles are open (1) or free (0)
    assumes there are <64 handles
*/
uint64_t tf_get_open_handles(void)
{
    int         i;
    TFFile*     fp;
    uint64_t    retval = 0;

    for (i = 0; i < min(TF_FILE_HANDLES, 64); i++) 
    {
        retval <<= 1;
        fp = &tf_file_handles[i];
        if (fp->flags & TF_FLAG_OPEN)
        {
            retval |= 1;
        }
    }
    return retval;
}


