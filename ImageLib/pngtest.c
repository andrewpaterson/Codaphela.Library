
/* pngtest.c - a simple test program to test libpng
 *
 * Last changed in libpng 1.2.6 - August 15, 2004
 * For conditions of distribution and use, see copyright notice in png.h
 * Copyright (c) 1998-2004 Glenn Randers-Pehrson
 * (Version 0.96 Copyright (c) 1996, 1997 Andreas Dilger)
 * (Version 0.88 Copyright (c) 1995, 1996 Guy Eric Schalnat, Group 42, Inc.)
 *
 * This program reads in a PNG image, writes it out again, and then
 * compares the two files.  If the files are identical, this shows that
 * the basic chunk handling, filtering, and (de)compression code is working
 * properly.  It does not currently test all of the transforms, although
 * it probably should.
 *
 * The program will report "FAIL" in certain legitimate cases:
 * 1) when the compression level or filter selection method is changed.
 * 2) when the maximum IDAT size (PNG_ZBUF_SIZE in pngconf.h) is not 8192.
 * 3) unknown unsafe-to-copy ancillary chunks or unknown critical chunks
 *    exist in the input file.
 * 4) others not listed here...
 * In these cases, it is best to check with another tool such as "pngcheck"
 * to see what the differences between the two files are.
 *
 * If a filename is given on the command-line, then this file is used
 * for the input, rather than the default "pngtest.png".  This allows
 * testing a wide variety of files easily.  You can also test a number
 * of files at once by typing "pngtest -m file1.png file2.png ..."
 */

#include "png.h"

#if defined(_WIN32_WCE)
#  if _WIN32_WCE < 211
     __error__ (f|w)printf functions are not supported on old WindowsCE.;
#  endif
#  include <windows.h>
#  include <stdlib.h>
#  define READFILE(file, data, length, check) \
     if (ReadFile(file, data, length, &check,NULL)) check = 0
#  define WRITEFILE(file, data, length, check)) \
     if (WriteFile(file, data, length, &check, NULL)) check = 0
#  define FCLOSE(file) CloseHandle(file)
#else
#  include <stdio.h>
#  include <stdlib.h>
#  define READFILE(file, data, length, check) \
     check=(png_size_t)fread(data,(png_size_t)1,length,file)
#  define WRITEFILE(file, data, length, check) \
     check=(png_size_t)fwrite(data,(png_size_t)1, length, file)
#  define FCLOSE(file) fclose(file)
#endif

#if defined(PNG_NO_STDIO)
#  if defined(_WIN32_WCE)
     typedef HANDLE                png_FILE_p;
#  else
     typedef FILE                * png_FILE_p;
#  endif
#endif

/* Makes pngtest verbose so we can find problems (needs to be before png.h) */
#ifndef PNG_DEBUG
#  define PNG_DEBUG 0
#endif

#if !PNG_DEBUG
#  define SINGLE_ROWBUF_ALLOC  /* makes buffer overruns easier to nail */
#endif

/* Turn on CPU timing
#define PNGTEST_TIMING
*/

#ifdef PNG_NO_FLOATING_POINT_SUPPORTED
#undef PNGTEST_TIMING
#endif

#ifdef PNGTEST_TIMING
static float t_start, t_stop, t_decode, t_encode, t_misc;
#include <time.h>
#endif

#if defined(PNG_TIME_RFC1123_SUPPORTED)
static int tIME_chunk_present=0;
static char tIME_string[30] = "no tIME chunk present in file";
#endif

static int verbose = 0;

int test_one_file PNGARG((PNG_CONST char *inname, PNG_CONST char *outname));

#ifdef __TURBOC__
#include <mem.h>
#endif

/* defined so I can write to a file on gui/windowing platforms */
/*  #define STDERR stderr  */
#define STDERR stdout   /* for DOS */

/* example of using row callbacks to make a simple progress meter */
static int status_pass=1;
static int status_dots_requested=0;
static int status_dots=1;

/* In case a system header (e.g., on AIX) defined jmpbuf */
#ifdef jmpbuf
#  undef jmpbuf
#endif

/* Define png_jmpbuf() in case we are using a pre-1.0.6 version of libpng */
#ifndef png_jmpbuf
#  define png_jmpbuf(png_ptr) png_ptr->jmpbuf
#endif

void
#ifdef PNG_1_0_X
PNGAPI
#endif
read_row_callback(png_structp png_ptr, uint32 row_number, int pass);
void
#ifdef PNG_1_0_X
PNGAPI
#endif
read_row_callback(png_structp png_ptr, uint32 row_number, int pass)
{
    if(png_ptr == NULL || row_number > PNG_UINT_31_MAX) return;
    if(status_pass != pass)
    {
       fprintf(stdout,"\n Pass %d: ",pass);
       status_pass = pass;
       status_dots = 31;
    }
    status_dots--;
    if(status_dots == 0)
    {
       fprintf(stdout, "\n         ");
       status_dots=30;
    }
    fprintf(stdout, "r");
}

void
#ifdef PNG_1_0_X
PNGAPI
#endif
write_row_callback(png_structp png_ptr, uint32 row_number, int pass);
void
#ifdef PNG_1_0_X
PNGAPI
#endif
write_row_callback(png_structp png_ptr, uint32 row_number, int pass)
{
    if(png_ptr == NULL || row_number > PNG_UINT_31_MAX || pass > 7) return;
    fprintf(stdout, "w");
}


#if defined(PNG_READ_USER_TRANSFORM_SUPPORTED)
/* Example of using user transform callback (we don't transform anything,
   but merely examine the row filters.  We set this to 256 rather than
   5 in case illegal filter values are present.) */
