#ifndef _CPU_SPARC_SPARC_H_
#define _CPU_SPARC_SPARC_H_

#define SPARC_AS_SRMMU_CACHE 0x3
#define SPARC_AS_SRMMU_REG 0x4
#define SPARC_AS_UDATA 0xa
#define SPARC_AS_SDATA 0xb
#define SPARC_AS_UISN 0x8
#define SPARC_AS_SISN 0x9
typedef uint8_t asi_t;

/* Register a memory controller for sparc alternate space accesses */
int scpu_register_mem(struct cpu * cpu, asi_t id, struct dev *dev);
/* Remove a memory controller for sparc alternate space accesses */
int scpu_remove_mem(struct cpu * cpu, asi_t id);

#endif
