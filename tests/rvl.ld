/*--------------------------------------------------------------------------------
	** Linker script for Wii homebrew **
--------------------------------------------------------------------------------*/

PHDRS {
	stub PT_LOAD FLAGS(5);
}

SECTIONS {

	/* stub is loaded at physical address 0x00003400 (though both 0x80003400 and 0x00003400 are equivalent for IOS) */
	/* This can also be used to load an arbitrary standalone stub at an arbitrary address in memory, for any purpose */
	/* Use -Wl,--section-start,.stub=0xADDRESS to change */
	. = 0x00003400;

	.stub : {
		KEEP(*(.stub))
	} :stub = 0

	/* Default base address */
	/* Use -Wl,--section-start,.init=0xADDRESS to change */
	. = 0x80003f00;

}

PROVIDE_HIDDEN( __Arena2Lo    = 0x90002000 );
PROVIDE_HIDDEN( __Arena2Hi    = 0x933e0000 );
PROVIDE_HIDDEN( __ipcbufferLo = 0x933e0000 );
PROVIDE_HIDDEN( __ipcbufferHi = 0x93400000 );

INCLUDE ../libogc_common.ld
