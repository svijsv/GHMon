#if USE_MONOLITHIC_ULIB
# undef USE_MONOLITHIC_ULIB
# include "date_to_seconds.c"
# include "seconds_to_date.c"
# include "seconds_to_time.c"
# include "time_to_seconds.c"
# include "private.c"
#endif // USE_MONOLITHIC_ULIB
