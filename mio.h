#ifndef MIO_H_
#define MIO_H_
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>


// Defined Constants
#define MBSIZE 10       // default size for buffers
#define MODE_R 0        // read only
#define MODE_WA 1       // write only create/append
#define MODE_WT 2       // write only truncate
#define MTAB '\t'       // Tab
#define MNLINE '\n'     // Newline
#define MCRET '\r'      // Carriage return
#define MSPACE ' '      // Space

// Macros
// Is char X whitespace: 1 - yes, 0 - no
#define M_ISWS(X) (((X==MTAB)||(X==MNLINE)||(X==MSPACE)||(X==MCRET)) ? (1) : (0))
// Is int X mode a write type: 1 - yes, 0 - no
#define M_ISMW(X) (((X==MODE_WA)||(X==MODE_WT)) ? (1) : (0))



// mininum information for MIO
struct _mio {
        int fd;                 // file descriptor
        int rw;                 // 0 - read, 1 - write append, 2 - write truncate
        char *rb, *wb;          // buffers
        int rsize, wsize;       // buffer sizes
        int rs, re, ws, we;     // buffer indices
};
typedef struct _mio MIO;

//global variables
extern MIO *mioin;
extern MIO *mioout;
extern MIO *mioerr;

//initialize globals
void myinit();

// open/close functions
MIO *myopen(const char *name, const int mode, const int bsize);
int myclose(MIO *m);

// read functions
int myread(MIO *m, char *b, const int size);
int mygetc(MIO *m, char *c);
char *mygets(MIO *m, int *len);

// write functions
int mywrite(MIO *m, const char *b, const int size);
int myflush(MIO *m);
int myputc(MIO *m, const char c);
int myputs(MIO *m, const char *str, const int len);
MIO *mydopen(const int fd, const int mode, const int bsize);
char *mygetline(MIO *m, int *len);

#endif
