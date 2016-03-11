/*
    Copyright (c) 2012 Martin Sustrik  All rights reserved.

    Permission is hereby granted, free of charge, to any person obtaining a copy
    of this software and associated documentation files (the "Software"),
    to deal in the Software without restriction, including without limitation
    the rights to use, copy, modify, merge, publish, distribute, sublicense,
    and/or sell copies of the Software, and to permit persons to whom
    the Software is furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included
    in all copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
    THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
    FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
    IN THE SOFTWARE.
*/

#include "../src/grid.h"
#include "../src/tcp.h"
#include "../src/pair.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

int main (int argc, char *argv [])
{
    const char *bind_to;
    size_t sz;
    int rts;
    char *buf;
    int nbytes;
    int s;
    int rc;
    int i;
    int opt;

    if (argc != 4) {
        printf ("usage: local_lat <bind-to> <msg-size> <roundtrips>\n");
        return 1;
    }
    bind_to = argv [1];
    sz = atoi (argv [2]);
    rts = atoi (argv [3]);

    s = grid_socket (AF_SP, GRID_PAIR);
    assert (s != -1);
    opt = 1;
    rc = grid_setsockopt (s, GRID_TCP, GRID_TCP_NODELAY, &opt, sizeof (opt));
    assert (rc == 0);
    rc = grid_bind (s, bind_to);
    assert (rc >= 0);

    buf = malloc (sz);
    assert (buf);
    memset (buf, 111, sz);

    for (i = 0; i != rts; i++) {
        nbytes = grid_recv (s, buf, sz, 0);
        assert (nbytes == (int)sz);
        nbytes = grid_send (s, buf, sz, 0);
        assert (nbytes == (int)sz);
    }

    free (buf);

    rc = grid_close (s);
    assert (rc == 0);

    return 0;
}