static uint32 filters_used[256];
void
#ifdef PNG_1_0_X
PNGAPI
#endif
count_filters(png_structp png_ptr, png_row_infop row_info, png_bytep data);
void
#ifdef PNG_1_0_X
PNGAPI
#endif
count_filters(png_structp png_ptr, png_row_infop row_info, png_bytep data)
{
    if(png_ptr != NULL && row_info != NULL)
      ++filters_used[*(data-1)];
}
#endif

#if defined(PNG_WRITE_USER_TRANSFORM_SUPPORTED)
/* example of using user transform callback (we don't transform anything,
   but merely count the zero samples) */

static uint32 zero_samples;

void
#ifdef PNG_1_0_X
PNGAPI
#endif
count_zero_samples(png_structp png_ptr, png_row_infop row_info, png_bytep data);
void
#ifdef PNG_1_0_X
PNGAPI
#endif
count_zero_samples(png_structp png_ptr, png_row_infop row_info, png_bytep data)
{
   png_bytep dp = data;
   if(png_ptr == NULL)return;

   /* contents of row_info:
    *  uint32 width      width of row
    *  uint32 rowbytes   number of bytes in row
    *  uint8 color_type    color type of pixels
    *  uint8 bit_depth     bit depth of samples
    *  uint8 channels      number of channels (1-4)
    *  uint8 pixel_depth   bits per pixel (depth*channels)
    */


    /* counts the number of zero samples (or zero pixels if color_type is 3 */

    if(row_info->color_type == 0 || row_info->color_type == 3)
    {
       int pos=0;
       uint32 n, nstop;
       for (n=0, nstop=row_info->width; n<nstop; n++)
       {
          if(row_info->bit_depth == 1)
          {
             if(((*dp << pos++ ) & 0x80) == 0) zero_samples++;
             if(pos == 8)
             {
                pos = 0;
                dp++;
             }
          }
          if(row_info->bit_depth == 2)
          {
             if(((*dp << (pos+=2)) & 0xc0) == 0) zero_samples++;
             if(pos == 8)
             {
                pos = 0;
                dp++;
             }
          }
          if(row_info->bit_depth == 4)
          {
             if(((*dp << (pos+=4)) & 0xf0) == 0) zero_samples++;
             if(pos == 8)
             {
                pos = 0;
                dp++;
             }
          }
          if(row_info->bit_depth == 8)
             if(*dp++ == 0) zero_samples++;
          if(row_info->bit_depth == 16)
          {
             if((*dp | *(dp+1)) == 0) zero_samples++;
             dp+=2;
          }
       }
    }
    else /* other color types */
    {
       uint32 n, nstop;
       int channel;
       int color_channels = row_info->channels;
       if(row_info->color_type > 3)color_channels--;

       for (n=0, nstop=row_info->width; n<nstop; n++)
       {
          for (channel = 0; channel < color_channels; channel++)
          {
             if(row_info->bit_depth == 8)
                if(*dp++ == 0) zero_samples++;
             if(row_info->bit_depth == 16)
             {
                if((*dp | *(dp+1)) == 0) zero_samples++;
                dp+=2;
             }
          }
          if(row_info->color_type > 3)
          {
             dp++;
             if(row_info->bit_depth == 16)dp++;
          }
       }
    }
}
#endif /* PNG_WRITE_USER_TRANSFORM_SUPPORTED */

static int wrote_question = 0;

#if defined(PNG_NO_STDIO)
/* START of code to validate stdio-free compilation */
/* These copies of the default read/write functions come from pngrio.c and */
/* pngwio.c.  They allow "don't include stdio" testing of the library. */
/* This is the function that does the actual reading of data.  If you are
   not reading from a standard C stream, you should create a replacement
   read_data function and use it at run time with png_set_read_fn(), rather
   than changing the library. */

#ifndef USE_FAR_KEYWORD
static void
pngtest_read_data(png_structp png_ptr, png_bytep data, png_size_t length)
{
   png_size_t check;

   /* fread() returns 0 on error, so it is OK to store this in a png_size_t
    * instead of an int, which is what fread() actually returns.
    */
   READFILE((png_FILE_p)png_ptr->io_ptr, data, length, check);

   if (check != length)
   {
      png_error(png_ptr, "Read Error!");
   }
}
#else
/* this is the model-independent version. Since the standard I/O library
   can't handle far buffers in the medium and small models, we have to copy
   the data.
*/

#define NEAR_BUF_SIZE 1024
#define MIN(a,b) (a <= b ? a : b)

static void
pngtest_read_data(png_structp png_ptr, png_bytep data, png_size_t length)
{
   int check;
   uint8 *n_data;
   png_FILE_p io_ptr;

   /* Check if data really is near. If so, use usual code. */
   n_data = (uint8 *)CVT_PTR_NOCHECK(data);
   io_ptr = (png_FILE_p)CVT_PTR(png_ptr->io_ptr);
   if ((png_bytep)n_data == data)
   {
      READFILE(io_ptr, n_data, length, check);
   }
   else
   {
      uint8 buf[NEAR_BUF_SIZE];
      png_size_t read, remaining, err;
      check = 0;
      remaining = length;
      do
      {
         read = MIN(NEAR_BUF_SIZE, remaining);
         READFILE(io_ptr, buf, 1, err);
         png_memcpy(data, buf, read); /* copy far buffer to near buffer */
         if(err != read)
            break;
         else
            check += err;
         data += read;
         remaining -= read;
      }
      while (remaining != 0);
   }
   if (check != length)
   {
      png_error(png_ptr, "read Error");
   }
}
#endif /* USE_FAR_KEYWORD */

