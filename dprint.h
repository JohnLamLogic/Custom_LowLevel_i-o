#ifndef DPRINT_H_
#define DPRINT_H_
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

// the following allows us to add debugging statements that are enabled
// or disabled at compile time
#ifdef DEBUG
#define PDEBUG 1
#else
#define PDEBUG 0
#endif

#define DPRINT(fmt, ...) do { if (PDEBUG) fprintf(stderr, "%s:%d:%s(): "fmt,\
                __FILE__, __LINE__, __func__, ##__VA_ARGS__); } while (0)

#endif

