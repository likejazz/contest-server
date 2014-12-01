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

#include <stdio.h>
#include <stdlib.h>

#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>

#include "contest.h"
#include "client.h"
#include "error.h"

int main(int argc, char *argv[]) {
  int parentfd;                   // listening socket
  int childfd;                    // connection socket
  int port = DEFAULT_PORT;        // port to listen on
  unsigned int clientlen;         // byte size of client's address
  struct sockaddr_in serveraddr;  // server's addr
  struct sockaddr_in clientaddr;  // client addr
  struct hostent *hostp;          // client host info
  int optval = 1;                 // flag value for `setsockopt`

  pthread_t thread_id;            // thread id

  // initialize global variables
  clientcount = 0;
  clientlatest = 0;
  params = NULL;

  /*
    check command line arguments
   */
  if (argc > 1) {
    if (strcmp(argv[1], "-h") == 0 ||
            strcmp(argv[1], "--help") == 0 ||
            argc > 2) {
      printf("Usage: %s <Port>\n", argv[0]);
      exit(EXIT_FAILURE);
    }
    port = atoi(argv[1]);
  }

  pthread_mutex_init(&mutex, NULL);
  // create the parent socket
  if ((parentfd = socket(PF_INET, SOCK_STREAM, 0)) < 0)
    error("socket() opening ERROR");

  // build server's internet addr and initialize
  memset((char *) &serveraddr, 0, sizeof(serveraddr));
  serveraddr.sin_family = AF_INET;                // an internet addr
  serveraddr.sin_addr.s_addr = htonl(INADDR_ANY); // let the system figure out our IP addr
  serveraddr.sin_port = htons(port);              // the port we will listen on

  // setsockopt: handy debugging trick
  // that lets us rerun the server immediately after we kill it
  if (setsockopt(parentfd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)) < 0)
    error("setsockopt() ERROR");
  if (bind(parentfd, (struct sockaddr *) &serveraddr, sizeof(serveraddr)) < 0)
    error("bind() ERROR. Not enough privilleges(<1024) or already in use");
  if (listen(parentfd, BACKLOG) < 0)
    error("listen() ERROR");

  printf("Listening on %d\n", port);
  clientlen = sizeof(clientaddr);

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wmissing-noreturn"
  /*
    enter an infinite loop to respond to client requests
   */
  while (1) {
    /*
      wait for a connection, then accept() it
     */
    if ((childfd = accept(parentfd, (struct sockaddr *) &clientaddr, &clientlen)) < 0)
      error("accept() ERROR");

    // determine who sent the message
    if ((hostp = gethostbyaddr(
            (const char *) &clientaddr.sin_addr.s_addr,
            sizeof(clientaddr.sin_addr.s_addr),
            AF_INET)) == NULL)
      error("gethostbyaddr() ERROR");

    pthread_mutex_lock(&mutex);
    clientcount++;
    clientlatest++;
    printf("#%d - Connected from %s(%s:%d), TOTAL:%d\n",
            clientlatest,
            hostp->h_name, inet_ntoa(clientaddr.sin_addr), ntohs(clientaddr.sin_port),
            clientcount);
    pthread_mutex_unlock(&mutex);

    // memory allocation for `thread_param_t` struct
    params = malloc(sizeof(thread_param_t));
    params->client_id   = clientlatest;
    params->client_sock = childfd;

    // 1 client, 1 thread
    pthread_create(&thread_id, NULL, handle_client, (void *) params);
    pthread_detach(thread_id);
  }
#pragma clang diagnostic pop
}