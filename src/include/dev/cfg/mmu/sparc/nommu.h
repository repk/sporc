#ifndef _DEV_CFG_SPARC_NOMMU_H_
#define _DEV_CFG_SPARC_NOMMU_H_

/* Config for fixed sparc MMU device */
struct sparc_nommu_cfg {
	/* Sparc cpu */
	char const *cpu;
	/* Data memory controller device name */
	char const *dmem;
	/* Instruction memory controller device name */
	char const *imem;
};

#endif
