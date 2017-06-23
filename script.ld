SECTIONS {
	.rodata.memory : {
		__memory_start = .;
		*(.rodata.memory)
		__memory_end = .;
	}
}

INSERT BEFORE .rodata;
