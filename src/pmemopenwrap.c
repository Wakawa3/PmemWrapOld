#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <libpmemobj.h>
//#include "pmemopenwrap.h"

#define MAX_PATH_LENGTH 256

#undef pmemobj_create
#undef pmemobj_open
#undef pmemobj_close
#undef pmemobj_tx_process
#undef pmemobj_tx_end
// #undef pmemobj_root

PMEMobjpool *pop_replica;
int pool_size;
char path_original[MAX_PATH_LENGTH];
char path_replica[MAX_PATH_LENGTH];
int i_number = 0;
int original_fd;
void *original_addr;

int create_flag = 0;

size_t GetFileSize(const char* FileName);
int poolcopy(const char* destPath);
int copyFile(const char* srcPath, const char* destPath);

extern inline PMEMobjpool *pmemobjwrap_create(const char *path, const char *layout, size_t create_poolsize, mode_t mode){
    printf("wrap create\n");

    PMEMobjpool *pop_original = pmemobj_create(path, layout, create_poolsize, mode);
    if(pop_original == NULL){
        return pop_original;
    }

    pmemobj_close(pop_original);

    // int fd = open("/mnt/pmem0/test", O_CREAT|O_RDWR, 0666);
    // ftruncate(fd, PMEMOBJ_MIN_POOL);
	// pop_replica = mmap(NULL, PMEMOBJ_MIN_POOL, PROT_WRITE, MAP_SHARED, fd, 0);
    // memcpy(pop_replica, pop_original, PMEMOBJ_MIN_POOL);

    strncpy(path_original, path, MAX_PATH_LENGTH);
    sprintf(path_replica, "%s%s", path, "_replica");
    
    pop_replica = pmemobj_create(path_replica, layout, create_poolsize, mode);
    if(pop_replica == NULL){
        fprintf(stderr, "error: pmemobjwrap_create\n");
        exit(1);
    }
    pool_size = create_poolsize;
    printf("poolsize : %d\n", pool_size);

    original_fd = open(path_original, O_CREAT|O_RDWR, 0666);
    ftruncate(original_fd, pool_size);
    original_addr = mmap(NULL, pool_size, PROT_WRITE, MAP_SHARED, original_fd, 0);

    create_flag = 1;

    return pop_replica;
}

extern inline PMEMobjpool *pmemobjwrap_open(const char *path, const char *layout){
    printf("wrap open\n");

    strncpy(path_original, path, MAX_PATH_LENGTH);
    sprintf(path_replica, "%s%s", path, "_replica");

    pool_size = GetFileSize(path_original);
    printf("poolsize : %d\n", pool_size);

    if(copyFile(path_original, path_replica) != 0){
        fprintf(stderr, "copyFile error\n");
        exit(1);
    }

    //return pmemobj_open(path_replica, layout);
    pop_replica = pmemobj_open(path_replica, layout);

    original_fd = open(path_original, O_RDWR, 0666);
    original_addr = mmap(NULL, pool_size, PROT_WRITE, MAP_SHARED, original_fd, 0);

    return pop_replica;
}

// extern inline PMEMoid pmemobjwrap_root(PMEMobjpool *pop, size_t size){
//     return pmemobj_root(pop, size);
// }

extern inline void pmemobjwrap_tx_process(){
    printf("wrap tx_process\n");
    // char path_intermediate[MAX_PATH_LENGTH];
    // sprintf(path_intermediate, "%s%s%d", path_original, "_intermediate_", i_number++);
    // if(poolcopy(path_intermediate) != 0){
    //     fprintf(stderr, "copyFile error\n");
    //     exit(1);
    // }

    pmemobj_tx_process();
}

extern inline int pmemobjwrap_tx_end(){
    printf("wrap tx_end\n");
    char path_intermediate[MAX_PATH_LENGTH];
    sprintf(path_intermediate, "%s%s%d", path_original, "_intermediate_", i_number++);
    if(poolcopy(path_intermediate) != 0){
        fprintf(stderr, "poolcopy error\n");
        exit(1);
    }

    int ret = pmemobj_tx_end();

    memcpy(original_addr, pop_replica, pool_size);

    return ret;
}

extern inline void pmemobjwrap_close(PMEMobjpool *pop){
    munmap(original_addr, pool_size);
    close(original_fd);

    pmemobj_close(pop);
}

#define pmemobj_create pmemobjwrap_create
#define pmemobj_open pmemobjwrap_open
#define pmemobj_close pmemobjwrap_close
#define pmemobj_tx_process pmemobjwrap_tx_process
#define pmemobj_tx_end pmemobjwrap_tx_end
// #define pmemobj_root pmemobjwrap_root

size_t GetFileSize(const char* FileName)
{
    size_t fsize = 0;

    FILE* fp = fopen(FileName, "rb");

    /* ファイルサイズを調査 */
    fseek(fp, 0, SEEK_END);
    fsize = ftell(fp);

    fclose(fp);

    return fsize;
}

int poolcopy(const char* destPath){
    int fd = open(destPath, O_CREAT|O_RDWR, 0666);
    ftruncate(fd, pool_size);
    char *dest_addr = mmap(NULL, pool_size, PROT_WRITE, MAP_SHARED, fd, 0);

    memcpy(dest_addr, pop_replica, pool_size);

    munmap(dest_addr, pool_size);
    close(fd);
		
	return 0;
}

int copyFile(const char* srcPath, const char* destPath){
    int dfd = open(destPath, O_CREAT|O_RDWR, 0666);
    int sfd = open(srcPath, O_CREAT|O_RDWR, 0666);
    ftruncate(dfd, pool_size);
    char *dest_addr = mmap(NULL, pool_size, PROT_WRITE, MAP_SHARED, dfd, 0);
    char *src_addr = mmap(NULL, pool_size, PROT_READ, MAP_PRIVATE, dfd, 0);

    memcpy(dest_addr, src_addr, pool_size);

    munmap(dest_addr, pool_size);
    close(dfd);
    munmap(src_addr, pool_size);
    close(sfd);


    // if (strcmp(srcPath, destPath) == 0)
    //     return -1;

    // int result = 1;

    // FILE* fpSrc = fopen(srcPath, "rb");
    // FILE* fpDest = fopen(destPath, "wb");
    // if (fpSrc == NULL || fpDest == NULL)
    //     result = 0;

    // if (result != 0) {
    //     for (;;) {
    //         char c;

    //         // 1バイト読み込む
    //         if (fread(&c, sizeof(c), 1, fpSrc) < 1) {
    //             if (feof(fpSrc)) {
    //                 break;
    //             }
    //             else {
    //                 result = 0;
    //                 break;
    //             }
    //         }

    //         // 1バイト書き込む
    //         if (fwrite(&c, sizeof(c), 1, fpDest) < 1) {
    //             result = 0;
    //             break;
    //         }
    //     }
    // }

    // if (fpDest != NULL) 
    //     if (fclose(fpDest) == EOF) 
    //         result = 0;
    
    // if (fpSrc != NULL) 
    //     if (fclose(fpSrc) == EOF) 
    //         result = 0;

    return 0;
}