#if defined(PNG_WRITE_FLUSH_SUPPORTED)
static void
pngtest_flush(png_structp png_ptr)
{
#if !defined(_WIN32_WCE)
   png_FILE_p io_ptr;
   io_ptr = (png_FILE_p)CVT_PTR((png_ptr->io_ptr));
   if (io_ptr != NULL)
      fflush(io_ptr);
#endif
}
#endif

/* This is the function that does the actual writing of data.  If you are
   not writing to a standard C stream, you should create a replacement
   write_data function and use it at run time with png_set_write_fn(), rather
   than changing the library. */
#ifndef USE_FAR_KEYWORD
static void
pngtest_write_data(png_structp png_ptr, png_bytep data, png_size_t length)
{
   uint32 check;

   WRITEFILE((png_FILE_p)png_ptr->io_ptr,  data, length, check);
   if (check != length)
   {
      png_error(png_ptr, "Write Error");
   }
}
#else
/* this is the model-independent version. Since the standard I/O library
   can't handle far buffers in the medium and small models, we have to copy
   the data.
*/

#define NEAR_BUF_SIZE 1024
#define MIN(a,b) (a <= b ? a : b)

static void
pngtest_write_data(png_structp png_ptr, png_bytep data, png_size_t length)
{
   uint32 check;
   uint8 *near_data;  /* Needs to be "uint8 *" instead of "png_bytep" */
   png_FILE_p io_ptr;

   /* Check if data really is near. If so, use usual code. */
   near_data = (uint8 *)CVT_PTR_NOCHECK(data);
   io_ptr = (png_FILE_p)CVT_PTR(png_ptr->io_ptr);
   if ((png_bytep)near_data == data)
   {
      WRITEFILE(io_ptr, near_data, length, check);
   }
   else
   {
      uint8 buf[NEAR_BUF_SIZE];
      png_size_t written, remaining, err;
      check = 0;
      remaining = length;
      do
      {
         written = MIN(NEAR_BUF_SIZE, remaining);
         png_memcpy(buf, data, written); /* copy far buffer to near buffer */
         WRITEFILE(io_ptr, buf, written, err);
         if (err != written)
            break;
         else
            check += err;
         data += written;
         remaining -= written;
      }
      while (remaining != 0);
   }
   if (check != length)
   {
      png_error(png_ptr, "Write Error");
   }
}

#endif /* USE_FAR_KEYWORD */

/* This function is called when there is a warning, but the library thinks
 * it can continue anyway.  Replacement functions don't have to do anything
 * here if you don't want to.  In the default configuration, png_ptr is
 * not used, but it is passed in case it may be useful.
 */
static void
pngtest_warning(png_structp png_ptr, png_const_charp message)
{
   PNG_CONST char *name = "UNKNOWN (ERROR!)";
   if (png_ptr != NULL && png_ptr->error_ptr != NULL)
      name = png_ptr->error_ptr;
   fprintf(STDERR, "%s: libpng warning: %s\n", name, message);
}

/* This is the default error handling function.  Note that replacements for
 * this function MUST NOT RETURN, or the program will likely crash.  This
 * function is used by default, or if the program supplies NULL for the
 * error function pointer in png_set_error_fn().
 */
static void
pngtest_error(png_structp png_ptr, png_const_charp message)
{
   pngtest_warning(png_ptr, message);
   /* We can return because png_error calls the default handler, which is
    * actually OK in this case. */
}
#endif /* PNG_NO_STDIO */
/* END of code to validate stdio-free compilation */

/* START of code to validate memory allocation and deallocation */
#if defined(PNG_USER_MEM_SUPPORTED) && PNG_DEBUG

/* Allocate memory.  For reasonable files, size should never exceed
   64K.  However, zlib may allocate more then 64K if you don't tell
   it not to.  See zconf.h and png.h for more information.  zlib does
   need to allocate exactly 64K, so whatever you call here must
   have the ability to do that.

   This piece of code can be compiled to validate max 64K allocations
   by setting MAXSEG_64K in zlib zconf.h *or* PNG_MAX_MALLOC_64K. */
typedef struct memory_information
{
   uint32               size;
   png_voidp                 pointer;
   struct memory_information *next;
} memory_information;
typedef memory_information *memory_infop;

static memory_infop pinformation = NULL;
static int current_allocation = 0;
static int maximum_allocation = 0;
static int total_allocation = 0;
static int num_allocations = 0;

png_voidp png_debug_malloc PNGARG((png_structp png_ptr, uint32 size));
void png_debug_free PNGARG((png_structp png_ptr, png_voidp ptr));

png_voidp
png_debug_malloc(png_structp png_ptr, uint32 size)
{

   /* png_malloc has already tested for NULL; png_create_struct calls
      png_debug_malloc directly, with png_ptr == NULL which is OK */

   if (size == 0)
      return (NULL);

   /* This calls the library allocator twice, once to get the requested
      buffer and once to get a new free list entry. */
   {
      /* Disable malloc_fn and free_fn */
      memory_infop pinfo;
      png_set_mem_fn(png_ptr, NULL, NULL, NULL);
      pinfo = (memory_infop)png_malloc(png_ptr,
         (uint32)png_sizeof (*pinfo));
      pinfo->size = size;
      current_allocation += size;
      total_allocation += size;
      num_allocations ++;
      if (current_allocation > maximum_allocation)
         maximum_allocation = current_allocation;
      pinfo->pointer = (png_voidp)png_malloc(png_ptr, size);
      /* Restore malloc_fn and free_fn */
      png_set_mem_fn(png_ptr, png_voidp_NULL, (png_malloc_ptr)png_debug_malloc,
         (png_free_ptr)png_debug_free);
      if (size != 0 && pinfo->pointer == NULL)
      {
         current_allocation -= size;
         total_allocation -= size;
         png_error(png_ptr,
           "out of memory in pngtest->png_debug_malloc.");
      }
      pinfo->next = pinformation;
      pinformation = pinfo;
      /* Make sure the caller isn't assuming zeroed memory. */
      png_memset(pinfo->pointer, 0xdd, pinfo->size);
      if(verbose)
         printf("png_malloc %lu bytes at %x\n",(uint32)size,
          pinfo->pointer);
      return (png_voidp)(pinfo->pointer);
   }
}

