SECTIONS {
	.rodata.driver : {
		__drv_start = .;
		KEEP(*(.rodata.driver))
		__drv_end = .;
	}
	.rodata.cpu : {
		__cpu_start = .;
		KEEP(*(.rodata.cpu))
		__cpu_end = .;
	}
}

INSERT BEFORE .rodata;
