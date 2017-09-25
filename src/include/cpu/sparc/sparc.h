#ifndef _CPU_SPARC_SPARC_H_
#define _CPU_SPARC_SPARC_H_

/* Register a memory controller for sparc cpu */
int scpu_add_memory(struct cpu * cpu, struct dev *dev);
/* Remove a memory controller for sparc cpu */
int scpu_rm_memory(struct cpu * cpu);

#endif