/* Free a pointer.  It is removed from the list at the same time. */
void
png_debug_free(png_structp png_ptr, png_voidp ptr)
{
   if (png_ptr == NULL)
      fprintf(STDERR, "NULL pointer to png_debug_free.\n");
   if (ptr == 0)
   {
#if 0 /* This happens all the time. */
      fprintf(STDERR, "WARNING: freeing NULL pointer\n");
#endif
      return;
   }

   /* Unlink the element from the list. */
   {
      memory_infop *ppinfo = &pinformation;
      for (;;)
      {
         memory_infop pinfo = *ppinfo;
         if (pinfo->pointer == ptr)
         {
            *ppinfo = pinfo->next;
            current_allocation -= pinfo->size;
            if (current_allocation < 0)
               fprintf(STDERR, "Duplicate free of memory\n");
            /* We must free the list element too, but first kill
               the memory that is to be freed. */
            png_memset(ptr, 0x55, pinfo->size);
            png_free_default(png_ptr, pinfo);
            pinfo=NULL;
            break;
         }
         if (pinfo->next == NULL)
         {
            fprintf(STDERR, "Pointer %x not found\n", (uint32)ptr);
            break;
         }
         ppinfo = &pinfo->next;
      }
   }

   /* Finally free the data. */
   if(verbose)
      printf("Freeing %x\n",ptr);
   png_free_default(png_ptr, ptr);
   ptr=NULL;
}
#endif /* PNG_USER_MEM_SUPPORTED && PNG_DEBUG */
/* END of code to test memory allocation/deallocation */

/* Test one file */
int
test_one_file(PNG_CONST char *inname, PNG_CONST char *outname)
{
   static png_FILE_p fpin;
   static png_FILE_p fpout;  /* "static" prevents setjmp corruption */
   png_structp read_ptr;
   png_infop read_info_ptr, end_info_ptr;
#ifdef PNG_WRITE_SUPPORTED
   png_structp write_ptr;
   png_infop write_info_ptr;
   png_infop write_end_info_ptr;
#else
   png_structp write_ptr = NULL;
   png_infop write_info_ptr = NULL;
   png_infop write_end_info_ptr = NULL;
#endif
   png_bytep row_buf;
   uint32 y;
   uint32 width, height;
   int num_pass, pass;
   int bit_depth, color_type;
#ifdef PNG_SETJMP_SUPPORTED
#ifdef USE_FAR_KEYWORD
   jmp_buf jmpbuf;
#endif
#endif

#if defined(_WIN32_WCE)
   TCHAR path[MAX_PATH];
#endif
   char inbuf[256], outbuf[256];

   row_buf = NULL;

#if defined(_WIN32_WCE)
   MultiByteToWideChar(CP_ACP, 0, inname, -1, path, MAX_PATH);
   if ((fpin = CreateFile(path, GENERIC_READ, 0, NULL, OPEN_EXISTING, 0, NULL)) == INVALID_HANDLE_VALUE)
#else
   if ((fpin = fopen(inname, "rb")) == NULL)
#endif
   {
      fprintf(STDERR, "Could not find input file %s\n", inname);
      return (1);
   }

#if defined(_WIN32_WCE)
   MultiByteToWideChar(CP_ACP, 0, outname, -1, path, MAX_PATH);
   if ((fpout = CreateFile(path, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, 0, NULL)) == INVALID_HANDLE_VALUE)
#else
   if ((fpout = fopen(outname, "wb")) == NULL)
#endif
   {
      fprintf(STDERR, "Could not open output file %s\n", outname);
      FCLOSE(fpin);
      return (1);
   }

   png_debug(0, "Allocating read and write structures\n");
#if defined(PNG_USER_MEM_SUPPORTED) && PNG_DEBUG
   read_ptr = png_create_read_struct_2(PNG_LIBPNG_VER_STRING, png_voidp_NULL,
      png_error_ptr_NULL, png_error_ptr_NULL, png_voidp_NULL,
      (png_malloc_ptr)png_debug_malloc, (png_free_ptr)png_debug_free);
#else
   read_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, png_voidp_NULL,
      png_error_ptr_NULL, png_error_ptr_NULL);
#endif
#if defined(PNG_NO_STDIO)
   png_set_error_fn(read_ptr, (png_voidp)inname, pngtest_error,
       pngtest_warning);
#endif
#ifdef PNG_WRITE_SUPPORTED
#if defined(PNG_USER_MEM_SUPPORTED) && PNG_DEBUG
   write_ptr = png_create_write_struct_2(PNG_LIBPNG_VER_STRING, png_voidp_NULL,
      png_error_ptr_NULL, png_error_ptr_NULL, png_voidp_NULL,
      (png_malloc_ptr)png_debug_malloc, (png_free_ptr)png_debug_free);
#else
   write_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, png_voidp_NULL,
      png_error_ptr_NULL, png_error_ptr_NULL);
#endif
#if defined(PNG_NO_STDIO)
   png_set_error_fn(write_ptr, (png_voidp)inname, pngtest_error,
       pngtest_warning);
#endif
#endif
   png_debug(0, "Allocating read_info, write_info and end_info structures\n");
   read_info_ptr = png_create_info_struct(read_ptr);
   end_info_ptr = png_create_info_struct(read_ptr);
