---

# MIO(3) — Library Functions — MIO(3)

### NAME

**mio** — buffered I/O library functions

### SYNOPSIS

```c
#include "mio.h"

void myinit(void);

MIO *myopen(const char *name, const int mode, const int bsize);
MIO *mydopen(const int fd, const int mode, const int bsize);
int  myclose(MIO *m);

int  myread(MIO *m, char *b, const int size);
int  mygetc(MIO *m, char *c);
char *mygets(MIO *m, int *len);
char *mygetline(MIO *m, int *len);

int  mywrite(MIO *m, const char *b, const int size);
int  myflush(MIO *m);
int  myputc(MIO *m, const char c);
int  myputs(MIO *m, const char *str, const int len);
```

### DESCRIPTION

MIO provides a minimal buffered I/O layer over UNIX file descriptors. Each stream tracks its own `fd`, mode (`MODE_R`, `MODE_WA`, or `MODE_WT`), buffer(s), and buffer indices. Buffers are heap-allocated; unbuffered mode is possible by specifying `bsize = 0` for the respective direction.

### GLOBAL STREAMS

```c
void myinit(void);
```

Initializes global MIO pointers using `mydopen(2)`:

* `mioin`  → STDIN  (read, **buffered** with `MBSIZE`)
* `mioout` → STDOUT (write, **buffered** with `MBSIZE`)
* `mioerr` → STDERR (write, **unbuffered** with `bsize = 0`)

### FUNCTIONS

```c
MIO *myopen(const char *name, const int mode, const int bsize);
```

Opens a file and returns an `MIO *` with a read **or** write buffer sized to `bsize` (depending on mode).

Modes:

* `MODE_R`   — read only
* `MODE_WA`  — write, append, create if needed
* `MODE_WT`  — write, truncate/create

Returns `NULL` on failure.

```c
MIO *mydopen(const int fd, const int mode, const int bsize);
```

Wraps an existing file descriptor `fd` as an `MIO *`, allocating a read or write buffer per `mode` and `bsize`. Returns `NULL` on failure.

```c
int myclose(MIO *m);
```

Flushes buffered output if needed, closes the underlying descriptor, frees buffers and struct. Returns the result of `close(2)` (0 on success, −1 on error).

```c
int myread(MIO *m, char *b, const int size);
```

Reads up to `size` bytes. If buffered, data is taken from the read buffer (refilled by `read(2)` as needed). In unbuffered read mode, calls `read(2)` directly. Returns number of bytes read; returns −1 on EOF/error when no bytes were read.

```c
int mygetc(MIO *m, char *c);
```

Reads one byte via `myread()`. Returns 1 on success or −1 on EOF/error.

```c
char *mygets(MIO *m, int *len);
```

Reads a whitespace-delimited token into a newly allocated, NUL-terminated string. Leading whitespace is skipped (tab, space, CR, newline). Caller must `free()` the returned pointer. Returns `NULL` on EOF/error.

```c
char *mygetline(MIO *m, int *len);
```

Reads a line up to and **including** the first `'\n'` (if present). The returned buffer is NUL-terminated; `*len` receives the character count **including** the newline when present. Returns a newly allocated string or `NULL` on EOF/error with no data. Caller must `free()`.

```c
int mywrite(MIO *m, const char *b, const int size);
```

Writes `size` bytes. In buffered mode, bytes are copied into the internal buffer and flushed as needed; in unbuffered mode, calls `write(2)` directly. Returns number of bytes accepted or −1 on error/invalid usage.

```c
int myflush(MIO *m);
```

Flushes buffered output. Returns the number of bytes written; returns 0 if there was nothing to flush; returns −1 on error.

```c
int myputc(MIO *m, const char c);
```

Writes one character via `mywrite()`. Returns 1 on success or −1 on error.

```c
int myputs(MIO *m, const char *str, const int len);
```

Writes `len` characters using `myputc()` repeatedly. Returns the count written, or −1 on error if nothing was written.

### CONSTANTS / MACROS

| Name        | Meaning                                |
| ----------- | -------------------------------------- |
| `MBSIZE`    | default buffer size (currently 10)     |
| `MODE_R`    | read                                   |
| `MODE_WA`   | write append                           |
| `MODE_WT`   | write truncate                         |
| `M_ISWS(c)` | true if tab, space, CR, or newline     |
| `M_ISMW(x)` | true if `x` is a write mode (WA or WT) |

### NOTES

* Streams are one-directional per `MIO` instance (read **or** write).
* `bsize = 0` disables buffering in that direction.
* Caller must `free()` strings returned by `mygets()` and `mygetline()`.
* `mygetline()` includes the trailing newline when present (NUL-terminated).
