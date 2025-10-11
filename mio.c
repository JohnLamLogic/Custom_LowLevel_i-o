#include "mio.h"

//global variables
MIO *mioin;
MIO *mioout;
MIO *mioerr;

//Initialization
void myinit()
{
    mioin = mydopen(STDIN_FILENO, MODE_R, MBSIZE);
    mioout = mydopen(STDOUT_FILENO, MODE_WA, 0);
    mioerr = mydopen(STDERR_FILENO, MODE_WA, 0);
}


MIO *myopen(const char *name, const int mode, const int bsize)
{
    int fd;

    if (mode == MODE_R)
    {
        fd = open(name, O_RDONLY);
        if (fd < 0)
        {
            return NULL;
        }
    }
    else if (mode == MODE_WA)
    {
        fd = open(name, O_WRONLY | O_CREAT | O_APPEND, 0666);
        if (fd < 0)
        {
            return NULL;
        }
    }
    else if (mode == MODE_WT)
    {
        fd = open(name, O_WRONLY | O_CREAT | O_TRUNC, 0666);
        if (fd < 0)
        {
            return NULL;
        }
    }
    else
    {
        return NULL;
    }

    // alloc the control block (small)
    MIO *m = (MIO *)malloc(sizeof(MIO));
    if (!m)
    {
        close(fd);
        return NULL;
    }

    m->fd    = fd;
    m->rw    = mode;
    m->rsize = 0;
    m->wsize = 0;
    m->rb    = NULL;
    m->wb    = NULL;

    // set up buffer based on mode (read vs write)
    if (mode == MODE_R)
    {
        m->rsize = (bsize > 0) ? bsize : 0;
        if (m->rsize > 0)
        {
          char *tmp = (char *)realloc(m->rb, m->rsize);
          if (!tmp)
          {
              free(m);
              close(fd);
              return NULL;
          }
          m->rb = tmp;
        }
    }
    else if (M_ISMW(mode))
    {
        m->wsize = (bsize > 0) ? bsize : 0;
        if (m->wsize > 0)
        {
          char *tmp = (char *)realloc(m->wb, m->wsize);
          if (!tmp)
          {
              free(m);
              close(fd);
              return NULL;
          }
          m->wb = tmp;
        }
    }
    else
    {
        // unknown mode, bail
        free(m);
        close(fd);
        return NULL;
    }

    m->rs = 0;
    m->re = 0;
    m->ws = 0;
    m->we = 0;

    return m;
}

int myclose(MIO *m)
{
    if (!m)
    {
        return -1;
    }

    // flush if I were in write mode and have pending bytes
    if (M_ISMW(m->rw) && m->wb && m->we > 0)
    {
        if (myflush(m) < 0)
        {
            // even if flush fails, still try to clean up
            // (caller can treat ret code)
        }
    }

    if (m->rb)
    {
        free(m->rb);
    }
    if (m->wb)
    {
        free(m->wb);
    }

    // close the fd last
    int ret = close(m->fd);
    free(m);
    return ret;
}

// read functions
int myread(MIO *m, char *b, const int size)
{
    if (!m || !b || size <= 0)
    {
        return -1;
    }

    if (m->rsize == 0 || m->rb == NULL)
    {
      int bytes = (int)read(m->fd, b, size);
      return ((bytes <= 0) ? -1 : bytes);
    }


    int total = 0;

    while (total < size)
    {
        if (m->rs == m->re)
        {
            // refill input buffer (I zero for clarity; not strictly needed)
            memset(m->rb, 0, m->rsize);

            int bytes = read(m->fd, m->rb, m->rsize);
            if (bytes <= 0)
            {
                // if I already read something, return that amt
                return (total > 0) ? total : -1;
            }
            m->rs = 0;
            m->re = bytes;
        }

        // copy as much as I can from rb to user buf
        int avail = m->re - m->rs;
        int need  = size - total;
        int to_copy = (avail < need) ? avail : need;

        memcpy(b + total, m->rb + m->rs, to_copy);
        m->rs += to_copy;
        total += to_copy;
    }

    return total;
}

int mygetc(MIO *m, char *c)
{
    if (!m || !c)
    {
        return -1;
    }

    // base read of 1 byte (nice and tiny)
    int n = myread(m, c, 1);
    if (n == 1)
    {
        return 1;
    }
    else
    {
        return -1;
    }
}

char *mygets(MIO *m, int *len)
{
    if (!m || !len)
    {
        return NULL;
    }

    int capacity = MBSIZE;              // start size (can grow)
    char *buf = (char *)malloc(capacity);
    if (!buf)
    {
        return NULL;
    }

    int i = 0;
    char c;

    // prime first char (I grab one so I can check WS state)
    if (mygetc(m, &c) != 1)
    {
        free(buf);
        return NULL;
    }

    // skip leading whitespace (space, tab, cr, nl... per your macro)
    while (M_ISWS(c))
    {
        if (mygetc(m, &c) != 1)
        {
            free(buf);
            return NULL;
        }
    }

    // collect chars until next whitespace or EOF
    while (!M_ISWS(c))
    {
        if (i >= capacity - 1)          // keep space for the '\0'
        {
            capacity *= 2;
            char *tmp = (char *)realloc(buf, capacity);
            if (!tmp)
            {
                free(buf);
                return NULL;
            }
            buf = tmp;
        }

        buf[i++] = c;

        if (mygetc(m, &c) != 1)
        {
            // EOF ends the token; break out
            break;
        }
    }

    buf[i] = '\0';
    *len = i;

    return buf;
}

