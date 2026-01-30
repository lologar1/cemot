#ifndef MAIN_H
#define MAIN_H

#include <stdio.h>
#include "cemot.h"

#define INFOSTREAM stderr
#define OUTSTREAM stdout
#define INSTREAM stdin

#ifdef _WIN32
	#define NULLPATH "NUL"
#else
	#define NULLPATH "/dev/null"
#endif

extern u64 linewidth_;

#endif
