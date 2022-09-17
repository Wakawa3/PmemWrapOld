#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <libpmemobj.h>
//#include "pmemopenwrap.h"

#define MAX_PATH_LENGTH 256

#undef pmemobj_open
#undef pmemobj_tx_process
#undef pmemobj_root

PMEMobjpool *pop_copy;
char path_original[MAX_PATH_LENGTH];
char path_replica[MAX_PATH_LENGTH];
int i_number = 0;

int copyFile(const char* srcPath, const char* destPath);

extern inline PMEMobjpool *pmemobjwrap_open(const char *path, const char *layout){
    printf("wrap\n");
    strncpy(path_original, path, MAX_PATH_LENGTH);
    sprintf(path_replica, "%s%s", path, "_replica");
    if(copyFile(path, path_replica) != 0){
        fprintf(stderr, "copyFile error\n");
        exit(1);
    }

    //return pmemobj_open(path_replica, layout);
    pop_copy = pmemobj_open(path_replica, layout);
    return pop_copy;
}

extern inline PMEMoid pmemobjwrap_root(PMEMobjpool *pop, size_t size){
    return pmemobj_root(pop, size);
}

extern inline void pmemobjwrap_tx_process(){
    printf("wrap tx_process\n");
    char path_intermediate[MAX_PATH_LENGTH];
    sprintf(path_intermediate, "%s%s%d", path_original, "_intermediate_", i_number++);
    if(copyFile(path_replica, path_intermediate) != 0){
        fprintf(stderr, "copyFile error\n");
        exit(1);
    }

    pmemobj_tx_process();
}

#define pmemobj_open pmemobjwrap_open
#define pmemobj_tx_process pmemobjwrap_tx_process
#define pmemobj_root pmemobjwrap_root

int copyFile(const char* srcPath, const char* destPath){
    if (strcmp(srcPath, destPath) == 0)
        return -1;

    int result = 1;

    FILE* fpSrc = fopen(srcPath, "rb");
    FILE* fpDest = fopen(destPath, "wb");
    if (fpSrc == NULL || fpDest == NULL)
        result = 0;

    if (result != 0) {
        for (;;) {
            char c;

            // 1バイト読み込む
            if (fread(&c, sizeof(c), 1, fpSrc) < 1) {
                if (feof(fpSrc)) {
                    break;
                }
                else {
                    result = 0;
                    break;
                }
            }

            // 1バイト書き込む
            if (fwrite(&c, sizeof(c), 1, fpDest) < 1) {
                result = 0;
                break;
            }
        }
    }

    if (fpDest != NULL) 
        if (fclose(fpDest) == EOF) 
            result = 0;
    
    if (fpSrc != NULL) 
        if (fclose(fpSrc) == EOF) 
            result = 0;

    return 0;
}



