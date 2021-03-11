#if USE_MONOLITHIC_ULIB
# undef USE_MONOLITHIC_ULIB
# include "cstring_basename.c"
# include "cstring_cmp.c"
# include "cstring_eat_whitespace.c"
# include "cstring_to_int.c"
# include "cstring_from_int.c"
# include "cstring_len.c"
# include "cstring_ncmp.c"
# include "cstring_next_token.c"
#endif // USE_MONOLITHIC_ULIB
