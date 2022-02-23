/**
 ***********************************************************************************************************************
 * Copyright (c) 2020, China Mobile Communications Group Co.,Ltd.
 *
 * Licensed under the Apache License, Version 2.0 (the "License"); you may not use this file except in compliance with
 * the License. You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software distributed under the License is distributed on
 * an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the License for the
 * specific language governing permissions and limitations under the License.
 *
 * @file        dirent.h
 *
 * @brief       Header file for directory operation.
 *
 * @revision
 * Date         Author          Notes
 * 2020-04-17   OneOS team      First Version
 ***********************************************************************************************************************
 */
#ifndef __SYS_DIRENT_H__
#define __SYS_DIRENT_H__

#include <sys/types.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Ref: http://www.opengroup.org/onlinepubs/009695399/basedefs/dirent.h.html */
#define DT_UNKNOWN  0
#define DT_FIFO	    1
#define DT_CHR      2
#define DT_DIR      4
#define DT_BLK      6
#define DT_REG      8
#define DT_LNK      10
#define DT_SOCK	    12
#define DT_WHT      14

struct dirent
{
    ino_t          d_ino;          /* file number    */
    unsigned char  d_type;         /* The type of the file. */
    char           d_name[256];    /* The null-terminated file name. */
};

struct __dirstream
{
    int     fd;			            /* File descriptor.  */
    size_t  allocation;             /* Space allocated for the block.  */
    size_t  size;		            /* Total valid data in the block.  */
    size_t  offset;		            /* Current offset into the block.  */
    off_t   filepos;		        /* Position of next entry to read.  */
    int     errcode;		        /* Delayed error code.  */
    void   *priv;
};

/*
 * This is the data type of directory stream objects.
 * The actual structure is opaque to users.
 */
typedef struct __dirstream DIR;

extern int            closedir(DIR *pdir);
extern DIR           *opendir(const char *path);
extern struct dirent *readdir(DIR *pdir);
extern void           rewinddir(DIR *pdir);
extern void           seekdir(DIR *pdir, long ofst);
extern long           telldir(DIR *pdir);

#ifdef __cplusplus
}
#endif

#endif /* __SYS_DIRENT_H__ */