// write functions
int mywrite(MIO *m, const char *b, const int size)
{
    if (!m || !b || size <= 0)
        return -1;

    if (!M_ISMW(m->rw))
        return -1;

    if (m->wsize == 0 || m->wb == NULL)
    {
      int bytes = (int)write(m->fd, b, size);
      return ((bytes <= 0) ? -1 : bytes);
    }

    int total = 0;

    while (total < size)
    {
        // Calculate free space in buffer from ws to wsize
        int free_spc = m->wsize - m->we;

        int need = size - total;
        int to_copy = (free_spc < need) ? free_spc : need;

        memcpy(m->wb + m->we, b + total, to_copy);
        m->we += to_copy;
        total += to_copy;

        // If buffer is full, flush it
        if (m->we == m->wsize)
        {
            int written = write(m->fd, m->wb + m->ws, m->we - m->ws);
            if (written != (m->we - m->ws))
                return (total > 0) ? total : -1;

            // reset indices after flush
            m->ws = 0;
            m->we = 0;
        }
    }

    return total;
}

int myflush(MIO *m)
{
    if (!m || !m->wb || m->we == 0)
    {
        // nothing to flush or invalid MIO (no-op ok)
        return 0;
    }

    int written = write(m->fd, m->wb, m->we);
    if (written != m->we)
    {
        return -1;
    }

    m->we = 0; // reset buffer index after flush (I cleared it)
    return written;
}

int myputc(MIO *m, const char c)
{
    if (!m)
    {
        return -1;
    }

    // just forward to mywrite, single byte
    int n = mywrite(m, &c, 1);
    if (n == 1)
    {
        // return the character written (stdio-style)
        return 1;
    }
    else
    {
        return -1;
    }
}

int myputs(MIO *m, const char *str, const int len)
{
    if (!m || !str || len <= 0)
    {
        return -1;
    }

    // write one char at a time via myputc (layered, a bit slower but simple)
    int i = 0;
    while (i < len)
    {
        if (myputc(m, str[i]) < 0)
        {
            // partial write? return how many I pushed out
            return (i > 0) ? i : -1;
        }
        i++;
    }

    return i;
}


MIO *mydopen(const int fd, const int mode, const int bsize)
{

  if (fd < 0)
  {
    return NULL;
  }

  MIO *m = (MIO *)malloc(sizeof(MIO));
  if (!m)
  {
      return NULL;
  }

  m->fd    = fd;
  m->rw    = mode;
  m->rsize = 0;
  m->wsize = 0;
  m->rb    = NULL;
  m->wb    = NULL;

  // set up buffer based on mode (read vs write)
  if (mode == MODE_R)
  {
      m->rsize = (bsize > 0) ? bsize : 0;
      if (m->rsize > 0)
      {
        char *tmp = (char *)realloc(m->rb, m->rsize);
        if (!tmp)
        {
            free(m);
            return NULL;
        }
        m->rb = tmp;
      }
  }
  else if (M_ISMW(mode))
  {
      m->wsize = (bsize > 0) ? bsize : 0;
      if (m->wsize > 0)
      {
        char *tmp = (char *)realloc(m->wb, m->wsize);
        if (!tmp)
        {
            free(m);
            return NULL;
        }
        m->wb = tmp;
      }
  }
  else
  {
      // unknown mode, bail
      free(m);
      return NULL;
  }

  m->rs = 0;
  m->re = 0;
  m->ws = 0;
  m->we = 0;

  return m;
}

char *mygetline(MIO *m, int *len)
{
    if (!m || !len)
    {
        return NULL;
    }

    int capacity = MBSIZE;
    char *buf = (char *)malloc(capacity);
    if (!buf)
    {
        return NULL;
    }

    int i = 0;
    char c;

    while (1)
    {
        if (mygetc(m, &c) != 1)
        {
            if (i == 0)
            {
                free(buf);
                return NULL;
            }
            break;
        }

        if (c == '\n' || c == '\r')
        {
            if (c == '\r')
            {
                char next;
                if (mygetc(m, &next) == 1)
                {
                    if (next != '\n')
                    {
                        if (m->rsize > 0 && m->rb && m->rs > 0)
                        {
                            m->rs--;
                        }
                    }
                }
            }
            break;
        }

        if (i >= capacity - 1)
        {
            capacity *= 2;
            char *tmp = (char *)realloc(buf, capacity);
            if (!tmp)
            {
                free(buf);
                return NULL;
            }
            buf = tmp;
        }

        buf[i++] = c;
    }

    buf[i] = '\0';
    *len = i;

    return buf;
}