#ifdef PNG_WRITE_SUPPORTED
   write_info_ptr = png_create_info_struct(write_ptr);
   write_end_info_ptr = png_create_info_struct(write_ptr);
#endif

#ifdef PNG_SETJMP_SUPPORTED
   png_debug(0, "Setting jmpbuf for read struct\n");
#ifdef USE_FAR_KEYWORD
   if (setjmp(jmpbuf))
#else
   if (setjmp(png_jmpbuf(read_ptr)))
#endif
   {
      fprintf(STDERR, "%s -> %s: libpng read error\n", inname, outname);
      if (row_buf)
         png_free(read_ptr, row_buf);
      png_destroy_read_struct(&read_ptr, &read_info_ptr, &end_info_ptr);
#ifdef PNG_WRITE_SUPPORTED
      png_destroy_info_struct(write_ptr, &write_end_info_ptr);
      png_destroy_write_struct(&write_ptr, &write_info_ptr);
#endif
      FCLOSE(fpin);
      FCLOSE(fpout);
      return (1);
   }
#ifdef USE_FAR_KEYWORD
   png_memcpy(png_jmpbuf(read_ptr),jmpbuf,png_sizeof(jmp_buf));
#endif

#ifdef PNG_WRITE_SUPPORTED
   png_debug(0, "Setting jmpbuf for write struct\n");
#ifdef USE_FAR_KEYWORD
   if (setjmp(jmpbuf))
#else
   if (setjmp(png_jmpbuf(write_ptr)))
#endif
   {
      fprintf(STDERR, "%s -> %s: libpng write error\n", inname, outname);
      png_destroy_read_struct(&read_ptr, &read_info_ptr, &end_info_ptr);
      png_destroy_info_struct(write_ptr, &write_end_info_ptr);
#ifdef PNG_WRITE_SUPPORTED
      png_destroy_write_struct(&write_ptr, &write_info_ptr);
#endif
      FCLOSE(fpin);
      FCLOSE(fpout);
      return (1);
   }
#ifdef USE_FAR_KEYWORD
   png_memcpy(png_jmpbuf(write_ptr),jmpbuf,png_sizeof(jmp_buf));
#endif
#endif
#endif

   png_debug(0, "Initializing input and output streams\n");
#if !defined(PNG_NO_STDIO)
   png_init_io(read_ptr, fpin);
#  ifdef PNG_WRITE_SUPPORTED
   png_init_io(write_ptr, fpout);
#  endif
#else
   png_set_read_fn(read_ptr, (png_voidp)fpin, pngtest_read_data);
#  ifdef PNG_WRITE_SUPPORTED
   png_set_write_fn(write_ptr, (png_voidp)fpout,  pngtest_write_data,
#    if defined(PNG_WRITE_FLUSH_SUPPORTED)
      pngtest_flush);
#    else
      NULL);
#    endif
#  endif
#endif
   if(status_dots_requested == 1)
   {
#ifdef PNG_WRITE_SUPPORTED
      png_set_write_status_fn(write_ptr, write_row_callback);
#endif
      png_set_read_status_fn(read_ptr, read_row_callback);
   }
   else
   {
#ifdef PNG_WRITE_SUPPORTED
      png_set_write_status_fn(write_ptr, png_write_status_ptr_NULL);
#endif
      png_set_read_status_fn(read_ptr, png_read_status_ptr_NULL);
   }

#if defined(PNG_READ_USER_TRANSFORM_SUPPORTED)
   {
     int i;
     for(i = 0; i<256; i++)
        filters_used[i]=0;
     png_set_read_user_transform_fn(read_ptr, count_filters);
   }
#endif
#if defined(PNG_WRITE_USER_TRANSFORM_SUPPORTED)
   zero_samples=0;
   png_set_write_user_transform_fn(write_ptr, count_zero_samples);
#endif

#if defined(PNG_READ_UNKNOWN_CHUNKS_SUPPORTED)
#  ifndef PNG_HANDLE_CHUNK_ALWAYS
#    define PNG_HANDLE_CHUNK_ALWAYS       3
#  endif
   png_set_keep_unknown_chunks(read_ptr, PNG_HANDLE_CHUNK_ALWAYS,
      png_bytep_NULL, 0);
#endif
#if defined(PNG_WRITE_UNKNOWN_CHUNKS_SUPPORTED)
#  ifndef PNG_HANDLE_CHUNK_IF_SAFE
#    define PNG_HANDLE_CHUNK_IF_SAFE      2
#  endif
   png_set_keep_unknown_chunks(write_ptr, PNG_HANDLE_CHUNK_IF_SAFE,
      png_bytep_NULL, 0);
#endif

   png_debug(0, "Reading info struct\n");
   png_read_info(read_ptr, read_info_ptr);

   png_debug(0, "Transferring info struct\n");
   {
      int interlace_type, compression_type, filter_type;

      if (png_get_IHDR(read_ptr, read_info_ptr, &width, &height, &bit_depth,
          &color_type, &interlace_type, &compression_type, &filter_type))
      {
         png_set_IHDR(write_ptr, write_info_ptr, width, height, bit_depth,
#if defined(PNG_WRITE_INTERLACING_SUPPORTED)
            color_type, interlace_type, compression_type, filter_type);
#else
            color_type, PNG_INTERLACE_NONE, compression_type, filter_type);
#endif
      }
   }
#if defined(PNG_FIXED_POINT_SUPPORTED)
#if defined(PNG_cHRM_SUPPORTED)
   {
      png_fixed_point white_x, white_y, red_x, red_y, green_x, green_y, blue_x,
         blue_y;
      if (png_get_cHRM_fixed(read_ptr, read_info_ptr, &white_x, &white_y, &red_x,
         &red_y, &green_x, &green_y, &blue_x, &blue_y))
      {
         png_set_cHRM_fixed(write_ptr, write_info_ptr, white_x, white_y, red_x,
            red_y, green_x, green_y, blue_x, blue_y);
      }
   }
