/*
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

#include "contest.h"
#include "client.h"
#include "error.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

/**
* thread handler after accept()
*/
void *handle_client(void *params) {
  thread_param_t *p = (thread_param_t *) params;  // thread params
  ssize_t strlen;                                 // message byte size
  char msg[BUF_SIZE];                             // message buffer

  /*
    `read` from a file descriptor(p->client_sock)
   */
  bzero(msg, sizeof(msg));
  while ((strlen = read(p->client_sock, msg, sizeof(msg))) != 0) {

    pthread_mutex_lock(&mutex);
    printf("#%d - Received %zd bytes: ", p->client_id, strlen);
    // print received message
    for (int i = 0; i < strlen; i++)
      printf("%c", msg[i]);
    pthread_mutex_unlock(&mutex);

    // if message is "quit", then exit
    if (strcmp(msg, "quit\r\n") == 0 || strcmp(msg, "QUIT\r\n") == 0)
      break;

    // clear message buffer
    bzero(msg, sizeof(msg));
  }

  pthread_mutex_lock(&mutex);
  printf("#%d Disconnected, TOTAL:%d\n", p->client_id, --clientcount);
  pthread_mutex_unlock(&mutex);

  // close socket and release memory
  if (close(p->client_sock) < 0)
    error("socket close() ERROR");
  free(p);

  return NULL;
}
