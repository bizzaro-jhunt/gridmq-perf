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
#include "../src/pair.h"

#include "../src/utils/attr.h"

#include "../src/utils/err.c"
#include "../src/utils/thread.c"
#include "../src/utils/stopwatch.c"

#include <stddef.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>

static size_t message_size;
static int message_count;

void worker (GRID_UNUSED void *arg)
{
    int rc;
    int s;
    int i;
    char *buf;

    s = grid_socket (AF_SP, GRID_PAIR);
    assert (s != -1);
    rc = grid_connect (s, "inproc://inproc_thr");
    assert (rc >= 0);

    buf = malloc (message_size);
    assert (buf);
    memset (buf, 111, message_size);

    rc = grid_send (s, NULL, 0, 0);
    assert (rc == 0);

    for (i = 0; i != message_count; i++) {
        rc = grid_send (s, buf, message_size, 0);
        assert (rc == (int)message_size);
    }

    free (buf);
    rc = grid_close (s);
    assert (rc == 0);
}

int main (int argc, char *argv [])
{
    int rc;
    int s;
    int i;
    char *buf;
    struct grid_thread thread;
    struct grid_stopwatch stopwatch;
    uint64_t elapsed;
    unsigned long throughput;
    double megabits;

    if (argc != 3) {
        printf ("usage: thread_thr <message-size> <message-count>\n");
        return 1;
    }

    message_size = atoi (argv [1]);
    message_count = atoi (argv [2]);

    s = grid_socket (AF_SP, GRID_PAIR);
    assert (s != -1);
    rc = grid_bind (s, "inproc://inproc_thr");
    assert (rc >= 0);

    buf = malloc (message_size);
    assert (buf);

    grid_thread_init (&thread, worker, NULL);

    /*  First message is used to start the stopwatch. */
    rc = grid_recv (s, buf, message_size, 0);
    assert (rc == 0);

    grid_stopwatch_init (&stopwatch);

    for (i = 0; i != message_count; i++) {
        rc = grid_recv (s, buf, message_size, 0);
        assert (rc == (int)message_size);
    }

    elapsed = grid_stopwatch_term (&stopwatch);

    grid_thread_term (&thread);
    free (buf);
    rc = grid_close (s);
    assert (rc == 0);

    if (elapsed == 0)
        elapsed = 1;
    throughput = (unsigned long)
        ((double) message_count / (double) elapsed * 1000000);
    megabits = (double) (throughput * message_size * 8) / 1000000;

    printf ("message size: %d [B]\n", (int) message_size);
    printf ("message count: %d\n", (int) message_count);
    printf ("mean throughput: %d [msg/s]\n", (int) throughput);
    printf ("mean throughput: %.3f [Mb/s]\n", (double) megabits);

    return 0;
}

