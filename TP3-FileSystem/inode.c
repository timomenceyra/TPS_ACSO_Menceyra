#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include "inode.h"
#include "diskimg.h"
#include "unixfilesystem.h"
#include "ino.h"

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

    int bytes = diskimg_readsector(fs->dfd, sector, inodes);
    if (bytes != DISKIMG_SECTOR_SIZE) {
        return -1; // error leyendo el sector
    }

    *inp = inodes[offset]; // copiar inodo deseado

    return 0; 
}

/**
 * TODO
 */
int inode_indexlookup(struct unixfilesystem *fs, struct inode *in, int fileBlockIndex) {
    (void) fs; // no lo usamos en el caso simple

    if (fileBlockIndex < 0) {
        return -1;
    }

    // Si el archivo no es grande (ILARG no seteado)
    if ((in->i_mode & ILARG) == 0) {
        if (fileBlockIndex >= 8) {
            return -1;
        }

        int sector = in->i_addr[fileBlockIndex];
        if (sector == 0) {
            return -1; // bloque no asignado
        }

        return sector;
    }

    // Archivos grandes: usar bloques indirectos
    int indirect_block_limit = 7 * (DISKIMG_SECTOR_SIZE / sizeof(uint16_t));

    if (fileBlockIndex < indirect_block_limit) {
        int indirect_block_index = fileBlockIndex / (DISKIMG_SECTOR_SIZE / sizeof(uint16_t));
        int index_within_block = fileBlockIndex % (DISKIMG_SECTOR_SIZE / sizeof(uint16_t));

        uint16_t indirect_block[DISKIMG_SECTOR_SIZE / sizeof(uint16_t)];

        int sector = in->i_addr[indirect_block_index];
        if (sector == 0) return -1;

        int bytes = diskimg_readsector(fs->dfd, sector, indirect_block);
        if (bytes != DISKIMG_SECTOR_SIZE) return -1;

        int data_sector = indirect_block[index_within_block];
        if (data_sector == 0) return -1;

        return data_sector;
    }

    // Doble indirección: i_addr[7]
    int single_indirect_capacity = DISKIMG_SECTOR_SIZE / sizeof(uint16_t); // 256
    int double_indirect_start = 7 * single_indirect_capacity;

    if (fileBlockIndex < double_indirect_start + single_indirect_capacity * single_indirect_capacity) {
        int relative_index = fileBlockIndex - double_indirect_start;
        int first_level_index = relative_index / single_indirect_capacity;
        int second_level_index = relative_index % single_indirect_capacity;

        uint16_t first_level_block[single_indirect_capacity];
        uint16_t second_level_block[single_indirect_capacity];

        int first_sector = in->i_addr[7];
        if (first_sector == 0) return -1;

        int bytes = diskimg_readsector(fs->dfd, first_sector, first_level_block);
        if (bytes != DISKIMG_SECTOR_SIZE) return -1;

        int second_sector = first_level_block[first_level_index];
        if (second_sector == 0) return -1;

        bytes = diskimg_readsector(fs->dfd, second_sector, second_level_block);
        if (bytes != DISKIMG_SECTOR_SIZE) return -1;

        int data_sector = second_level_block[second_level_index];
        if (data_sector == 0) return -1;

        return data_sector;
    }

    // No manejamos triple indirección aún
    return -1;
}

int inode_getsize(struct inode *inp) {
  return ((inp->i_size0 << 16) | inp->i_size1); 
}
