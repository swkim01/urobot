#ifndef UTILS_H
#define UTILS_H

#include <limits.h>
#include <getopt.h>

#define ABS(a) (((a) < 0) ? -(a) : (a))
#ifndef MIN
#define MIN(a, b) (((a) < (b)) ? (a) : (b))
#endif
#ifndef MAX
#define MAX(a, b) (((a) > (b)) ? (a) : (b))
#endif
#define LENGTH_OF(x) (sizeof(x)/sizeof(x[0]))

/******************************************************************************
Description.: getopt must get reset, otherwise it can only be called once
Input Value.: -
Return Value: -
******************************************************************************/
static inline void reset_getopt(void)
{
    /* optind=1; opterr=1; optopt=63; */
#ifdef __GLIBC__
    optind = 0;
#else
    optind = 1;
#endif

#ifdef HAVE_OPTRESET
    optreset = 1;
#endif
}

#endif
