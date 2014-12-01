/*
CONTEST: A Very Simple TCP CONnection TESter

The MIT License (MIT)

Copyright (c) 2010-2014 Sang-Kil Park <kaon.park@daumkakao.com>

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#include <stddef.h>
#include <pthread.h>

#define BACKLOG       128     // backlog for listen()
#define DEFAULT_PORT  11212   // default listening port
#define BUF_SIZE      1024    // message chunk size for read()

int clientcount;              // total count of connected clients
int clientlatest;             // latest number of connected clients

typedef struct {
  int client_id;
  int client_sock;
} thread_param_t;
thread_param_t *params;       // params structure for pthread

pthread_mutex_t mutex;        // protecting shared data structures from concurrent modifications.
