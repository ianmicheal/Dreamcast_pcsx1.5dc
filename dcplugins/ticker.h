
#include <time.h>

/* clock stuff */
typedef long uclock_t;
uclock_t uclock(void);
#define UCLOCKS_PER_SEC 1000000
#define TICKER  uclock_t
#define TICKS_PER_SEC UCLOCKS_PER_SEC
#define TICKS_PER_MS 1000.0 /* TICKS_PER_SEC / 1000 */
#define ticker() uclock()
