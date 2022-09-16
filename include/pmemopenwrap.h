#ifndef PMEMOPENWRAP_H
#define PMEMOPENWRAP_H

#include <libpmemobj.h>
#define pmemobj_open pmemobjtest_open
// #define pmemobj_create pmemobjtest_create

// #define pmemobj_persist pmemobjtest_persist
#define pmemobj_tx_process pmemobjtest_tx_process
// #define pmemobj_close pmemobjtest_close

extern inline PMEMobjpool *pmemobjtest_open(const char *path, const char *layout);

//gcc program.c -Wl,-wrap,getObjectName -o program

#endif

