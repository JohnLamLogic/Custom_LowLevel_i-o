````markdown
MIO(3) — Library Functions — MIO(3)
===================================

### NAME
**mio** — buffered I/O library functions

### SYNOPSIS
```c
#include "mio.h"

void myinit(void);

MIO *myopen(const char *name, const int mode, const int bsize);
MIO *mydopen(const int fd, const int mode, const int bsize);
int  myclose(MIO *m);

int   myread(MIO *m, char *b, const int size);
int   mygetc(MIO *m, char *c);
char *mygets(MIO *m, int *len);
char *mygetline(MIO *m, int *len);

int  mywrite(MIO *m, const char *b, const int size);
int  myflush(MIO *m);
int  myputc(MIO *m, const char c);
int  myputs(MIO *m, const char *str, const int len);
````

### DESCRIPTION

MIO provides a minimal buffered I/O layer over UNIX file descriptors. Each stream tracks its own `fd`, `mode`, buffer(s), and buffer indices. Buffers are heap-allocated; unbuffered mode is possible by specifying `bsize = 0`.

### GLOBAL STREAMS

```c
myinit(void)
```

Initializes global MIO pointers:

* `mioin`  → STDIN  (read, buffered)
* `mioout` → STDOUT (write, unbuffered)
* `mioerr` → STDERR (write, unbuffered)

### FUNCTIONS

```c
myopen(const char *name, int mode, int bsize)
```

Opens a file and returns an `MIO*` with a read or write buffer sized to `bsize`.

Modes:

* `MODE_R`   — read only
* `MODE_WA`  — write, append, create if needed
* `MODE_WT`  — write, truncate/create

Returns `NULL` on failure.

```c
mydopen(int fd, int mode, int bsize)
```

Same as `myopen` but wraps an existing file descriptor.

```c
myclose(MIO *m)
```

Flushes buffered output if needed, closes `fd`, frees buffers and struct. Returns `0` on success or `-1`.

```c
myread(MIO *m, char *b, int size)
```

Reads up to `size` bytes. If buffered, data is taken from the read buffer or `read(2)` refills it. Returns bytes read or `-1` on EOF/error.

```c
mygetc(MIO *m, char *c)
```

Reads one byte via `myread()`. Returns `1` or `-1`.

```c
mygets(MIO *m, int *len)
```

Reads a whitespace-delimited token into a newly allocated string. Leading whitespace skipped. Caller must free. Returns `NULL` on EOF/error.

```c
mygetline(MIO *m, int *len)
```

Reads a line up to `'\n'` or `'\r'`, excluding line break. Returns newly allocated string or `NULL`.

```c
mywrite(MIO *m, const char *b, int size)
```

Writes `size` bytes. Buffered writes fill internal buffer; unbuffered writes call `write(2)`. Returns bytes accepted or `-1`.

```c
myflush(MIO *m)
```

Flushes buffered output. Returns number of bytes flushed or `-1`.

```c
myputc(MIO *m, char c)
```

Writes one character via `mywrite()`. Returns `1` or `-1`.

```c
myputs(MIO *m, const char *str, int len)
```

Writes `len` characters using repeated `myputc()`. Returns count or `-1`.

### CONSTANTS / MACROS

| Name        | Meaning                            |
| ----------- | ---------------------------------- |
| `MBSIZE`    | default buffer size                |
| `MODE_R`    | read                               |
| `MODE_WA`   | write append                       |
| `MODE_WT`   | write truncate                     |
| `M_ISWS(c)` | true if tab, space, CR, or newline |
| `M_ISMW(m)` | true if write mode                 |

### NOTES

* Reads and writes are one-directional per stream.
* `bsize = 0` disables buffering in that direction.
* Caller must free strings returned by `mygets` / `mygetline`.

```
```
