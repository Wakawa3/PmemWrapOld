#ifndef PMEMOPENWRAP_H
#define PMEMOPENWRAP_H

#include <libpmemobj.h>
#define pmemobj_create pmemobjwrap_create
#define pmemobj_open pmemobjwrap_open
#define pmemobj_close pmemobjwrap_close
// #define pmemobj_root pmemobjwrap_root

// #define pmemobj_persist pmemobjwrap_persist
#define pmemobj_tx_process pmemobjwrap_tx_process
#define pmemobj_tx_end pmemobjwrap_tx_end

// #define pmemobj_close pmemobjwrap_close

extern inline PMEMobjpool *pmemobjwrap_create(const char *path, const char *layout, size_t create_poolsize, mode_t mode);
extern inline PMEMobjpool *pmemobjwrap_open(const char *path, const char *layout);
extern inline void pmemobjwrap_tx_process();
extern inline int pmemobjwrap_tx_end();
extern inline void pmemobjwrap_close(PMEMobjpool *pop);

//gcc program.c -Wl,-wrap,getObjectName -o program

#endif

