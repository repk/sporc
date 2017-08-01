SECTIONS {
	.rodata.memory : {
		__memory_start = .;
		KEEP(*(.rodata.memory))
		__memory_end = .;
	}
	.rodata.cpu : {
		__cpu_start = .;
		KEEP(*(.rodata.cpu))
		__cpu_end = .;
	}
}

INSERT BEFORE .rodata;
