SECTIONS
{
	. = 0x0;
	.text : {
		*(.text.boot)
		*(.text)
	}
	.data : {
		*(.data)
	}
	.bss : {
		*(.bss)
	}
}
