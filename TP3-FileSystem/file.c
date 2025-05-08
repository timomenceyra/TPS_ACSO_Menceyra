#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include "file.h"
#include "inode.h"
#include "diskimg.h"

/**
 * TODO
 */
int file_getblock(struct unixfilesystem *fs, int inumber, int blockNum, void *buf) {
    struct inode in;
    int err = inode_iget(fs, inumber, &in);
    if (err < 0) {
        return -1;
    }

    int sector = inode_indexlookup(fs, &in, blockNum);
    if (sector < 0) {
        return -1;
    }

    int bytes = diskimg_readsector(fs->dfd, sector, buf);
    if (bytes < 0) {
        return -1;
    }

    // Calculamos los bytes válidos en este bloque
    int filesize = (in.i_size0 << 16) | in.i_size1;
    int offset = blockNum * DISKIMG_SECTOR_SIZE;

    if (offset >= filesize) {
        return 0; // bloque fuera del archivo
    }

    int remaining = filesize - offset;
    return (remaining < DISKIMG_SECTOR_SIZE) ? remaining : DISKIMG_SECTOR_SIZE;
}

