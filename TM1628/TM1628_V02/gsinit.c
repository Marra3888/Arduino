extern	void main( void );

void _sdcc_gsinit_startup(void)
{
	__asm pagesel _main __endasm;
	__asm goto _main __endasm;
}