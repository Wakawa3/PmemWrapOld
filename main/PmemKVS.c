#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <libpmemobj.h>
#include "pmemopenwrap.h"

// Name of our layout in the pool
#define LAYOUT "KVS_layout"
//#define NUM_LAYOUT "number"

// Maximum length of our buffer
#define MAX_BUF_LEN 30
#define MAX_STORE 100

#define PATH "/mnt/pmem0/data"
// #define PATH "/mnt/pmem0/data_replica"
//#define NUM_PATH "num"

#define READ_FROM_KEY 0
#define READ_ALL 1
#define READ_FROM_ID 2

// Root structure
struct subKVstruct {
	//int id;
    int keylen;
	char key[MAX_BUF_LEN];
    int value;
};


struct KVstruct{
    struct subKVstruct data[MAX_STORE];
    int number;
};

POBJ_LAYOUT_BEGIN(string_store);
POBJ_LAYOUT_ROOT(string_store, struct KVstruct);
POBJ_LAYOUT_TOID(string_store, struct subKVstruct);
POBJ_LAYOUT_END(string_store);



void write_KVS (char *key, int value)
{
    PMEMobjpool *pop;
    int id;
    TOID(struct KVstruct) root;
    TOID(struct subKVstruct) sub;
    
    pop = pmemobj_create(PATH, POBJ_LAYOUT_NAME(string_store), PMEMOBJ_MIN_POOL, 0666);
		
	if (pop == NULL) 
	{
        pop = pmemobj_open(PATH, POBJ_LAYOUT_NAME(string_store));

        printf("test3\n");
        if(pop == NULL){
            perror(PATH);
            exit(1);
        }

        root = POBJ_ROOT(pop, struct KVstruct);
        id = D_RO(root)->number;
	}
    else{
        root = POBJ_ROOT(pop, struct KVstruct);
        id = 0;
    }
    

    TX_BEGIN(pop){
        TX_ADD(root);
	    D_RW(root)->data[id].keylen = strlen(key);
        D_RW(root)->data[id].value = value;
        TX_MEMCPY(D_RW(root)->data[id].key, key, D_RO(root)->data[id].keylen);

        D_RW(root)->number = id + 1;
    } TX_END

	printf("Write [key: %s, value: %d]\n", D_RO(root)->data[id].key, D_RO(root)->data[id].value);
    printf("number : %d\n", D_RO(root)->number);

	pmemobj_close(pop);	
		
	return;
}

void read_KVS(char *key, int read_flag, int id)
{
    PMEMobjpool *pop;
    int number;
    TOID(struct KVstruct) root;
    
    pop = pmemobj_open(PATH, POBJ_LAYOUT_NAME(string_store));
		
	if (pop == NULL) 
	{
        perror(PATH);
        exit(1);
    }

    root = POBJ_ROOT(pop, struct KVstruct);
    number = D_RO(root)->number;
	

    int flag = 0;
    int i;
    if(read_flag == READ_FROM_KEY){
        for(i = 0; i < number; i++){
            printf("loop: %d\n", i);
            if(strncmp(D_RO(root)->data[i].key, key, D_RO(root)->data[i].keylen) == 0){
                printf("Read [key: %s, value: %d]\n", D_RO(root)->data[i].key, D_RO(root)->data[i].value);
                flag = 1;
                break;
            }
        }
        if(flag == 0)
            printf("%s is not registered.\n", key);
    }
    else if(read_flag == READ_ALL){
        for(i = 0; i < number; i++){
            printf("Read [key: %s, value: %d]\n", D_RO(root)->data[i].key, D_RO(root)->data[i].value);
        }
    }
    else{
        printf("Read [key: %s, value: %d]\n", D_RO(root)->data[id].key, D_RO(root)->data[id].value);
    }
    
	pmemobj_close(pop);

	return;
}

