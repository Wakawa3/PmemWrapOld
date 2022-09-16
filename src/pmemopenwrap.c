#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <libpmemobj.h>

#undef pmemobj_open
#undef pmemobj_tx_process

int copyFile(const char* srcPath, const char* destPath);

extern inline PMEMobjpool *pmemobjtest_open(const char *path, const char *layout){
    printf("test\n");
    char dest_path[255];
    sprintf(dest_path, "%s%s", path, "_replica");
    if(copyFile(path, dest_path) != 0){
        fprintf(stderr, "copyFile error\n");
        exit(1);
    }

    return pmemobj_open(dest_path, layout);
}

extern inline void pmemobjtest_tx_process(){
    printf("test tx_process\n");

    pmemobj_tx_process();
}

#define pmemobj_open pmemobjtest_open
#define pmemobj_tx_process pmemobjtest_tx_process

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