#endif
#if defined(PNG_gAMA_SUPPORTED)
   {
      png_fixed_point gamma;

      if (png_get_gAMA_fixed(read_ptr, read_info_ptr, &gamma))
      {
         png_set_gAMA_fixed(write_ptr, write_info_ptr, gamma);
      }
   }
#endif
#else /* Use floating point versions */
#if defined(PNG_FLOATING_POINT_SUPPORTED)
#if defined(PNG_cHRM_SUPPORTED)
   {
      double white_x, white_y, red_x, red_y, green_x, green_y, blue_x,
         blue_y;
      if (png_get_cHRM(read_ptr, read_info_ptr, &white_x, &white_y, &red_x,
         &red_y, &green_x, &green_y, &blue_x, &blue_y))
      {
         png_set_cHRM(write_ptr, write_info_ptr, white_x, white_y, red_x,
            red_y, green_x, green_y, blue_x, blue_y);
      }
   }
#endif
#if defined(PNG_gAMA_SUPPORTED)
   {
      double gamma;

      if (png_get_gAMA(read_ptr, read_info_ptr, &gamma))
      {
         png_set_gAMA(write_ptr, write_info_ptr, gamma);
      }
   }
#endif
#endif /* floating point */
#endif /* fixed point */
#if defined(PNG_iCCP_SUPPORTED)
   {
      png_charp name;
      png_charp profile;
      uint32 proflen;
      int compression_type;

      if (png_get_iCCP(read_ptr, read_info_ptr, &name, &compression_type,
                      &profile, &proflen))
      {
         png_set_iCCP(write_ptr, write_info_ptr, name, compression_type,
                      profile, proflen);
      }
   }
#endif
#if defined(PNG_sRGB_SUPPORTED)
   {
      int intent;

      if (png_get_sRGB(read_ptr, read_info_ptr, &intent))
      {
         png_set_sRGB(write_ptr, write_info_ptr, intent);
      }
   }
#endif
   {
      png_colorp palette;
      int num_palette;

      if (png_get_PLTE(read_ptr, read_info_ptr, &palette, &num_palette))
      {
         png_set_PLTE(write_ptr, write_info_ptr, palette, num_palette);
      }
   }
#if defined(PNG_bKGD_SUPPORTED)
   {
      png_color_16p background;

      if (png_get_bKGD(read_ptr, read_info_ptr, &background))
      {
         png_set_bKGD(write_ptr, write_info_ptr, background);
      }
   }
#endif
#if defined(PNG_hIST_SUPPORTED)
   {
      png_uint_16p hist;

      if (png_get_hIST(read_ptr, read_info_ptr, &hist))
      {
         png_set_hIST(write_ptr, write_info_ptr, hist);
      }
   }
#endif
#if defined(PNG_oFFs_SUPPORTED)
   {
      int32 offset_x, offset_y;
      int unit_type;

      if (png_get_oFFs(read_ptr, read_info_ptr,&offset_x,&offset_y,&unit_type))
      {
         png_set_oFFs(write_ptr, write_info_ptr, offset_x, offset_y, unit_type);
      }
   }
#endif
#if defined(PNG_pCAL_SUPPORTED)
   {
      png_charp purpose, units;
      png_charpp params;
      int32 X0, X1;
      int type, nparams;

      if (png_get_pCAL(read_ptr, read_info_ptr, &purpose, &X0, &X1, &type,
         &nparams, &units, &params))
      {
         png_set_pCAL(write_ptr, write_info_ptr, purpose, X0, X1, type,
            nparams, units, params);
      }
   }
#endif
#if defined(PNG_pHYs_SUPPORTED)
   {
      uint32 res_x, res_y;
      int unit_type;

      if (png_get_pHYs(read_ptr, read_info_ptr, &res_x, &res_y, &unit_type))
      {
         png_set_pHYs(write_ptr, write_info_ptr, res_x, res_y, unit_type);
      }
   }
#endif
#if defined(PNG_sBIT_SUPPORTED)
   {
      png_color_8p sig_bit;

      if (png_get_sBIT(read_ptr, read_info_ptr, &sig_bit))
      {
         png_set_sBIT(write_ptr, write_info_ptr, sig_bit);
      }
   }
#endif
#if defined(PNG_sCAL_SUPPORTED)
#ifdef PNG_FLOATING_POINT_SUPPORTED
   {
      int unit;
      double scal_width, scal_height;

      if (png_get_sCAL(read_ptr, read_info_ptr, &unit, &scal_width,
         &scal_height))
      {
         png_set_sCAL(write_ptr, write_info_ptr, unit, scal_width, scal_height);
      }
   }
#else
#ifdef PNG_FIXED_POINT_SUPPORTED
   {
      int unit;
      png_charp scal_width, scal_height;

      if (png_get_sCAL_s(read_ptr, read_info_ptr, &unit, &scal_width,
          &scal_height))
      {
         png_set_sCAL_s(write_ptr, write_info_ptr, unit, scal_width, scal_height);
      }
   }
#endif
#endif
#endif
#if defined(PNG_TEXT_SUPPORTED)
   {
      png_textp text_ptr;
      int num_text;

      if (png_get_text(read_ptr, read_info_ptr, &text_ptr, &num_text) > 0)
      {
         png_debug1(0, "Handling %d iTXt/tEXt/zTXt chunks\n", num_text);
         png_set_text(write_ptr, write_info_ptr, text_ptr, num_text);
      }
   }
