#include "directory.h"
#include "inode.h"
#include "diskimg.h"
#include "file.h"
#include "direntv6.h"
#include <stdio.h>
#include <string.h>
#include <assert.h>

/**
 * TODO
 */
int directory_findname(struct unixfilesystem *fs, const char *name,
		int dirinumber, struct direntv6 *dirEnt) {
  
  struct inode dirinode;
  int err = inode_iget(fs, dirinumber, &dirinode);
  if (err < 0) return -1;

  if ((dirinode.i_mode & IFMT) != IFDIR) return -1; // no es un directorio

  int filesize = (dirinode.i_size0 << 16) | dirinode.i_size1;
  int offset = 0;

  while (offset < filesize) {
    char block[DISKIMG_SECTOR_SIZE];
    int bytes = file_getblock(fs, dirinumber, offset / DISKIMG_SECTOR_SIZE, block);
    if (bytes < 0) return -1;

    int entries = bytes / sizeof(struct direntv6);
    struct direntv6 *entry = (struct direntv6 *)block;

    for (int i = 0; i < entries; i++) {
      if (strncmp(entry[i].d_name, name, DIR_NAME_SIZE) == 0) {
        *dirEnt = entry[i];
        return 0;
      }
    }

    offset += bytes;
  }

  return -1; // no encontrado
}
