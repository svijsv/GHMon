#if USE_MONOLITHIC_ULIB
# undef USE_MONOLITHIC_ULIB
# include "mem_init.c"
# include "vaprintf.c"
# include "vvprintf.c"
#endif // USE_MONOLITHIC_ULIB
