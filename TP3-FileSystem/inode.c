#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include "inode.h"
#include "diskimg.h"
#include "unixfilesystem.h"
#define INODES_PER_SECTOR (DISKIMG_SECTOR_SIZE / sizeof(struct inode))

/**
 * TODO
 */
int inode_iget(struct unixfilesystem *fs, int inumber, struct inode *inp) {
    if (inumber < 1) {
        return -1; // número de inodo inválido
    }

    int sector = INODE_START_SECTOR + (inumber - 1) / INODES_PER_SECTOR;
    int offset = (inumber - 1) % INODES_PER_SECTOR;

    struct inode inodes[INODES_PER_SECTOR];

    int bytes = diskimg_readsector(fs->dfd, inodes, sector);
    if (bytes != DISKIMG_SECTOR_SIZE) {
        return -1; // error leyendo el sector
    }

    *inp = inodes[offset]; // copiar inodo deseado

    return 0; 
}

/**
 * TODO
 */
int inode_indexlookup(struct unixfilesystem *fs, struct inode *inp,
    int blockNum) {  
        //Implement code here
    return 0;
}

int inode_getsize(struct inode *inp) {
  return ((inp->i_size0 << 16) | inp->i_size1); 
}
