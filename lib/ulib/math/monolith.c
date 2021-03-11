#if USE_MONOLITHIC_ULIB
# undef USE_MONOLITHIC_ULIB
# include "log2_fixedp.c"
# include "log_fixedp.c"
# include "log10_fixedp.c"
# include "div_u64_u16.c"
# include "div_u64_u32.c"
# include "div_u64_u64.c"
# include "div_s64_s64.c"
#endif // USE_MONOLITHIC_ULIB