#endif
#if defined(PNG_tIME_SUPPORTED)
   {
      png_timep mod_time;

      if (png_get_tIME(read_ptr, read_info_ptr, &mod_time))
      {
         png_set_tIME(write_ptr, write_info_ptr, mod_time);
#if defined(PNG_TIME_RFC1123_SUPPORTED)
         /* we have to use png_strcpy instead of "=" because the string
            pointed to by png_convert_to_rfc1123() gets free'ed before
            we use it */
         png_strcpy(tIME_string,png_convert_to_rfc1123(read_ptr, mod_time));
         tIME_chunk_present++;
#endif /* PNG_TIME_RFC1123_SUPPORTED */
      }
   }
#endif
#if defined(PNG_tRNS_SUPPORTED)
   {
      png_bytep trans;
      int num_trans;
      png_color_16p trans_values;

      if (png_get_tRNS(read_ptr, read_info_ptr, &trans, &num_trans,
         &trans_values))
      {
         png_set_tRNS(write_ptr, write_info_ptr, trans, num_trans,
            trans_values);
      }
   }
#endif
#if defined(PNG_WRITE_UNKNOWN_CHUNKS_SUPPORTED)
   {
      png_unknown_chunkp unknowns;
      int num_unknowns = (int)png_get_unknown_chunks(read_ptr, read_info_ptr,
         &unknowns);
      if (num_unknowns)
      {
         png_size_t i;
         png_set_unknown_chunks(write_ptr, write_info_ptr, unknowns,
           num_unknowns);
         /* copy the locations from the read_info_ptr.  The automatically
            generated locations in write_info_ptr are wrong because we
            haven't written anything yet */
         for (i = 0; i < (png_size_t)num_unknowns; i++)
		 {
           png_set_unknown_chunk_location(write_ptr, write_info_ptr, (int)i, (int)unknowns[i].location);
		 }
      }
   }
#endif

#ifdef PNG_WRITE_SUPPORTED
   png_debug(0, "\nWriting info struct\n");

/* If we wanted, we could write info in two steps:
   png_write_info_before_PLTE(write_ptr, write_info_ptr);
 */
   png_write_info(write_ptr, write_info_ptr);
#endif

#ifdef SINGLE_ROWBUF_ALLOC
   png_debug(0, "\nAllocating row buffer...");
   row_buf = (png_bytep)png_malloc(read_ptr,
      png_get_rowbytes(read_ptr, read_info_ptr));
   png_debug1(0, "0x%08lx\n\n", (uint32)row_buf);
#endif /* SINGLE_ROWBUF_ALLOC */
   png_debug(0, "Writing row data\n");

#if defined(PNG_READ_INTERLACING_SUPPORTED) || \
  defined(PNG_WRITE_INTERLACING_SUPPORTED)
   num_pass = png_set_interlace_handling(read_ptr);
#  ifdef PNG_WRITE_SUPPORTED
   png_set_interlace_handling(write_ptr);
#  endif
#else
   num_pass=1;
#endif

#ifdef PNGTEST_TIMING
   t_stop = (float)clock();
   t_misc += (t_stop - t_start);
   t_start = t_stop;
#endif
   for (pass = 0; pass < num_pass; pass++)
   {
      png_debug1(0, "Writing row data for pass %d\n",pass);
      for (y = 0; y < height; y++)
      {
#ifndef SINGLE_ROWBUF_ALLOC
         png_debug2(0, "\nAllocating row buffer (pass %d, y = %ld)...", pass,y);
         row_buf = (png_bytep)png_malloc(read_ptr,
            png_get_rowbytes(read_ptr, read_info_ptr));
         png_debug2(0, "0x%08lx (%ld bytes)\n", (uint32)row_buf,
            png_get_rowbytes(read_ptr, read_info_ptr));
#endif /* !SINGLE_ROWBUF_ALLOC */
         png_read_rows(read_ptr, (png_bytepp)&row_buf, png_bytepp_NULL, 1);

#ifdef PNG_WRITE_SUPPORTED
#ifdef PNGTEST_TIMING
         t_stop = (float)clock();
         t_decode += (t_stop - t_start);
         t_start = t_stop;
#endif
         png_write_rows(write_ptr, (png_bytepp)&row_buf, 1);
#ifdef PNGTEST_TIMING
         t_stop = (float)clock();
         t_encode += (t_stop - t_start);
         t_start = t_stop;
#endif
#endif /* PNG_WRITE_SUPPORTED */

#ifndef SINGLE_ROWBUF_ALLOC
         png_debug2(0, "Freeing row buffer (pass %d, y = %ld)\n\n", pass, y);
         png_free(read_ptr, row_buf);
#endif /* !SINGLE_ROWBUF_ALLOC */
      }
   }

#if defined(PNG_READ_UNKNOWN_CHUNKS_SUPPORTED)
   png_free_data(read_ptr, read_info_ptr, PNG_FREE_UNKN, -1);
#endif
#if defined(PNG_WRITE_UNKNOWN_CHUNKS_SUPPORTED)
   png_free_data(write_ptr, write_info_ptr, PNG_FREE_UNKN, -1);
#endif

   png_debug(0, "Reading and writing end_info data\n");

   png_read_end(read_ptr, end_info_ptr);
#if defined(PNG_TEXT_SUPPORTED)
   {
      png_textp text_ptr;
      int num_text;

      if (png_get_text(read_ptr, end_info_ptr, &text_ptr, &num_text) > 0)
      {
         png_debug1(0, "Handling %d iTXt/tEXt/zTXt chunks\n", num_text);
         png_set_text(write_ptr, write_end_info_ptr, text_ptr, num_text);
      }
   }
