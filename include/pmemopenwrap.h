#ifndef PMEMOPENWRAP_H
#define PMEMOPENWRAP_H

#include <libpmemobj.h>
#define pmemobj_open pmemobjwrap_open
// #define pmemobj_create pmemobjwrap_create
#define pmemobj_root pmemobjwrap_root

// #define pmemobj_persist pmemobjwrap_persist
#define pmemobj_tx_process pmemobjwrap_tx_process
// #define pmemobj_close pmemobjwrap_close

extern inline PMEMobjpool *pmemobjwrap_open(const char *path, const char *layout);

//gcc program.c -Wl,-wrap,getObjectName -o program

#endif

