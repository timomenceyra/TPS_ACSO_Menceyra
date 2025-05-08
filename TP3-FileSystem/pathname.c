
#include "pathname.h"
#include "directory.h"
#include "inode.h"
#include "diskimg.h"
#include <stdio.h>
#include <string.h>
#include <assert.h>

/**
 * TODO
 */
int pathname_lookup(struct unixfilesystem *fs, const char *pathname) {
    if (pathname == NULL || pathname[0] != '/') {
        return -1;  // debe ser ruta absoluta
    }

    int inumber = ROOT_INUMBER; // comenzamos desde la raíz
    const char *cur = pathname;

    while (*cur == '/') cur++; // saltar barras iniciales

    char name[DIR_NAME_SIZE + 1];

    while (*cur != '\0') {
        // Extraer siguiente componente del nombre
        int i = 0;
        while (*cur != '/' && *cur != '\0' && i < DIR_NAME_SIZE) {
            name[i++] = *cur++;
        }
        name[i] = '\0';

        while (*cur == '/') cur++; // saltar múltiples '/'

        struct direntv6 dirent;
        if (directory_findname(fs, name, inumber, &dirent) < 0) {
            return -1;
        }

        inumber = dirent.d_inumber;
    }

    return inumber;
}