#endif
#if defined(PNG_tIME_SUPPORTED)
   {
      png_timep mod_time;

      if (png_get_tIME(read_ptr, end_info_ptr, &mod_time))
      {
         png_set_tIME(write_ptr, write_end_info_ptr, mod_time);
#if defined(PNG_TIME_RFC1123_SUPPORTED)
         /* we have to use png_strcpy instead of "=" because the string
            pointed to by png_convert_to_rfc1123() gets free'ed before
            we use it */
         png_strcpy(tIME_string,png_convert_to_rfc1123(read_ptr, mod_time));
         tIME_chunk_present++;
#endif /* PNG_TIME_RFC1123_SUPPORTED */
      }
   }
#endif
#if defined(PNG_WRITE_UNKNOWN_CHUNKS_SUPPORTED)
   {
      png_unknown_chunkp unknowns;
      int num_unknowns;
      num_unknowns = (int)png_get_unknown_chunks(read_ptr, end_info_ptr,
         &unknowns);
      if (num_unknowns)
      {
         png_size_t i;
         png_set_unknown_chunks(write_ptr, write_end_info_ptr, unknowns,
           num_unknowns);
         /* copy the locations from the read_info_ptr.  The automatically
            generated locations in write_end_info_ptr are wrong because we
            haven't written the end_info yet */
         for (i = 0; i < (png_size_t)num_unknowns; i++)
		 {
           png_set_unknown_chunk_location(write_ptr, write_end_info_ptr, (int)i, (int)unknowns[i].location);
		 }
      }
   }
#endif
#ifdef PNG_WRITE_SUPPORTED
   png_write_end(write_ptr, write_end_info_ptr);
#endif

#ifdef PNG_EASY_ACCESS_SUPPORTED
   if(verbose)
   {
      uint32 iwidth, iheight;
      iwidth = png_get_image_width(write_ptr, write_info_ptr);
      iheight = png_get_image_height(write_ptr, write_info_ptr);
      fprintf(STDERR, "Image width = %lu, height = %lu\n",
         (uint32)iwidth, (uint32)iheight);
   }
#endif

   png_debug(0, "Destroying data structs\n");
#ifdef SINGLE_ROWBUF_ALLOC
   png_debug(1, "destroying row_buf for read_ptr\n");
   png_free(read_ptr, row_buf);
   row_buf=NULL;
#endif /* SINGLE_ROWBUF_ALLOC */
   png_debug(1, "destroying read_ptr, read_info_ptr, end_info_ptr\n");
   png_destroy_read_struct(&read_ptr, &read_info_ptr, &end_info_ptr);
#ifdef PNG_WRITE_SUPPORTED
   png_debug(1, "destroying write_end_info_ptr\n");
   png_destroy_info_struct(write_ptr, &write_end_info_ptr);
   png_debug(1, "destroying write_ptr, write_info_ptr\n");
   png_destroy_write_struct(&write_ptr, &write_info_ptr);
#endif
   png_debug(0, "Destruction complete.\n");

   FCLOSE(fpin);
   FCLOSE(fpout);

   png_debug(0, "Opening files for comparison\n");
#if defined(_WIN32_WCE)
   MultiByteToWideChar(CP_ACP, 0, inname, -1, path, MAX_PATH);
   if ((fpin = CreateFile(path, GENERIC_READ, 0, NULL, OPEN_EXISTING, 0, NULL)) == INVALID_HANDLE_VALUE)
#else
   if ((fpin = fopen(inname, "rb")) == NULL)
#endif
   {
      fprintf(STDERR, "Could not find file %s\n", inname);
      return (1);
   }

#if defined(_WIN32_WCE)
   MultiByteToWideChar(CP_ACP, 0, outname, -1, path, MAX_PATH);
   if ((fpout = CreateFile(path, GENERIC_READ, 0, NULL, OPEN_EXISTING, 0, NULL)) == INVALID_HANDLE_VALUE)
#else
   if ((fpout = fopen(outname, "rb")) == NULL)
#endif
   {
      fprintf(STDERR, "Could not find file %s\n", outname);
      FCLOSE(fpin);
      return (1);
   }

   for(;;)
   {
      png_size_t num_in, num_out;

      READFILE(fpin, inbuf, 1, num_in);
      READFILE(fpout, outbuf, 1, num_out);

      if (num_in != num_out)
      {
         fprintf(STDERR, "\nFiles %s and %s are of a different size\n",
                 inname, outname);
         if(wrote_question == 0)
         {
            fprintf(STDERR,
         "   Was %s written with the same maximum IDAT chunk size (%d bytes),",
              inname,PNG_ZBUF_SIZE);
            fprintf(STDERR,
              "\n   filtering heuristic (libpng default), compression");
            fprintf(STDERR,
              " level (zlib default),\n   and zlib version (%s)?\n\n",
              ZLIB_VERSION);
            wrote_question=1;
         }
         FCLOSE(fpin);
         FCLOSE(fpout);
         return (0);
      }

      if (!num_in)
         break;

      if (png_memcmp(inbuf, outbuf, num_in))
      {
         fprintf(STDERR, "\nFiles %s and %s are different\n", inname, outname);
         if(wrote_question == 0)
         {
            fprintf(STDERR,
         "   Was %s written with the same maximum IDAT chunk size (%d bytes),",
                 inname,PNG_ZBUF_SIZE);
            fprintf(STDERR,
              "\n   filtering heuristic (libpng default), compression");
            fprintf(STDERR,
              " level (zlib default),\n   and zlib version (%s)?\n\n",
              ZLIB_VERSION);
            wrote_question=1;
         }
         FCLOSE(fpin);
         FCLOSE(fpout);
         return (0);
      }
   }

   FCLOSE(fpin);
   FCLOSE(fpout);

   return (0);
}