void delete_KVS(char *key){
    PMEMobjpool *pop;
    int number;
    TOID(struct KVstruct) root;
    
    pop = pmemobj_open(PATH, POBJ_LAYOUT_NAME(string_store));
		
	if (pop == NULL) 
	{
        perror(PATH);
        exit(1);
    }

    root = POBJ_ROOT(pop, struct KVstruct);
    number = D_RO(root)->number;

    int flag = 0;
    int i;
    for(i = 0; i < number; i++){
        printf("loop: %d\n", i);
        if(strncmp(D_RO(root)->data[i].key, key, D_RO(root)->data[i].keylen) == 0){
            printf("Read [key: %s, value: %d]\n", D_RO(root)->data[i].key, D_RO(root)->data[i].value);
            TX_BEGIN(pop){
                if(i != number - 1){     
                    TX_ADD(root);
                    TX_MEMCPY(&(D_RW(root)->data[i]), &(D_RO(root)->data[number-1]), sizeof(struct subKVstruct));
                    printf("overwrite %s\n", key);
                }
                D_RW(root)->number = number - 1;
            }TX_END
            flag = 1;
            break;
        }
    }
    if(flag == 0)
        printf("%s is not registered.\n", key);
    
	pmemobj_close(pop);

	return;
}

void manipulate_data(int id, char *key, int value){
    PMEMobjpool *pop;
    TOID(struct KVstruct) root;
    TOID(struct subKVstruct) sub;
    
    pop = pmemobj_create(PATH, POBJ_LAYOUT_NAME(string_store), PMEMOBJ_MIN_POOL, 0666);
		
	if (pop == NULL) 
	{
        pop = pmemobj_open(PATH, POBJ_LAYOUT_NAME(string_store));

        if(pop == NULL){
            perror(PATH);
            exit(1);
        }
    }

    root = POBJ_ROOT(pop, struct KVstruct);

    TX_BEGIN(pop){
        TX_ADD(root);
	    D_RW(root)->data[id].keylen = strlen(key);
        D_RW(root)->data[id].value = value;
        TX_MEMCPY(D_RW(root)->data[id].key, key, D_RO(root)->data[id].keylen);
    } TX_END

	printf("Manipulate [id: %d, key: %s, value: %d]\n", id, D_RO(root)->data[id].key, D_RO(root)->data[id].value);
    printf("number : %d\n", D_RO(root)->number);

	pmemobj_close(pop);	
		
	return;
}

void manipulate_number(int number){
    PMEMobjpool *pop;
    TOID(struct KVstruct) root;
    TOID(struct subKVstruct) sub;
    
    pop = pmemobj_create(PATH, POBJ_LAYOUT_NAME(string_store), PMEMOBJ_MIN_POOL, 0666);
		
	if (pop == NULL) 
	{
        pop = pmemobj_open(PATH, POBJ_LAYOUT_NAME(string_store));

        if(pop == NULL){
            perror(PATH);
            exit(1);
        }
    }

    root = POBJ_ROOT(pop, struct KVstruct);

    TX_BEGIN(pop){
        TX_ADD(root);
	    D_RW(root)->number = number;
    } TX_END
    
    printf("number : %d\n", D_RO(root)->number);

	pmemobj_close(pop);	
		
	return;
}

/****************************
 * This main function gather from the command line and call the appropriate
 * function to perform read and write persistently to memory.
 *****************************/
int main(int argc, char *argv[])
{
    char *key = argv[2];
	
	if (strcmp (argv[1], "-w") == 0 && argc == 4) {
        int value = atoi(argv[3]);
		write_KVS(key, value);
	} else if (strcmp (argv[1], "-r") == 0 && argc == 3) {
		read_KVS(key, READ_FROM_KEY, -1);
	} else if (strcmp (argv[1], "-ra") == 0 && argc == 2) {
		read_KVS("", READ_ALL, -1);
    } else if (strcmp (argv[1], "-ri") == 0 && argc == 3) {
		read_KVS("", READ_FROM_ID, atoi(argv[2]));
	} else if (strcmp (argv[1], "-d") == 0 && argc == 3){
        delete_KVS(key);
    } else if (strcmp (argv[1], "-md") == 0 && argc == 5){
        //-md number key value
        manipulate_data(atoi(argv[2]), argv[3], atoi(argv[4]));
    } else if (strcmp (argv[1], "-mn") == 0 && argc == 3){
        //-mn number
        manipulate_number(atoi(argv[2]));
    } else { 
		fprintf(stderr, "Usage error: %s\n", argv[0]);
		exit(1);
	}

}