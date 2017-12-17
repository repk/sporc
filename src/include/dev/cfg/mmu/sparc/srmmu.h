#ifndef _DEV_CFG_SPARC_SRMMU_H_
#define _DEV_CFG_SPARC_SRMMU_H_

/* Config for reference sparc MMU device */
struct sparc_srmmu_cfg {
	/* Sparc cpu */
	char const *cpu;
	/* Data memory controller device name */
	char const *dmem;
	/* Instruction memory controller device name */
	char const *imem;
};

#endif
